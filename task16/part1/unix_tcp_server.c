#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/unix_tcp_socket"

int main() {
    int server_fd, client_fd;
    struct sockaddr_un addr;
    char buffer[100];
    
    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    
    unlink(SOCKET_PATH);
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);
    
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    
    listen(server_fd, 5);
    printf("UNIX TCP Server waiting for connection...\n");
    
    client_fd = accept(server_fd, NULL, NULL);
    
    recv(client_fd, buffer, sizeof(buffer), 0);
    printf("Received: %s\n", buffer);
    
    send(client_fd, "Hello", 6, 0);
    printf("Sent: Hello\n");
    
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH);
    
    return 0;
}