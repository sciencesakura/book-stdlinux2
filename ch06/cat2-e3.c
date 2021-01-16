#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#define BUF_SIZE 2048

static noreturn void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

static void do_cat(FILE *f)
{
  unsigned char buf[BUF_SIZE];
  size_t n;
  do {
    n = fread(buf, 1, sizeof(buf), f);
    if (ferror(f) || fwrite(buf, 1, n, stdout) < n) {
      die(NULL);
    }
  } while (n == sizeof(buf));
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    do_cat(stdin);
  } else {
    for (int i = 1; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (!f) {
        die(argv[i]);
      }
      do_cat(f);
      if (fclose(f) == EOF) {
        die(argv[i]);
      }
    }
  }
  return EXIT_SUCCESS;
}
