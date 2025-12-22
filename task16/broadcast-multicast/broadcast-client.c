#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BROADCAST_PORT 8888

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[1024];
    int reuse_enable = 1;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation error\n");
        return 1;
    }
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
                   &reuse_enable, sizeof(reuse_enable)) < 0) {
        printf("setsockopt error\n");
        close(sockfd);
        return 1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BROADCAST_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind failed\n");
        close(sockfd);
        return 1;
    }
    
    printf("Broadcast client is running. Waiting for messages...\n");
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        
        int recv_len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&client_addr, &client_len);
        
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Received message: %s from %s:%d\n", 
                   buffer, 
                   inet_ntoa(client_addr.sin_addr), 
                   ntohs(client_addr.sin_port));
        }
    }
    
    close(sockfd);
    return 0;
}