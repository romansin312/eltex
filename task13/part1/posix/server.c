#include <mqueue.h>
#include <malloc.h>
#include <string.h>

#define SERVER_MQ_NAME "/server_mq"
#define CLIENT_MQ_NAME "/client_mq"
#define IN_BUFFER_SIZE 100

int main() {    
    struct mq_attr attrs;
    attrs.mq_maxmsg = 50;
    attrs.mq_flags = 0;
    attrs.mq_msgsize = IN_BUFFER_SIZE * sizeof(char);
    attrs.mq_curmsgs = 0;
    mqd_t server_mqd = mq_open(SERVER_MQ_NAME, O_CREAT | O_RDONLY, 0600, &attrs);
    if (server_mqd == (mqd_t)-1) {
        printf("error on server mq open\n");
        return 1;
    }
    
    mqd_t client_mqd = mq_open(CLIENT_MQ_NAME, O_CREAT | O_WRONLY, 0600, &attrs);
    if (client_mqd == (mqd_t)-1) {
        printf("error on client mq open\n");
        mq_close(server_mqd);
        mq_unlink(SERVER_MQ_NAME);
        return 1;
    }

    char *out_buffer = "Hi";
    if (mq_send(client_mqd, out_buffer, strlen(out_buffer) + 1, 0) == -1) {
        printf("error on message sending\n");
    } else {
        printf("message to client has been sent successfully\n");
    }

    printf("Waiting for an incoming message...\n");
    char *in_buffer = malloc(sizeof(char) * IN_BUFFER_SIZE);
    if (mq_receive(server_mqd, in_buffer, IN_BUFFER_SIZE, NULL) == -1) {
        printf("error on message receiving\n");
    } else {
        printf("%s\n", in_buffer);
    }

    mq_close(client_mqd);
    mq_close(server_mqd);
    mq_unlink(SERVER_MQ_NAME);
    mq_unlink(CLIENT_MQ_NAME);
    return 0;
}