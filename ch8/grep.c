#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void usage(FILE *dest, const char *cmd)
{
  fprintf(dest, "Usage: %s [-iv] PATTERN [FILES...]\n", cmd);
}

static void do_grep(const regex_t *pat, FILE *f, bool invert_match)
{
  char buf[4096];
  while (fgets(buf, sizeof(buf), f)) {
    if (regexec(pat, buf, 0, NULL, 0) == 0) {
      if (!invert_match) {
        fputs(buf, stdout);
      }
    } else if (invert_match) {
      fputs(buf, stdout);
    }
  }
}

int main(int argc, char *argv[])
{
  bool ignore_case = false;
  bool invert_match = false;
  int opt;
  while ((opt = getopt(argc, argv, "hiv")) != -1) {
    switch (opt) {
    case 'h':
      usage(stdout, argv[0]);
      exit(EXIT_SUCCESS);
    case 'i':
      ignore_case = true;
      break;
    case 'v':
      invert_match = true;
      break;
    case '?':
      usage(stderr, argv[0]);
      exit(EXIT_FAILURE);
    }
  }
  if (optind == argc) {
    usage(stderr, argv[0]);
    exit(EXIT_FAILURE);
  }
  int cflags = REG_EXTENDED | REG_NOSUB | REG_NEWLINE;
  if (ignore_case) {
    cflags |= REG_ICASE;
  }
  regex_t pat;
  int err = regcomp(&pat, argv[optind], cflags);
  if (err) {
    char buf[1024];
    regerror(err, &pat, buf, sizeof(buf));
    fprintf(stderr, "%s\n", buf);
    exit(EXIT_FAILURE);
  }
  if (optind + 1 == argc) {
    do_grep(&pat, stdin, invert_match);
  } else {
    for (int i = optind + 1; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (!f) {
        perror(argv[i]);
        exit(EXIT_FAILURE);
      }
      do_grep(&pat, f, invert_match);
      fclose(f);
    }
  }
  return EXIT_SUCCESS;
}
