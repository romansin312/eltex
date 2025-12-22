#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MULTICAST_PORT 8889
#define MULTICAST_GROUP "239.0.0.1"
#define TTL 1

int main() {
    int sockfd;
    struct sockaddr_in multicast_addr;
    char message[] = "Hi";
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation error\n");
        return 1;
    }
    
    int ttl = TTL;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,  &ttl, sizeof(TTL)) < 0) {
        printf("setsockopt error\n");
        close(sockfd);
        return 1;
    }
    
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(MULTICAST_PORT);
    multicast_addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    
    printf("Multicast server is running. Sending messages to multicast group %s...\n", MULTICAST_GROUP);
    
    while (1) {
        if (sendto(sockfd, message, strlen(message), 0,
                   (struct sockaddr*)&multicast_addr, 
                   sizeof(multicast_addr)) < 0) {
            printf("send failed\n");
        } else {
            printf("The message %s has been sent to the group\n", message);
        }
        
        sleep(2);
    }
    
    close(sockfd);
    return 0;
}