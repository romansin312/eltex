#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define SERVER_PORT 8080

typedef struct {
    uint32_t ip;
    uint16_t port;
    int counter;
} client_state;

client_state clients[100];
int clients_count = 0;

int get_client_index(uint32_t ip, uint16_t port) {
    for (int i = 0; i < clients_count; i++) {
        if (clients[i].ip == ip && clients[i].port == port) {
            return i;
        }
    }
    return -1;
}

int add_client(uint32_t ip, uint16_t port) {
    if (clients_count >= 100) {
        return -1;
    }
    clients[clients_count].ip = ip;
    clients[clients_count].port = port;
    clients[clients_count].counter = 0;
    return clients_count++;
}

void remove_client(int index) {
    if (index < 0 || index >= clients_count) {
        return;
    }
    for (int i = index; i < clients_count - 1; i++) {
        clients[i] = clients[i + 1];
    }
    clients_count--;
}

unsigned short csum(unsigned short *buf, int nwords) {
    unsigned long sum;
    for (sum = 0; nwords > 0; nwords--) {
        sum += *buf++;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
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
    
    printf("Echo server started on port %d\n", SERVER_PORT);
    printf("Waiting for messages...\n");
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        
        ssize_t packet_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0,
                                     (struct sockaddr *)&client_addr, &addr_len);
        
        if (packet_len < 0) {
            printf("recvfrom error\n");
            continue;
        }
        
        struct iphdr *ip_header = (struct iphdr *)buffer;
        int ip_header_len = ip_header->ihl * 4;
        
        if (packet_len < ip_header_len + sizeof(struct udphdr)) {
            continue;
        }
        
        struct udphdr *udp_header = (struct udphdr *)(buffer + ip_header_len);
        
        if (ntohs(udp_header->dest) != SERVER_PORT) {
            continue;
        }
        
        char *data = buffer + ip_header_len + sizeof(struct udphdr);
        int data_len = packet_len - ip_header_len - sizeof(struct udphdr);
        
        if (data_len <= 0) {
            continue;
        }
        
        if (data_len >= BUFFER_SIZE - (data - buffer)) {
            data_len = BUFFER_SIZE - (data - buffer) - 1;
        }
        data[data_len] = '\0';
        
        uint32_t client_ip = ip_header->saddr;
        uint16_t client_port = ntohs(udp_header->source);
        
        printf("Received packet from %s:%d, data: %s\n",
               inet_ntoa(*(struct in_addr*)&client_ip), client_port, data);
        
        int client_idx = get_client_index(client_ip, client_port);
        if (strcmp(data, "CLOSE_CONNECTION") == 0) {
            if (client_idx != -1) {
                printf("Client %s:%d closed connection\n", 
                       inet_ntoa(*(struct in_addr*)&client_ip), client_port);
                remove_client(client_idx);
            }
            continue;
        }
        
        if (client_idx == -1) {
            client_idx = add_client(client_ip, client_port);
            printf("New client connected: %s:%d\n", 
                   inet_ntoa(*(struct in_addr*)&client_ip), client_port);
        }
        
        clients[client_idx].counter++;
        
        char response[BUFFER_SIZE];
        int response_len = snprintf(response, BUFFER_SIZE, "%s %d", data, clients[client_idx].counter);
        
        printf("Responding to %s:%d: %s\n",
               inet_ntoa(*(struct in_addr*)&client_ip), client_port, response);
        
        char send_buffer[BUFFER_SIZE];
        memset(send_buffer, 0, BUFFER_SIZE);
        
        struct iphdr *send_ip_header = (struct iphdr *)send_buffer;
        send_ip_header->ihl = 5;
        send_ip_header->version = 4;
        send_ip_header->tos = 0;
        send_ip_header->tot_len = htons(ip_header_len + sizeof(struct udphdr) + response_len + 1);
        send_ip_header->id = htons(54321);
        send_ip_header->frag_off = 0;
        send_ip_header->ttl = 64;
        send_ip_header->protocol = IPPROTO_UDP;
        send_ip_header->check = 0;
        send_ip_header->saddr = inet_addr("127.0.0.1");
        send_ip_header->daddr = client_ip;
        
        send_ip_header->check = csum((unsigned short *)send_ip_header, ip_header_len / 2);
        
        struct udphdr *send_udp_header = (struct udphdr *)(send_buffer + ip_header_len);
        send_udp_header->source = htons(SERVER_PORT);
        send_udp_header->dest = htons(client_port);
        send_udp_header->len = htons(sizeof(struct udphdr) + response_len + 1);
        send_udp_header->check = 0;
        
        char *send_data = send_buffer + ip_header_len + sizeof(struct udphdr);
        strcpy(send_data, response);
        
        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = client_ip;
        dest_addr.sin_port = htons(client_port);
        
        ssize_t sent_len = sendto(sockfd, send_buffer, 
                                 ntohs(send_ip_header->tot_len), 0,
                                 (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        
        if (sent_len < 0) {
            printf("sendto error\n");
        } else {
            printf("Sent %zd bytes to client\n", sent_len);
        }
    }
    
    close(sockfd);
    return 0;
}