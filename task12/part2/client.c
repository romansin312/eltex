#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define FIFO_NAME "/tmp/task12fifo"

int main() {
    char buffer[256];
    
    int fd = open(FIFO_NAME, O_RDONLY);
    if (fd == -1) {
        printf("fifo open error\n");
        return 1;
    }
    
    ssize_t bytes_count = read(fd, buffer, sizeof(buffer));
    if (bytes_count > 0) {
        printf("Recevied message: %s\n", buffer);
    }
    
    close(fd);
    
    if (unlink(FIFO_NAME) == -1) {
        printf("fifo unlink error\n");
        return 1;
    }
    
    printf("Fifo has been unlinked sucessfully\n");
    
    return 0;
}