#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

static char *filetype(mode_t mode)
{
  if (S_ISREG(mode))
    return "file";
  if (S_ISDIR(mode))
    return "directory";
  if (S_ISCHR(mode))
    return "chardev";
  if (S_ISBLK(mode))
    return "blockdev";
  if (S_ISFIFO(mode))
    return "fifo";
  if (S_ISLNK(mode))
    return "symlink";
  if (S_ISSOCK(mode))
    return "socket";
  return "unknown";
}

int main(int argc, char *argv[])
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s PATH\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  struct stat s;
  if (lstat(argv[1], &s)) {
    perror(argv[1]);
    exit(EXIT_FAILURE);
  }
  printf("type\t%o (%s)\n", s.st_mode & S_IFMT, filetype(s.st_mode));
  printf("mode\t%o\n", s.st_mode & ~S_IFMT);
  printf("dev\t%d\n", s.st_dev);
  printf("ino\t%llu\n", s.st_ino);
  printf("rdev\t%d\n", s.st_rdev);
  printf("nlink\t%hu\n", s.st_nlink);
  printf("uid\t%d\n", s.st_uid);
  printf("gid\t%d\n", s.st_gid);
  printf("size\t%lld\n", s.st_size);
  printf("blksize\t%d\n", s.st_blksize);
  printf("blocks\t%lld\n", s.st_blocks);
  printf("atime\t%s", ctime(&s.st_atimespec.tv_sec));
  printf("mtime\t%s", ctime(&s.st_mtimespec.tv_sec));
  printf("ctime\t%s", ctime(&s.st_ctimespec.tv_sec));
  return EXIT_SUCCESS;
}
