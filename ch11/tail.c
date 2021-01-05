#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LINESIZE 1024

static void usage(FILE *dest, const char *cmd)
{
  fprintf(dest, "Usage: %s [-n LINES] [FILE...]\n", cmd);
}

static void do_tail(FILE *f, long n)
{
  if (n <= 0)
    return;
  char **buf = calloc(n, sizeof(char *));
  for (int i = 0; i < n; i++) {
    buf[i] = calloc(LINESIZE, sizeof(char));
  }
  long i = 0;
  while (fgets(buf[i % n], LINESIZE, f))
    i++;
  for (int j = i % n; j < n; j++) {
    fputs(buf[j], stdout);
  }
  for (int j = 0; j < i % n; j++) {
    fputs(buf[j], stdout);
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
    do_tail(stdin, n);
  } else {
    for (int i = optind; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (!f) {
        perror(argv[i]);
        exit(EXIT_FAILURE);
      }
      do_tail(f, n);
      fclose(f);
    }
  }
  return EXIT_SUCCESS;
}
