#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>

#define BUF_SIZE 2048

static noreturn void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

static void do_cat(const char *path)
{
  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    die(path);
  }
  ssize_t n;
  unsigned char buf[BUF_SIZE];
  while ((n = read(fd, buf, sizeof(buf))) != 0) {
    if (n < 0 || write(STDOUT_FILENO, buf, n) < 0) {
      die(path);
    }
  }
  if (close(fd) < 0) {
    die(path);
  }
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "%s: file name not given\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  for (int i = 1; i < argc; i++) {
    do_cat(argv[i]);
  }
  return EXIT_SUCCESS;
}
