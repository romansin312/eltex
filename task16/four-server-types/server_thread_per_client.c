#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    
    printf("Thread %lu: Client connected\n", pthread_self());
    
    while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("Thread %lu received: %s", pthread_self(), buffer);
        
        write(client_fd, buffer, bytes_read);
    }
    
    printf("Thread %lu: Client disconnected\n", pthread_self());
    close(client_fd);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("socket error\n");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind error\n");
        exit(1);
    }
    
    if (listen(server_fd, 10) < 0) {
        printf("listen error\n");
        exit(1);
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            printf("accept error\n");
            continue;
        }
        
        printf("New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));
        
        pthread_t thread;
        int* client_fd_ptr = malloc(sizeof(int));
        *client_fd_ptr = client_fd;
        
        if (pthread_create(&thread, NULL, handle_client, client_fd_ptr) != 0) {
            printf("pthread_create error\n");
            close(client_fd);
            free(client_fd_ptr);
        } else {
            pthread_detach(thread);
        }
    }
    
    close(server_fd);
    return 0;
}