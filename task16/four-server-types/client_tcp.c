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
    char buffer[BUFFER_SIZE];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("socket error\n");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("connect error\n");
        exit(1);
    }
    
    printf("Connected to server %s:%d\n", SERVER_IP, PORT);
    
    for (int i = 0; i < 3; i++) {
        snprintf(buffer, BUFFER_SIZE, "Message from TCP client number %d\n", i + 1);
        
        write(sockfd, buffer, strlen(buffer));
        
        ssize_t bytes_read = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Server response: %s", buffer);
        }
        
        sleep(1);
    }
    
    close(sockfd);
    return 0;
}