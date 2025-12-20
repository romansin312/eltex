#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main() {
    sigset_t wait_set;
    int sig;
    
    sigemptyset(&wait_set);
    sigaddset(&wait_set, SIGUSR1);
    
    if (sigprocmask(SIG_BLOCK, &wait_set, NULL) == -1) {
        printf("sigprocmask error\n");
        return 1;
    }
    
    printf("Sigwait_loop is running (PID: %d)\n", getpid());
    printf("Waiting for SIGUSR1...\n");
    
    while(1) {
        printf("Working...\n");
        
        if (sigwait(&wait_set, &sig) != 0) {
            printf("sigwait error\n");
            return 1;
        }
        
        if (sig == SIGUSR1) {
            printf("SIGUSR1 has been received\n");
        }
    }
    
    return 0;
}