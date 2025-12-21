#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081

int main() {
    int sockfd;
    struct sockaddr_in addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[100];
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    printf("INET UDP Server is listening on port %d...\n", PORT);
    
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
    printf("Received: %s\n", buffer);
    
    sendto(sockfd, "Hello", 6, 0, (struct sockaddr*)&client_addr, client_len);
    
    close(sockfd);
    
    return 0;
}