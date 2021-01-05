#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>

#define MSG_BEF "before rename"
#define MSG_AFT "after rename"

static void noreturn die(const char *s)
{
  perror(s);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  if (argc != 3) {
    exit(EXIT_FAILURE);
  }
  char buf[2048];
  int fd = open(argv[1], O_WRONLY);
  if (fd < 0) {
    sprintf(buf, "open(\"%s\")", argv[1]);
    die(buf);
  }
  if (write(fd, MSG_BEF "\n", sizeof(MSG_BEF) + 1) < 0) {
    die(MSG_BEF);
  }
  if (rename(argv[1], argv[2])) {
    sprintf(buf, "rename(\"%s\", \"%s\")", argv[1], argv[2]);
    die(buf);
  }
  if (write(fd, MSG_AFT "\n", sizeof(MSG_AFT) + 1) < 0) {
    die(MSG_AFT);
  }
  if (close(fd)) {
    sprintf(buf, "close(\"%s\")", argv[1]);
    die(buf);
  }
  return EXIT_SUCCESS;
}
