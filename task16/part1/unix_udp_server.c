#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_udp_socket"

int main() {
    int sockfd;
    struct sockaddr_un addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[100];
    
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    
    unlink(SOCKET_PATH);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    
    bind(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    printf("UNIX UDP Server waiting for messages...\n");
    
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
    printf("Received: %s\n", buffer);
    
    sendto(sockfd, "Hello", 6, 0, (struct sockaddr*)&client_addr, client_len);    
    printf("Sent: Hello\n");
    
    close(sockfd);
    unlink(SOCKET_PATH);
    
    return 0;
}