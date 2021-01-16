#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 2048

static noreturn void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

static void do_cat(int fd)
{
  ssize_t n;
  unsigned char buf[BUF_SIZE];
  while ((n = read(fd, buf, sizeof(buf))) != 0) {
    if (n < 0 || write(STDOUT_FILENO, buf, n) < 0) {
      die(NULL);
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    do_cat(STDIN_FILENO);
  } else {
    for (int i = 1; i < argc; i++) {
      int fd = open(argv[i], O_RDONLY);
      if (fd < 0) {
        die(argv[i]);
      }
      do_cat(fd);
      if (close(fd) < 0) {
        die(argv[i]);
      }
    }
  }
  return EXIT_SUCCESS;
}
