#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sigusr1_handler(int signum) {
    printf("Signal SIGUSR1 is received. Signum: %d\n", signum);
}

int main() {
    struct sigaction sa;
    
    sa.sa_handler = sigusr1_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        printf("sigaction error\n");
        return 1;
    }
    
    printf("Receiver is running, PID: %d\n", getpid());
    printf("Waiting for SIGUSR1...\n");
    
    while(1) {
        pause();
    }
    
    return 0;
}