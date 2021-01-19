#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#define SERVER_NAME                "httpd"
#define SERVER_VER                 "1.0"
#define HTTP_VERSION_PREFIX        "HTTP/1."
#define DEFAULT_PORT               "80"
#define MAX_BACKLOG                5
#define MAX_REQUEST_CONTENT_LENGTH 4096 * 4096

struct HttpHeaderField {
  char *name;
  char *value;
  struct HttpHeaderField *next;
};

struct HttpRequest {
  int protocol_minor_version;
  char *method;
  char *path;
  struct HttpHeaderField *headers;
  long length;
  char *body;
};

struct FileInfo {
  char *path;
  long size;
  bool exists;
};

static bool debug_mode = false;

static void usage(FILE *restrict f, const char *restrict cmd)
{
  fprintf(f, "Udage: %s [-p PORT] [-c -g GROUP -u USER] <DOCROOT>\n", cmd);
}

static void setup_environment(const char *restrict docroot, const char *restrict group,
                              const char *restrict user)
{
}

static noreturn void log_exit(const char *restrict fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fputc('\n', stderr);
  va_end(ap);
  exit(EXIT_FAILURE);
}

static void trap_signal(int sig, void (*handler)(int))
{
  struct sigaction act;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  if (sigaction(sig, &act, NULL) != 0) {
    log_exit("sigaction(2) failed: %s", strerror(errno));
  }
}

static noreturn void signal_exit(int sig)
{
  log_exit("exit by signal %d", sig);
}

static void detach_children(void (*handler)(int))
{
  struct sigaction act;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART | SA_NOCLDWAIT;
  if (sigaction(SIGCHLD, &act, NULL) != 0) {
    log_exit("sigaction(2) failed: %s", strerror(errno));
  }
}

static void noop_handler(int sig)
{
}

static void install_signal_handlers()
{
  trap_signal(SIGPIPE, signal_exit);
  detach_children(noop_handler);
}

static int listen_socket(const char *port)
{
  struct addrinfo hints = { 0 };
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  struct addrinfo *res;
  int err = getaddrinfo(NULL, port, &hints, &res);
  if (err) {
    log_exit(gai_strerror(err));
  }
  for (struct addrinfo *ai = res; ai; ai = ai->ai_next) {
    int sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sock < 0)
      continue;
    if (bind(sock, ai->ai_addr, ai->ai_addrlen) < 0 || listen(sock, MAX_BACKLOG) < 0) {
      close(sock);
      continue;
    }
    freeaddrinfo(res);
    return sock;
  }
  log_exit("listen_socket failed");
}

static void become_daemon()
{
}

static void upcase(char *s)
{
  for (char *c = s; *c; c++) {
    *c = toupper(*c);
  }
}

static void read_request_line(struct HttpRequest *req, FILE *in)
{
  char buf[1024];
  if (!fgets(buf, sizeof(buf), in)) {
    log_exit("fgets(3) failed");
  }
  char *p = strchr(buf, ' ');
  if (!p) {
    log_exit("parse error on read_request_line: %s", buf);
  }
  *p++ = '\0';
  req->method = malloc(p - buf);
  strcpy(req->method, buf);
  upcase(req->method);
  char *path = p;
  p = strchr(path, ' ');
  if (!p) {
    log_exit("parse error on read_request_line: %s", path);
  }
  *p++ = '\0';
  req->path = malloc(p - path);
  strcpy(req->path, path);
  if (strncasecmp(p, HTTP_VERSION_PREFIX, sizeof(HTTP_VERSION_PREFIX) - 1) != 0) {
    log_exit("parse error on read_request_line: %s", p);
  }
  req->protocol_minor_version = atoi(p + sizeof(HTTP_VERSION_PREFIX) - 1);
}

static int split(char *s, char c)
{
  int cnt = 1;
  for (char *p = s; *p; p++) {
    if (*p == c) {
      *p = '\0';
      cnt++;
    }
  }
  return cnt;
}

static char *trim(char *s)
{
  while (*s && isspace(*s))
    s++;
  for (size_t i = strlen(s) - 1; 0 <= i; i--) {
    if (isspace(s[i])) {
      s[i] = '\0';
    } else {
      break;
    }
  }
  return s;
}

static struct HttpHeaderField *read_header_field(FILE *in)
{
  char buf[1024];
  if (!fgets(buf, sizeof(buf), in)) {
    log_exit("fgets(3) failed");
  }
  if (buf[0] == '\n' || strcmp(buf, "\r\n") == 0)
    return NULL;
  if (split(buf, ':') != 2) {
    log_exit("parse error on read_header_field");
  }
  char *value = trim(buf + strlen(buf) + 1);
  char *name = trim(buf);
  struct HttpHeaderField *f = malloc(sizeof(struct HttpHeaderField));
  f->name = malloc(strlen(name) + 1);
  strcpy(f->name, name);
  f->value = malloc(strlen(value) + 1);
  strcpy(f->value, value);
  return f;
}

static char *lookup_header(struct HttpHeaderField *restrict headers, const char *restrict name)
{
  for (struct HttpHeaderField *h = headers; h; h = h->next) {
    if (strcasecmp(h->name, name) == 0)
      return h->value;
  }
  return NULL;
}

static struct HttpRequest *read_request(FILE *in)
{
  struct HttpRequest *req = malloc(sizeof(struct HttpRequest));
  read_request_line(req, in);
  req->headers = NULL;
  struct HttpHeaderField *h;
  while ((h = read_header_field(in)) != NULL) {
    h->next = req->headers;
    req->headers = h;
  }
  char *content_length = lookup_header(req->headers, "Content-Length");
  req->length = content_length == NULL ? 0 : atol(content_length);
  if (req->length == 0) {
    req->body = NULL;
  } else {
    if (MAX_REQUEST_CONTENT_LENGTH < req->length) {
      log_exit("request body too long");
    }
    req->body = malloc(req->length);
    if (fread(req->body, req->length, 1, in) < 1) {
      log_exit("fread(3) failed");
    }
  }
  return req;
}

static void log_request(const struct HttpRequest *req)
{
  fprintf(stderr, "HttpRequest {\n");
  fprintf(stderr, "  protocol_minor_version: %d\n", req->protocol_minor_version);
  fprintf(stderr, "  method: %s\n", req->method);
  fprintf(stderr, "  path: %s\n", req->path);
  fprintf(stderr, "  headers: [\n");
  for (struct HttpHeaderField *f = req->headers; f; f = f->next) {
    fprintf(stderr, "    {name: %s, value: %s}\n", f->name, f->value);
  }
  fprintf(stderr, "  ]\n");
  fprintf(stderr, "  length: %ld\n", req->length);
  if (req->body != NULL)
    fprintf(stderr, "  body: %s\n", req->body);
  fprintf(stderr, "}\n");
}

static struct FileInfo *get_fileinfo(const char *restrict docroot, const char *restrict urlpath)
{
  struct FileInfo *fi = malloc(sizeof(struct FileInfo));
  fi->path = malloc(strlen(docroot) + strlen(urlpath) + 2);
  sprintf(fi->path, "%s/%s", docroot, urlpath);
  fi->size = 0;
  fi->exists = false;
  struct stat st;
  if (lstat(fi->path, &st) < 0 || !S_ISREG(st.st_mode))
    return fi;
  fi->size = st.st_size;
  fi->exists = true;
  return fi;
}

static void free_fileinfo(struct FileInfo *fi)
{
  free(fi->path);
  free(fi);
}

static void output_common_headers(const struct HttpRequest *restrict req, FILE *restrict out,
                                  const char *restrict status)
{
  time_t t = time(NULL);
  struct tm *tm = gmtime(&t);
  if (!tm) {
    log_exit("gmtime(3) failed");
  }
  char date[30];
  strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S GMT", tm);
  fprintf(out, "HTTP/1.%d %s\r\n", req->protocol_minor_version, status);
  fprintf(out, "Date: %s\r\n", date);
  fprintf(out, "Server: %s/%s\r\n", SERVER_NAME, SERVER_VER);
  fprintf(out, "Connection: close\r\n");
}

static void not_found(const struct HttpRequest *req, FILE *out)
{
  output_common_headers(req, out, "404 Not Found");
  fprintf(out, "Content-Type: text/html\r\n");
  fprintf(out, "\r\n");
  if (strcmp(req->method, "HEAD") == 0)
    return;
  fprintf(out, "<!DOCTYPE html>\n");
  fprintf(out, "<html>");
  fprintf(out, "<head><title>Not Found</title></head>");
  fprintf(out, "<body><h1>404: Not Found</h1></body>");
  fprintf(out, "</html>\n");
}

static char *guess_content_type(const struct FileInfo *fi)
{
  return "text/plain";
}

static void do_file_response(const struct HttpRequest *restrict req, FILE *restrict out,
                             const char *restrict docroot)
{
  struct FileInfo *fi = get_fileinfo(docroot, req->path);
  if (!fi->exists) {
    free_fileinfo(fi);
    not_found(req, out);
    return;
  }
  output_common_headers(req, out, "200 OK");
  fprintf(out, "Content-Length: %ld\r\n", fi->size);
  fprintf(out, "Content-Type: %s\r\n", guess_content_type(fi));
  fprintf(out, "\r\n");
  if (strcmp(req->method, "HEAD") != 0) {
    int fd = open(fi->path, O_RDONLY);
    if (fd < 0) {
      log_exit("open(2) failed: %s", strerror(errno));
    }
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) != 0) {
      if (n < 0) {
        log_exit("read(2) failed: %s", strerror(errno));
      }
      if (fwrite(buf, 1, n, out) < n) {
        log_exit("write(3) failed");
      }
    }
    close(fd);
  }
  free_fileinfo(fi);
}

static void method_not_allowed(const struct HttpRequest *req, FILE *out)
{
  output_common_headers(req, out, "405 Method Not Allowed");
  fprintf(out, "Content-Type: text/html\r\n");
  fprintf(out, "\r\n");
  fprintf(out, "<!DOCTYPE html>\n");
  fprintf(out, "<html>");
  fprintf(out, "<head><title>Method Not Allowed</title></head>");
  fprintf(out, "<body><h1>405: Method Not Allowed</h1></body>");
  fprintf(out, "</html>\n");
}

static void not_implemented(const struct HttpRequest *req, FILE *out)
{
  output_common_headers(req, out, "501 Not Implemented");
  fprintf(out, "Content-Type: text/html\r\n");
  fprintf(out, "\r\n");
  fprintf(out, "<!DOCTYPE html>\n");
  fprintf(out, "<html>");
  fprintf(out, "<head><title>Not Implemented</title></head>");
  fprintf(out, "<body><h1>501: Not Implemented</h1></body>");
  fprintf(out, "</html>\n");
}

static void respond_to(const struct HttpRequest *restrict req, FILE *restrict out,
                       const char *restrict docroot)
{
  if (strcmp(req->method, "GET") == 0) {
    do_file_response(req, out, docroot);
  } else if (strcmp(req->method, "HEAD") == 0) {
    do_file_response(req, out, docroot);
  } else if (strcmp(req->method, "POST") == 0) {
    method_not_allowed(req, out);
  } else {
    not_implemented(req, out);
  }
  fflush(out);
}

static void free_request(struct HttpRequest *req)
{
  free(req->method);
  free(req->path);
  free(req->body);
  struct HttpHeaderField *h = req->headers;
  while (h) {
    struct HttpHeaderField *i = h;
    h = h->next;
    free(i->name);
    free(i->value);
    free(i);
  }
  free(req);
}

static void service(FILE *restrict in, FILE *restrict out, const char *restrict docroot)
{
  struct HttpRequest *req = read_request(in);
  log_request(req);
  respond_to(req, out, docroot);
  free_request(req);
}

static void server_main(int fd, const char *docroot)
{
  while (true) {
    struct sockaddr addr;
    socklen_t addrlen;
    int sock = accept(fd, &addr, &addrlen);
    if (sock < 0) {
      log_exit("accept(2) failed: %s", strerror(errno));
    }
    pid_t pid = fork();
    if (pid < 0) {
      log_exit("fork(2) failed: %s", strerror(errno));
    }
    if (pid == 0) {
      FILE *inf = fdopen(sock, "r");
      FILE *outf = fdopen(sock, "w");
      service(inf, outf, docroot);
      exit(EXIT_SUCCESS);
    }
    close(sock);
  }
}

int main(int argc, char *argv[])
{
  bool chroot = false;
  char *group = NULL;
  char *user = NULL;
  char *port = DEFAULT_PORT;
  int opt;
  while ((opt = getopt(argc, argv, "Dcg:hp:u:")) != -1) {
    switch (opt) {
    case 'D':
      debug_mode = true;
      break;
    case 'c':
      chroot = true;
      break;
    case 'g':
      group = optarg;
      break;
    case 'h':
      usage(stdout, argv[0]);
      exit(EXIT_SUCCESS);
    case 'p':
      group = optarg;
      break;
    case 'u':
      group = optarg;
      break;
    case '?':
      usage(stderr, argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if (optind != argc - 1) {
    usage(stderr, argv[0]);
    exit(EXIT_FAILURE);
  }
  char *docroot = argv[optind];
  if (chroot) {
    setup_environment(docroot, group, user);
    docroot = "";
  }
  install_signal_handlers();
  int fd = listen_socket(port);
  if (!debug_mode) {
    openlog(SERVER_NAME, LOG_PID | LOG_NDELAY, LOG_DAEMON);
    become_daemon();
  }
  server_main(fd, docroot);
  return EXIT_SUCCESS;
}
