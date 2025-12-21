#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SERVER_PATH "/tmp/unix_udp_socket"
#define CLIENT_PATH "/tmp/unix_udp_client"

int main() {
    int sockfd;
    struct sockaddr_un server_addr, client_addr;
    char buffer[100];
    
    sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
    
    unlink(CLIENT_PATH);
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, CLIENT_PATH);
    bind(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr));
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, SERVER_PATH);
    
    sendto(sockfd, "Hi", 3, 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Sent: Hi\n");
    
    recvfrom(sockfd, buffer, sizeof(buffer), 0, NULL, NULL);
    printf("Received: %s\n", buffer);
    
    close(sockfd);
    unlink(CLIENT_PATH);
    
    return 0;
}