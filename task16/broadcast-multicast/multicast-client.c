#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MULTICAST_PORT 8889
#define MULTICAST_GROUP "239.0.0.1"

int main() {
    int sockfd;
    struct sockaddr_in local_addr, multicast_addr;
    socklen_t addr_len = sizeof(multicast_addr);
    char buffer[1024];
    struct ip_mreq multicast_request;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation error\n");
        return 1;
    }
    
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(MULTICAST_PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) < 0) {
        printf("bind error\n");
        close(sockfd);
        return 1;
    }
    
    multicast_request.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    multicast_request.imr_interface.s_addr = INADDR_ANY;
    
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &multicast_request, sizeof(multicast_request)) < 0) {
        printf("setsockopt error\n");
        close(sockfd);
        return 1;
    }
    
    printf("Multicast client is running. Joined to the group %s. Waiting for messages...\n",  MULTICAST_GROUP);
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        
        int recv_len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&multicast_addr, &addr_len);
        
        if (recv_len > 0) {
            buffer[recv_len] = '\0';
            printf("Received message: %s from %s:%d\n", 
                   buffer, 
                   inet_ntoa(multicast_addr.sin_addr), 
                   ntohs(multicast_addr.sin_port));
        }
    }
    
    setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
               &multicast_request, sizeof(multicast_request));
    
    close(sockfd);
    return 0;
}