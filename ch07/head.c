#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(FILE *dest, const char *cmd)
{
  fprintf(dest, "Usage: %s [-n LINES] [FILE...]\n", cmd);
}

static void do_head(FILE *f, long n)
{
  if (n <= 0)
    return;
  int c;
  while ((c = fgetc(f)) != EOF) {
    if (putchar(c) == EOF) {
      perror(NULL);
      exit(EXIT_FAILURE);
    }
    if (c == '\n' && --n == 0)
      break;
  }
}

int main(int argc, char *argv[])
{
  long n = 10;
  int opt;
  while ((opt = getopt(argc, argv, "hn:")) != -1) {
    switch (opt) {
    case 'h':
      usage(stdout, argv[0]);
      exit(EXIT_SUCCESS);
    case 'n':
      n = atol(optarg);
      break;
    case '?':
      usage(stderr, argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if (optind == argc) {
    do_head(stdin, n);
  } else {
    for (int i = optind; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (!f) {
        perror(argv[i]);
        exit(EXIT_FAILURE);
      }
      do_head(f, n);
      fclose(f);
    }
  }
  return EXIT_SUCCESS;
}
