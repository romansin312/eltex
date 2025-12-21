#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sockfd;
    struct sockaddr_in addr;
    char buffer[100];
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    
    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    
    send(sockfd, "Hi", 3, 0);
    printf("Sent: Hi\n");
    
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("Received: %s\n", buffer);
    
    close(sockfd);
    
    return 0;
}