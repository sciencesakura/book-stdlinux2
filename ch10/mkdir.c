#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s PATH...\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  for (int i = 1; i < argc; i++) {
    if (mkdir(argv[i], 0777)) {
      perror(argv[i]);
      exit(EXIT_FAILURE);
    }
  }
  return EXIT_SUCCESS;
}
