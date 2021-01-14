#include <ctype.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <sys/errno.h>

#define MAX_REQUEST_CONTENT_LENGTH 4096 * 4096
#define HTTP_VERSION_PREFIX        "HTTP/1."

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

static void install_signal_handlers()
{
  trap_signal(SIGPIPE, signal_exit);
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
  char *p = buf;
  char *ss[3];
  int idx = 0;
  int len = 0;
  while (true) {
    if (!*p || isspace(*p)) {
      if (len != 0) {
        if (idx == 3) {
          log_exit("parse error on read_request_line: %s", buf);
        }
        ss[idx] = malloc(len + 1);
        memcpy(ss[idx], p - len, len);
        ss[idx++][len] = '\0';
        len = 0;
      }
      if (!*p)
        break;
    } else {
      len++;
    }
    p++;
  }
  upcase(ss[0]);
  req->method = ss[0];
  req->path = ss[1];
  if (strncasecmp(ss[2], HTTP_VERSION_PREFIX, sizeof(HTTP_VERSION_PREFIX) - 1) != 0) {
    log_exit("parse error on read_request_line: %s", buf);
  }
  req->protocol_minor_version = atoi(ss[2] + sizeof(HTTP_VERSION_PREFIX) - 1);
  free(ss[2]);
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

static void mallocpy(char **restrict dest, const char *restrict src)
{
  *dest = malloc(strlen(src) + 1);
  strcpy(*dest, src);
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
  mallocpy(&f->name, name);
  mallocpy(&f->value, value);
  return f;
}

static char *lookup_header(struct HttpHeaderField *headers, const char *name)
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
  printf("HttpRequest {\n");
  printf("  protocol_minor_version: %d\n", req->protocol_minor_version);
  printf("  method: %s\n", req->method);
  printf("  path: %s\n", req->path);
  printf("  headers: [\n");
  for (struct HttpHeaderField *f = req->headers; f; f = f->next) {
    printf("    {name: %s, value: %s}\n", f->name, f->value);
  }
  printf("  ]\n");
  printf("  length: %ld\n", req->length);
  if (req->body != NULL)
    printf("  body: %s\n", req->body);
  printf("}\n");
}

static void respond_to(struct HttpRequest *req, FILE *out, const char *docroot)
{
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

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <DOCROOT>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  install_signal_handlers();
  service(stdin, stdout, argv[1]);
  return EXIT_SUCCESS;
}
