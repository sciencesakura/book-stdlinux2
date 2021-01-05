#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void putblk(int n)
{
  for (int i = 0; i < n; i++) {
    fputs("  ", stdout);
  }
}

static void do_tree(const char *path, int lv)
{
  puts(path);
  char pbuf[1024];
  int plen = strlen(path);
  strcpy(pbuf, path);
  char *ppath = pbuf + plen;
  if (pbuf[plen - 1] != '/') {
    pbuf[plen] = '/';
    ppath++;
  }
  DIR *dir = opendir(path);
  struct dirent *e;
  while ((e = readdir(dir)) != NULL) {
    if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0)
      continue;
    putblk(lv + 1);
    strcpy(ppath, e->d_name);
    struct stat st;
    lstat(pbuf, &st);
    if (S_ISDIR(st.st_mode) && !S_ISLNK(st.st_mode)) {
      do_tree(pbuf, lv + 1);
    } else {
      puts(e->d_name);
    }
  }
  closedir(dir);
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    do_tree(".", 0);
  } else {
    for (int i = 1; i < argc; i++) {
      do_tree(argv[i], 0);
    }
  }
  return EXIT_SUCCESS;
}
