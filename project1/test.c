#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int foo(const char *whoami, int n) {
    printf("I am a %s number %d.  My pid is:%d  my ppid is %d\n", whoami, n, getpid(), getppid() );
    return 1;
}

int func(int n) {
    if (n == 0) {
        return 0;
    }
    int pid = fork();
    if (pid == -1) {
        exit(0);
    }
    if (pid==0) {
        foo("child", n);
        n = n-1;
        func(n);
        exit(0);
    }
    else {
       wait(NULL);
    }
    return 0;
}


int main() {
  func(8);
  return 0;
}
