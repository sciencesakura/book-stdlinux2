#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>

static void do_ls(const char *path)
{
  DIR *d = opendir(path);
  if (!d) {
    perror(path);
    exit(EXIT_FAILURE);
  }
  struct dirent *e;
  while ((e = readdir(d)) != NULL) {
    puts(e->d_name);
  }
  closedir(d);
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s PATH...\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  for (int i = 1; i < argc; i++) {
    do_ls(argv[i]);
  }
  return EXIT_SUCCESS;
}
