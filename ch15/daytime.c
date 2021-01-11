#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
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

int main(int argc, char *argv[])
{
  int sock = open_connection(argc == 1 ? "localhost" : argv[1], "daytime");
  FILE *f = fdopen(sock, "r");
  if (!f) {
    perror("fdopen(3)");
    exit(EXIT_FAILURE);
  }
  char buf[2014];
  fgets(buf, sizeof(buf), f);
  fputs(buf, stdout);
  return EXIT_SUCCESS;
}
