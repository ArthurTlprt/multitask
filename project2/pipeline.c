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


void run_pipeline(char *commands[7][4], int pipes[6][2], int n_task) {
  pid_t pid;
  pid = fork();

  if(pid==0) {
    if( n_task < 6) { // la derniere tache ne doit pas parler à une autre tache
      dup2(pipes[n_task][WRITE_END], STDOUT_FILENO);
      close(pipes[n_task][READ_END]);
      close(pipes[n_task][WRITE_END]);
      printf("command: %s talks to someone\n", commands[n_task][0]);
    }
    if( n_task > 0) { // Personne ne parle à la premiere tache
      dup2(pipes[n_task-1][READ_END], STDIN_FILENO);
      close(pipes[n_task-1][WRITE_END]);
      close(pipes[n_task-1][READ_END]);
      printf("command: %s listen to someone\n", commands[n_task][0]);
    }
    execvp(commands[n_task][0], commands[n_task]);
    printf("command: %s \n", commands[n_task][0]);
    exit(1);

  } else if(n_task != 6){
    run_pipeline(commands, pipes, n_task+1);
  } else {
    int status;
    // to do: close all pipelines
    printf("wait: \n");
    waitpid(pid, &status, 0);
  }
}

// int create_pipe(struct Task *tasks[7]) {
//   pid_t pid;
//   int fd[2];
//
//   pipe(fd);
//   pid = fork();
//
//   if(pid==0)
//   {
//     dup2(fd[WRITE_END], STDOUT_FILENO);
//     close(fd[READ_END]);
//     close(fd[WRITE_END]);
//     execvp(tasks[0][0], tasks[0]);
//     exit(1);
//   }
//   else
//   {
//     pid=fork();
//
//     if(pid==0)
//     {
//       dup2(fd[READ_END], STDIN_FILENO);
//       close(fd[WRITE_END]);
//       close(fd[READ_END]);
//       execvp(tasks[2][0], tasks[2]);
//       exit(1);
//     }
//     else
//     {
//       int status;
//       close(fd[READ_END]);
//       close(fd[WRITE_END]);
//       waitpid(pid, &status, 0);
//     }
//   }
//   return 0;
// }


int main(int argc, char *argv[]) {

  int pipes[6][2];

  for(int i=0; i < 6; i++) {
    pipe(pipes[i]);
  }

  char *commands[7][4] = {
    {"cat", "big.txt"},
    {"tr", "-s", "'[:digit:]'", "' '"},
    {"tr", "'[A-Z]'", "'[a-z]'"},
    {"tr", "-s", "'[:punct:]'", "' '"},
    {"tr", "-s", "'\n\f\t\r '", "'\n'"},
    {"sort"},
    {"uniq", "-c"}
  };

  run_pipeline(commands, pipes, 0);

  return 0;
}
