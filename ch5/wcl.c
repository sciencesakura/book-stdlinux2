#include <fcntl.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
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

static uint_fast64_t do_wcl(int fd)
{
  uint_fast64_t cnt = 0;
  ssize_t n;
  unsigned char buf[BUF_SIZE];
  while ((n = read(fd, buf, sizeof(buf))) != 0) {
    if (n < 0) {
      exit(EXIT_FAILURE);
    }
    for (ssize_t i = 0; i < n; i++) {
      if (buf[i] == '\n')
        cnt++;
    }
  }
  return cnt;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("%" PRIuFAST64 "\n", do_wcl(STDIN_FILENO));
  } else {
    uint_fast64_t tot = 0;
    for (int i = 1; i < argc; i++) {
      int fd = open(argv[i], O_RDONLY);
      if (fd < 0) {
        die(argv[i]);
      }
      uint_fast64_t cnt = do_wcl(fd);
      printf("%4" PRIuFAST64 " %s\n", cnt, argv[i]);
      tot += cnt;
      if (close(fd) < 0) {
        die(argv[i]);
      }
    }
    printf("%4" PRIuFAST64 " total\n", tot);
  }
  return EXIT_SUCCESS;
}
