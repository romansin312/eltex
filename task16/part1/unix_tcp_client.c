#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_tcp_socket"

int main() {
    int sockfd;
    struct sockaddr_un addr;
    char buffer[100];
    
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    
    connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
    
    send(sockfd, "Hi", 3, 0);
    printf("Sent: Hi\n");
    
    recv(sockfd, buffer, sizeof(buffer), 0);
    printf("Received: %s\n", buffer);
    
    close(sockfd);
    
    return 0;
}