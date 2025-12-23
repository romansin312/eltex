#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 8080
#define CLIENT_PORT 9999
#define BUFFER_SIZE 1024
#define PACKET_SIZE 4096
#define MESSAGE "hello"

int main() {    
    int raw_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char packet[PACKET_SIZE];
    
    if ((raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) < 0) {
        printf("create raw socket error\n");
        return 1;
    }
    
    int on = 1;
    if (setsockopt(raw_socket, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        printf("setsockopt error\n");
        close(raw_socket);
        return 1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDRESS, &server_addr.sin_addr);
    
    strcpy(buffer, MESSAGE);
    int data_len = strlen(buffer);
    
    struct iphdr *ip_header = (struct iphdr *)packet;
    
    ip_header->version = 4;
    ip_header->ihl = 5;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + data_len);
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;
    ip_header->check = 0;
    
    struct sockaddr_in local_addr;
    socklen_t len = sizeof(local_addr);
    getsockname(raw_socket, (struct sockaddr *)&local_addr, &len);
    
    ip_header->saddr = local_addr.sin_addr.s_addr;
    ip_header->daddr = server_addr.sin_addr.s_addr;
    
    struct udphdr *udp_header = (struct udphdr *)(packet + sizeof(struct iphdr));
    
    udp_header->source = htons(CLIENT_PORT);
    udp_header->dest = htons(SERVER_PORT);
    udp_header->len = htons(sizeof(struct udphdr) + data_len);
    udp_header->check = 0;
    
    char *data = packet + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(data, buffer, data_len);
    
    printf("Sending packet to server\n");
    
    if (sendto(raw_socket, packet, ntohs(ip_header->tot_len), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("sendto error\n");
        close(raw_socket);
        return 1;
    }
    
    printf("Packet has been sent. Waiting for response...\n");
    
    while (1) {
        char recv_buffer[PACKET_SIZE];
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        int recv_len = recvfrom(raw_socket, recv_buffer, PACKET_SIZE, 0, (struct sockaddr *)&from_addr, &from_len);
        
        if (recv_len < 0) {
            printf("recvfrom error\n\n");
            continue;
        }
        
        if (from_addr.sin_addr.s_addr != server_addr.sin_addr.s_addr) {
            printf("Packet is from unknown source, dropping\n");
            continue;
        }
        
        struct iphdr *recv_ip_header = (struct iphdr *)recv_buffer;
        
        if (recv_ip_header->protocol != IPPROTO_UDP) {
            printf("Not UDP packet, dropping...\n");
            continue;
        }
        
        struct udphdr *recv_udp_header = (struct udphdr *)(recv_buffer + (recv_ip_header->ihl * 4));
        
        if (ntohs(recv_udp_header->dest) != CLIENT_PORT) {
            printf("Packet port is %d, our port is %d, dropping...\n", ntohs(recv_udp_header->dest), CLIENT_PORT);
            continue;
        }
        
        char *recv_data = recv_buffer + (recv_ip_header->ihl * 4) + sizeof(struct udphdr);
        int recv_data_len = recv_len - (recv_ip_header->ihl * 4) - sizeof(struct udphdr);
        
        recv_data[recv_data_len] = '\0';
        
        printf("Response from server:\n");
        printf("Source IP: %s\n", inet_ntoa(from_addr.sin_addr));
        printf("Source Port: %d\n", ntohs(recv_udp_header->source));
        printf("Destination Port: %d\n", ntohs(recv_udp_header->dest));
        printf("Length: %d\n", ntohs(recv_udp_header->len));
        printf("Message: %s\n", recv_data);
        
        break;
    }
    
    close(raw_socket);
    return 0;
}