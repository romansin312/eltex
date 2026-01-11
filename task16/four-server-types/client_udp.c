#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("socket error\n");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    
    for (int i = 0; i < 3; i++) {
        snprintf(buffer, BUFFER_SIZE, "Message from UDP client number %d\n", i + 1);
        
        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr, server_len);
        
        ssize_t bytes_received = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                                         (struct sockaddr*)&server_addr, &server_len);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            printf("Server response: %s", buffer);
        }
        
        sleep(1);
    }
    
    close(sockfd);
    return 0;
}