#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static void do_ls(const char *path)
{
  DIR *d = opendir(path);
  if (!d) {
    perror(path);
    exit(EXIT_FAILURE);
  }
  int pathlen = strlen(path);
  char s[pathlen + 1024];
  memcpy(s, path, pathlen);
  s[pathlen] = '/';
  struct dirent *e;
  while ((e = readdir(d)) != NULL) {
    struct stat st;
    strcpy(s + (pathlen + 1), e->d_name);
    if (lstat(s, &st) < 0) {
      printf("%s\tUnknown\tUnknown\n", e->d_name);
    } else {
      struct passwd *passwd = getpwuid(st.st_uid);
      char *owner = passwd == NULL ? "Unknown" : passwd->pw_name;
      char mtime[20];
      strftime(mtime, sizeof(mtime), "%FT%T", localtime(&st.st_mtime));
      printf("%s\t%s\t%s\n", owner, e->d_name, mtime);
    }
  }
  closedir(d);
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    do_ls(".");
  } else {
    for (int i = 1; i < argc; i++) {
      do_ls(argv[i]);
    }
  }
  return EXIT_SUCCESS;
}
