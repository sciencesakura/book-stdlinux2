#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

static noreturn void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

static void do_cat(FILE *f)
{
  int c;
  while ((c = fgetc(f)) != EOF) {
    if (putchar(c) == EOF) {
      die(NULL);
    }
  }
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
