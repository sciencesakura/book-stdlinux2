#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

static noreturn void die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

static uint_fast64_t do_wcl(FILE *f)
{
  uint_fast64_t cnt = 0;
  int a;
  int b = '\n';
  while ((a = fgetc(f)) != EOF) {
    if (a == '\n')
      cnt++;
    b = a;
  }
  if (b != '\n')
    cnt++;
  return cnt;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("%" PRIuFAST64 "\n", do_wcl(stdin));
  } else {
    uint_fast64_t tot = 0;
    for (int i = 1; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (!f) {
        die(argv[i]);
      }
      uint_fast64_t cnt = do_wcl(f);
      printf("%4" PRIuFAST64 " %s\n", cnt, argv[i]);
      tot += cnt;
      if (fclose(f) == EOF) {
        die(argv[i]);
      }
    }
    printf("%4" PRIuFAST64 " total\n", tot);
  }
  return EXIT_SUCCESS;
}
