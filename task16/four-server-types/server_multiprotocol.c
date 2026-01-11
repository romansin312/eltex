#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

int main() {
    int tcp_fd, udp_fd, max_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    fd_set read_fds;
    char buffer[BUFFER_SIZE];
    
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_fd < 0) {
        printf("TCP socket error\n");
        exit(1);
    }
    
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        printf("UDP socket error\n");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(tcp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("TCP bind error\n");
        exit(1);
    }
    
    if (bind(udp_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("UDP bind error\n");
        exit(1);
    }
    
    if (listen(tcp_fd, 10) < 0) {
        printf("listen error\n");
        exit(1);
    }
    
    printf("Multiprotocol server listening on port %d...\n", PORT);
    printf("Supports both TCP and UDP protocols\n");
    
    int client_fds[MAX_CLIENTS] = {0};
    
    while (1) {
        FD_ZERO(&read_fds);
        
        FD_SET(tcp_fd, &read_fds);
        max_fd = tcp_fd;
        
        FD_SET(udp_fd, &read_fds);
        if (udp_fd > max_fd) max_fd = udp_fd;
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_fds[i] > 0) {
                FD_SET(client_fds[i], &read_fds);
                if (client_fds[i] > max_fd) max_fd = client_fds[i];
            }
        }
        
        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        
        if (activity < 0) {
            printf("select error\n");
            continue;
        }
        
        if (FD_ISSET(tcp_fd, &read_fds)) {
            client_len = sizeof(client_addr);
            int new_client = accept(tcp_fd, (struct sockaddr*)&client_addr, &client_len);
            
            if (new_client < 0) {
                printf("accept error\n");
                continue;
            }
            
            printf("New TCP connection from %s:%d\n",
                   inet_ntoa(client_addr.sin_addr),
                   ntohs(client_addr.sin_port));
            
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (client_fds[i] == 0) {
                    client_fds[i] = new_client;
                    break;
                }
            }
        }
        
        if (FD_ISSET(udp_fd, &read_fds)) {
            client_len = sizeof(client_addr);
            ssize_t bytes_received = recvfrom(udp_fd, buffer, BUFFER_SIZE - 1, 0,
                                            (struct sockaddr*)&client_addr, &client_len);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                printf("UDP packet from %s:%d: %s",
                       inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port),
                       buffer);
                
                sendto(udp_fd, buffer, bytes_received, 0,
                       (struct sockaddr*)&client_addr, client_len);
            }
        }
        
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int client_fd = client_fds[i];
            
            if (client_fd > 0 && FD_ISSET(client_fd, &read_fds)) {
                ssize_t bytes_received = read(client_fd, buffer, BUFFER_SIZE - 1);
                
                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';
                    printf("TCP data from client %d: %s", client_fd, buffer);
                    
                    write(client_fd, buffer, bytes_received);
                } else {
                    printf("TCP client %d disconnected\n", client_fd);
                    close(client_fd);
                    client_fds[i] = 0;
                }
            }
        }
    }
    
    close(tcp_fd);
    close(udp_fd);
    return 0;
}