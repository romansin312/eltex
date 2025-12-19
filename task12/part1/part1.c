#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pfd[2];
    char buffer[256];
    
    if (pipe(pfd) == -1) {
        printf("pipe creation error\n");
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        printf("fork error\n");
        return 1;
    }
    
    if (pid > 0) {
        close(pfd[0]);
        
        const char* message = "Hi!";
        write(pfd[1], message, strlen(message) + 1);
        
        close(pfd[1]);
        
        wait(NULL);
    }
    else {
        close(pfd[1]);
        
        ssize_t bytes_count = read(pfd[0], buffer, sizeof(buffer));
        if (bytes_count > 0) {
            printf("Received message: %s\n", buffer);
        }
        
        close(pfd[0]);
        
        return 0;
    }
    
    return 0;
}