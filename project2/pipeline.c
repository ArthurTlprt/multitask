#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#define READ_END 0
#define WRITE_END 1


void run_pipeline(char *commands[7][5], int in_fd, int n_task) {
  if(n_task == 6) { // derniere commande
    dup2(in_fd, STDIN_FILENO); // je lis l'entree du pipe
    execvp(commands[n_task][0], commands[n_task]);
    return;
  }
  int fd[2]; // nouveau pipe
  pipe(fd);
  switch(fork()){
      case -1: // ERROR
          break;
      case 0: // Child
          close(fd[READ_END]);
          dup2(in_fd, STDIN_FILENO); // lit le pipe precedant
          dup2(fd[WRITE_END], STDOUT_FILENO); //  ecrit dans le nouveau pipe
          execvp(commands[n_task][0], commands[n_task]);
          exit(EXIT_FAILURE);
          break;
      default: // Parent
          close(fd[WRITE_END]); // plus besoin
          close(in_fd); // plus besoin
          run_pipeline(commands,fd[READ_END],n_task+1); // on passe la sortie du nouveau pipe au suivant
          break;
    }
}


int main(int argc, char *argv[]) {

  char *commands[7][5] = {
    {"cat", "big.txt", NULL},
    {"tr", "-s", "[:digit:]", " ", NULL},
    {"tr", "[A-Z]", "[a-z]", NULL},
    {"tr", "-s", "[:punct:]", " ", NULL},
    {"tr", "-s", "\n\f\t\r ", "\n", NULL},
    {"sort", NULL},
    {"uniq", "-c", NULL}
  };

  run_pipeline(commands, STDIN_FILENO, 0);

  return 0;
}
