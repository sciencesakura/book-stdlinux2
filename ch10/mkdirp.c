#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void do_mkdirp(char *path)
{
  if (mkdir(path, 0777) == 0)
    return;
  if (errno == ENOENT) {
    int len = strlen(path);
    int idx = len;
    while (0 <= idx && path[idx] != '/')
      idx--;
    if (idx == -1) {
      exit(EXIT_FAILURE);
    }
    path[idx] = '\0';
    do_mkdirp(path);
    path[idx] = '/';
    if (mkdir(path, 0777)) {
      perror(path);
      exit(EXIT_FAILURE);
    }
  } else {
    perror(path);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s DIR...\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  for (int i = 1; i < argc; i++) {
    do_mkdirp(argv[i]);
  }
  return EXIT_SUCCESS;
}
