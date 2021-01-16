#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(FILE *dest, const char *cmd)
{
  fprintf(dest, "Usage: %s [-iv] PATTERN [FILES...]\n", cmd);
}

static void do_grep(const regex_t *pat, FILE *f)
{
  char buf[4096];
  while (fgets(buf, sizeof(buf), f)) {
    regmatch_t mat[1];
    if (regexec(pat, buf, 1, mat, 0) == 0) {
      char s[4096];
      regoff_t len = mat[0].rm_eo - mat[0].rm_so;
      memcpy(s, buf + mat[0].rm_so, len);
      s[len] = '\n';
      s[len + 1] = '\0';
      fputs(s, stdout);
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    usage(stderr, argv[0]);
    exit(EXIT_FAILURE);
  }
  regex_t pat;
  int err = regcomp(&pat, argv[1], REG_EXTENDED | REG_NEWLINE);
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
