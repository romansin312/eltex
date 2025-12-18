#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_KEY 1234
#define CLIENT_KEY 5678
#define MSG_SIZE 256

struct message {
    long mtype;
    char mtext[MSG_SIZE];
};

int main() {
    int server_qid, client_qid;
    struct message msg;
    
    server_qid = msgget(SERVER_KEY, IPC_CREAT | 0666);
    if (server_qid == -1) {
        printf("msgget server error\n");
        return 1;
    }
    
    client_qid = msgget(CLIENT_KEY, IPC_CREAT | 0666);
    if (client_qid == -1) {
        printf("msgget client error\n");
        msgctl(server_qid, IPC_RMID, NULL);
        return 1;
    }
    
    msg.mtype = 1;
    strcpy(msg.mtext, "Hi");
    
    if (msgsnd(client_qid, &msg, strlen(msg.mtext) + 1, 0) == -1) {
        printf("msgsnd error\n");
    } else {
        printf("A meesage is sent to client\n");
    }
    
    if (msgrcv(server_qid, &msg, sizeof(msg.mtext), 0, 0) == -1) {
        perror("msgrcv error\n");
    } else {
        printf("Received message from client: %s\n", msg.mtext);
    }
    
    msgctl(server_qid, IPC_RMID, NULL);
    msgctl(client_qid, IPC_RMID, NULL);
    
    return 0;
}