#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("Parent pid: %d, Parent ppid: %d\n", getpid(), getppid());

    pid_t child_pid = fork();

    if (child_pid == 0) {
        printf("Child pid: %d, Child ppid: %d\n", getpid(), getppid());
        exit(EXIT_SUCCESS);
    } else if (child_pid > 0) {
        wait(NULL);
        printf("Child process %d terminated\n", child_pid);
    } else {
        printf("fork error");
    }
        
    return 0;
}