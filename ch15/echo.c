#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int open_connection(const char *host, const char *serv)
{
  struct addrinfo hints = { 0 };
  struct addrinfo *res;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  int err = getaddrinfo(host, serv, &hints, &res);
  if (err != 0) {
    fprintf(stderr, "getaddrinfo(3): %s\n", gai_strerror(err));
    exit(EXIT_FAILURE);
  }
  for (struct addrinfo *i = res; i; i = i->ai_next) {
    int sock = socket(i->ai_family, i->ai_socktype, i->ai_protocol);
    if (sock < 0)
      continue;
    if (connect(sock, i->ai_addr, i->ai_addrlen) < 0) {
      close(sock);
      continue;
    }
    freeaddrinfo(res);
    return sock;
  }
  fprintf(stderr, "socket(2)/connect(2) failed\n");
  exit(EXIT_FAILURE);
}

static void prompt()
{
  if (fputs("> ", stdout) == EOF) {
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  fflush(stdout);
}

int main(int argc, char *argv[])
{
  int sock = open_connection(argc == 1 ? "localhost" : argv[1], "echo");
  FILE *f = fdopen(sock, "w+");
  if (!f) {
    perror("fdopen(3)");
    exit(EXIT_FAILURE);
  }
  prompt();
  char buf[2014];
  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    if (strcmp(buf, ":quit\n") == 0) {
      exit(EXIT_SUCCESS);
    }
    fputs(buf, f);
    fflush(f);
    fgets(buf, sizeof(buf), f);
    fputs(buf, stdout);
    prompt();
  }
  return EXIT_SUCCESS;
}
