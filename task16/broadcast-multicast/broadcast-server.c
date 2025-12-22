#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BROADCAST_PORT 8888
#define BROADCAST_IP "255.255.255.255"

int main() {
    int sockfd;
    struct sockaddr_in broadcast_addr;
    int broadcast_enable = 1;
    char message[] = "hi";
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation error\n");
        return 1;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, 
                   &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        printf("setsockopt error\n");
        close(sockfd);
        return 1;
    }
    
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(BROADCAST_PORT);
    broadcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    
    printf("Broadcast server is running. Sending messages...\n");
    
    while (1) {
        if (sendto(sockfd, message, strlen(message), 0,
                   (struct sockaddr*)&broadcast_addr, 
                   sizeof(broadcast_addr)) < 0) {
            printf("Send error\n");
        } else {
            printf("The message %s has been sent\n", message);
        }
        
        sleep(2);
    }
    
    close(sockfd);
    return 0;
}