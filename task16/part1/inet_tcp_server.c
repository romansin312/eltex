#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, client_fd;
    struct sockaddr_in addr;
    char buffer[100];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    
    listen(server_fd, 5);
    printf("INET TCP Server is listening on port %d...\n", PORT);
    
    client_fd = accept(server_fd, NULL, NULL);
    
    recv(client_fd, buffer, sizeof(buffer), 0);
    printf("Received: %s\n", buffer);
    
    send(client_fd, "Hello", 6, 0);
    
    close(client_fd);
    close(server_fd);
    
    return 0;
}