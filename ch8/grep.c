#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

static void do_grep(const regex_t *pat, FILE *f)
{
  char buf[4096];
  while (fgets(buf, sizeof(buf), f)) {
    if (!regexec(pat, buf, 0, NULL, 0)) {
      fputs(buf, stdout);
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s PATTERN [FILES...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  regex_t pat;
  int err = regcomp(&pat, argv[1], REG_EXTENDED | REG_NOSUB | REG_NEWLINE);
  if (err) {
    char buf[1024];
    regerror(err, &pat, buf, sizeof(buf));
    fprintf(stderr, "%s\n", buf);
    exit(EXIT_FAILURE);
  }
  if (argc == 2) {
    do_grep(&pat, stdin);
  } else {
    for (int i = 2; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (!f) {
        perror(argv[i]);
        exit(EXIT_FAILURE);
      }
      do_grep(&pat, f);
      fclose(f);
    }
  }
  return EXIT_SUCCESS;
}
