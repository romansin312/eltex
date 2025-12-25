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
#include <linux/if_ether.h>
#include <net/ethernet.h>
#include <net/if_arp.h>

#define SERVER_ADDRESS "172.21.98.3"
#define CLIENT_ADDRESS "172.21.98.224"
#define SERVER_PORT 8080
#define CLIENT_PORT 9999
#define BUFFER_SIZE 1024
#define PACKET_SIZE 4096
#define MESSAGE "hello"
#define INTERFACE "eth0"
#define CLIENT_MAC {0x00, 0x15, 0x5d, 0xe4, 0x82, 0xe0}
#define SERVER_MAC {0x08, 0x00, 0x27, 0x63, 0xe2, 0x7e}

void fill_ip_header(char packet[PACKET_SIZE], struct in_addr client_ip, struct in_addr server_ip) {    
    struct iphdr *ip_header = (struct iphdr *)(packet + sizeof(struct ethhdr));
    
    ip_header->version = 4;
    ip_header->ihl = 5;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(MESSAGE));
    ip_header->ttl = 64;
    ip_header->protocol = IPPROTO_UDP;    
    ip_header->saddr = client_ip.s_addr;    
    ip_header->daddr = server_ip.s_addr;    
    ip_header->check = 0;
    
    unsigned short *ip_ptr = (unsigned short *)ip_header;
    unsigned int ip_sum = 0;
    for (int i = 0; i < sizeof(struct iphdr) / 2; i++) {
        ip_sum += ntohs(ip_ptr[i]);
    }
    while (ip_sum >> 16) {
        ip_sum = (ip_sum & 0xFFFF) + (ip_sum >> 16);
    }
    ip_header->check = htons(~(unsigned short)ip_sum);
}

void fill_udp_header(char packet[PACKET_SIZE]) {
    struct udphdr *udp = (struct udphdr *)(packet + sizeof(struct ethhdr) + sizeof(struct iphdr));
    
    udp->source = htons(CLIENT_PORT);
    udp->dest = htons(SERVER_PORT);
    udp->len = htons(sizeof(struct udphdr) + strlen(MESSAGE));
    udp->check = 0; 
}

void fill_eth_header(char packet[PACKET_SIZE]) {    
    struct ethhdr *eth = (struct ethhdr *)packet;
    memcpy(eth->h_source, (char[ETH_ALEN])CLIENT_MAC, ETH_ALEN);
    memcpy(eth->h_dest, (char[ETH_ALEN])SERVER_MAC, ETH_ALEN);
    eth->h_proto = htons(ETH_P_IP);
}

int main() {    
    int raw_socket;
    struct sockaddr_ll device;
    char packet[PACKET_SIZE];
    unsigned char src_mac[ETH_ALEN] = CLIENT_MAC;
    
    if ((raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        printf("create raw socket error\n");
        return 1;
    }
    
    int ifindex = if_nametoindex(INTERFACE);
    if (ifindex == 0) {
        printf("interface not found\n");
        close(raw_socket);
        return 1;
    }
    
    memset(&device, 0, sizeof(device));
    device.sll_family = AF_PACKET;
    device.sll_ifindex = ifindex;
    device.sll_halen = ETH_ALEN;
    memcpy(device.sll_addr, src_mac, ETH_ALEN);

    struct in_addr client_ip;  
    inet_pton(AF_INET, CLIENT_ADDRESS, &client_ip);
    
    struct in_addr server_ip;
    inet_pton(AF_INET, SERVER_ADDRESS, &server_ip);

    fill_eth_header(packet);
    fill_ip_header(packet, client_ip, server_ip);    
    fill_udp_header(packet);
    
    char *data = packet + sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(data, MESSAGE, strlen(MESSAGE));
    
    int packet_len = sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr) + strlen(MESSAGE);
    
    printf("Sending packet to server\n");

    if (sendto(raw_socket, packet, packet_len, 0, (struct sockaddr *)&device, sizeof(device)) < 0) {
        printf("sendto error\n");
        close(raw_socket);
        return 1;
    }
    

    printf("Packet has been sent. Waiting for response...\n");

    while (1) {
        char recv_buffer[PACKET_SIZE];
        struct sockaddr_ll from_addr;
        socklen_t from_len = sizeof(from_addr);
        
        int recv_len = recvfrom(raw_socket, recv_buffer, PACKET_SIZE, 0, (struct sockaddr *)&from_addr, &from_len);
        
        if (recv_len < 0) {
            printf("recvfrom error\n");
            continue;
        }
        
        if (from_addr.sll_ifindex != ifindex) {
            continue;
        }
        
        struct ethhdr *recv_eth = (struct ethhdr *)recv_buffer;
        if (ntohs(recv_eth->h_proto) != ETH_P_IP) {
            continue;
        }
        
        struct iphdr *recv_ip_header = (struct iphdr *)(recv_buffer + sizeof(struct ethhdr));
        if (recv_ip_header->protocol != IPPROTO_UDP) {
            continue;
        }
        
        if (recv_ip_header->daddr != client_ip.s_addr) {
            continue;
        }
        
        struct udphdr *recv_udp_header = (struct udphdr *)(recv_buffer + sizeof(struct ethhdr) + (recv_ip_header->ihl * 4));

        if (ntohs(recv_udp_header->dest) != CLIENT_PORT) {
            continue;
        }
        
        char *recv_data = recv_buffer + sizeof(struct ethhdr) + (recv_ip_header->ihl * 4) + sizeof(struct udphdr);
        int recv_data_len = recv_len - (recv_ip_header->ihl * 4) - sizeof(struct udphdr);
        
        recv_data[recv_data_len] = '\0';
        printf("Response from server:\n");
        printf("Source IP: %s\n", CLIENT_ADDRESS);
        printf("Source Port: %d\n", ntohs(recv_udp_header->source));
        printf("Destination Port: %d\n", ntohs(recv_udp_header->dest));
        printf("Length: %d\n", ntohs(recv_udp_header->len));
        printf("Message: %s\n", recv_data);
        break;
    }
    
    close(raw_socket);
    return 0;
}