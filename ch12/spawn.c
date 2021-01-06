#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
  if (argc == 1) {
    fprintf(stderr, "Usage: %s COMMAND [ARGS...]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  pid_t pid = fork();
  if (pid < 0) {
    perror(NULL);
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    char **args = calloc(argc, sizeof(char *));
    for (int i = 1; i < argc; i++) {
      args[i - 1] = argv[i];
    }
    args[argc - 1] = NULL;
    execv(argv[1], args);
    perror(argv[1]);
    exit(EXIT_FAILURE);
  }
  int stat;
  waitpid(pid, &stat, 0);
  printf("child (PID=%d) finished; ", pid);
  if (WIFEXITED(stat)) {
    printf("exit, status=%d\n", WEXITSTATUS(stat));
  } else if (WIFSIGNALED(stat)) {
    printf("signal, sig=%d\n", WTERMSIG(stat));
  } else {
    printf("abnormal exit\n");
  }
  return EXIT_SUCCESS;
}
