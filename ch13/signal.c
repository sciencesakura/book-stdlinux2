#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void handler(int sig)
{
  printf("trap signal: %d\n", sig);
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
  struct sigaction act;
  act.sa_handler = handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = SA_RESTART;
  if (sigaction(SIGINT, &act, NULL) < 0) {
    perror(NULL);
    exit(EXIT_FAILURE);
  }
  pause();
  return EXIT_SUCCESS;
}
