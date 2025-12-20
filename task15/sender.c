#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <signal [INT or USR1]> <receiver PID>\n", argv[0]);
        return 1;
    }
    
    int signal;
    if (strcmp(argv[1], "USR1") == 0) {
        signal = SIGUSR1;
    } else if (strcmp(argv[1], "INT") == 0) {
        signal = SIGINT;
    } else {
        printf("Unsupported signal\n");
        return 1;
    }
    pid_t pid = atoi(argv[2]);

    printf("Sending %s to process with PID %d\n", argv[1], pid);
    
    if (kill(pid, signal) == -1) {
        printf("kill error\n");
        return 1;
    }
    
    printf("The signal has been sent successfully\n");
    
    return 0;
}