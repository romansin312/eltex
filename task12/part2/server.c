#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/task12fifo"

int main() {
    if (mkfifo(FIFO_NAME, S_IRUSR | S_IWUSR) == -1) {
        printf("fifo creation error\n");
        return 1;
    }
    
    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd == -1) {
        printf("fifo open error\n");
        return 1;
    }
    
    const char* message = "Hi!";
    write(fd, message, strlen(message) + 1);
    
    close(fd);
    
    printf("The message has been sent\n");
    
    return 0;
}