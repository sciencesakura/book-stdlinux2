#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void prompt()
{
  if (fputs("> ", stdout) == EOF) {
    perror(NULL);
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  char input[1024];
  char *cmd[1024];
  prompt();
  while (fgets(input, sizeof(input), stdin) != NULL) {
    pid_t pid = fork();
    if (pid < 0) {
      perror(NULL);
    } else if (pid == 0) {
      input[strlen(input) - 1] = '\0';
      int i = 0;
      char *s = strtok(input, " ");
      do {
        cmd[i++] = s;
      } while ((s = strtok(NULL, " ")) != NULL);
      cmd[i] = NULL;
      execv(cmd[0], cmd);
      perror(cmd[0]);
    } else {
      int stat;
      if (waitpid(pid, &stat, 0) < 0) {
        perror(NULL);
      }
    }
    prompt();
  }
  return EXIT_SUCCESS;
}
