#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s [FILE...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  for (int i = 1; i < argc; i++) {
    if (unlink(argv[i])) {
      perror(argv[i]);
      exit(EXIT_FAILURE);
    }
  }
  return EXIT_SUCCESS;
}
