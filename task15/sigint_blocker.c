#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main() {
    sigset_t block_set;
    
    sigemptyset(&block_set);
    sigaddset(&block_set, SIGINT);
    
    if (sigprocmask(SIG_BLOCK, &block_set, NULL) == -1) {
        printf("sigprocmask error\n");
        return 1;
    }
    
    printf("Sigint bloker is running (PID: %d)\n", getpid());
    printf("The SIGINT is blocked\n");
    
    while(1) {
        printf("Working...\n");
        sleep(2);
    }
    
    return 0;
}