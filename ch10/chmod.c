#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
  if (argc < 3) {
    fprintf(stderr, "Usage: %s MODE PATH...\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  long mode = strtol(argv[1], NULL, 8);
  for (int i = 2; i < argc; i++) {
    if (chmod(argv[i], mode)) {
      perror(argv[i]);
      exit(EXIT_FAILURE);
    }
  }
  return EXIT_SUCCESS;
}
