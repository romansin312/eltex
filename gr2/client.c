#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

unsigned short csum(unsigned short *buf, int nwords) {
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--) {
        sum += *buf++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[]) {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];
    
    int client_port = 8081;
    
    if (argc > 1) {
        client_port = atoi(argv[1]);
        printf("Using client port: %d\n", client_port);
    } else {
        printf("Using default client port: %d\n", client_port);
    }
    
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    if (sockfd < 0) {
        printf("socket error\n");
        exit(EXIT_FAILURE);
    }
    
    int on = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        printf("setsockopt error\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    
    printf("Echo client started\n");
    printf("Type messages to send (type 'exit' to quit):\n");
    
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (!fgets(message, BUFFER_SIZE, stdin)) {
            break;
        }
        
        size_t len = strlen(message);
        if (len > 0 && message[len - 1] == '\n') {
            message[len - 1] = '\0';
        }
        
        if (strcmp(message, "exit") == 0) {
            char close_msg[] = "CLOSE_CONNECTION";
            
            memset(buffer, 0, BUFFER_SIZE);
            
            struct iphdr *ip_header = (struct iphdr *)buffer;
            ip_header->ihl = 5;
            ip_header->version = 4;
            ip_header->tos = 0;
            ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(close_msg) + 1);
            ip_header->id = htons(getpid() & 0xFFFF);
            ip_header->frag_off = 0;
            ip_header->ttl = 64;
            ip_header->protocol = IPPROTO_UDP;
            ip_header->check = 0;
            ip_header->saddr = inet_addr("127.0.0.1");
            ip_header->daddr = server_addr.sin_addr.s_addr;
            
            ip_header->check = csum((unsigned short *)ip_header, sizeof(struct iphdr) / 2);
            
            struct udphdr *udp_header = (struct udphdr *)(buffer + sizeof(struct iphdr));
            udp_header->source = htons(client_port);
            udp_header->dest = htons(SERVER_PORT);
            udp_header->len = htons(sizeof(struct udphdr) + strlen(close_msg) + 1);
            udp_header->check = 0;
            
            char *data = buffer + sizeof(struct iphdr) + sizeof(struct udphdr);
            strcpy(data, close_msg);
            
            sendto(sockfd, buffer, ntohs(ip_header->tot_len), 0,
                  (struct sockaddr *)&server_addr, sizeof(server_addr));
            
            printf("Connection closed\n");
            break;
        }
        
        memset(buffer, 0, BUFFER_SIZE);
        
        struct iphdr *ip_header = (struct iphdr *)buffer;
        ip_header->ihl = 5;
        ip_header->version = 4;
        ip_header->tos = 0;
        ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(message) + 1);
        ip_header->id = htons(getpid() & 0xFFFF);
        ip_header->frag_off = 0;
        ip_header->ttl = 64;
        ip_header->protocol = IPPROTO_UDP;
        ip_header->check = 0;
        ip_header->saddr = inet_addr("127.0.0.1");
        ip_header->daddr = server_addr.sin_addr.s_addr;
        
        ip_header->check = csum((unsigned short *)ip_header, sizeof(struct iphdr) / 2);
        
        struct udphdr *udp_header = (struct udphdr *)(buffer + sizeof(struct iphdr));
        udp_header->source = htons(client_port);
        udp_header->dest = htons(SERVER_PORT);
        udp_header->len = htons(sizeof(struct udphdr) + strlen(message) + 1);
        udp_header->check = 0;
        
        char *data = buffer + sizeof(struct iphdr) + sizeof(struct udphdr);
        strcpy(data, message);
        
        printf("Sending: %s\n", message);
        
        ssize_t sent_len = sendto(sockfd, buffer, ntohs(ip_header->tot_len), 0,
                                 (struct sockaddr *)&server_addr, sizeof(server_addr));
        
        if (sent_len < 0) {
            printf("sendto error\n");
            continue;
        }
        
        printf("Sent %zd bytes, waiting for response...\n", sent_len);
        
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            
            ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
            
            if (recv_len < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    printf("Timeout waiting for response\n");
                } else {
                    printf("recvfrom error\n");
                }
                break;
            }
            
            struct iphdr *recv_ip_header = (struct iphdr *)buffer;
            int ip_header_len = recv_ip_header->ihl * 4;
            
            if (recv_len < ip_header_len + sizeof(struct udphdr)) {
                continue;
            }
            
            struct udphdr *recv_udp_header = (struct udphdr *)(buffer + ip_header_len);
            
            if (ntohs(recv_udp_header->source) == SERVER_PORT && 
                ntohs(recv_udp_header->dest) == client_port) {
                
                char *recv_data = buffer + ip_header_len + sizeof(struct udphdr);
                int data_len = recv_len - ip_header_len - sizeof(struct udphdr);
                
                if (data_len > 0) {
                    if (data_len >= BUFFER_SIZE - (recv_data - buffer)) {
                        data_len = BUFFER_SIZE - (recv_data - buffer) - 1;
                    }
                    recv_data[data_len] = '\0';
                    
                    printf("Server response: %s\n", recv_data);
                }
                break;
            }
        }
    }
    
    close(sockfd);
    return 0;
}