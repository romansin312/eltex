#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#define SERVER_QUEUE_NAME   "/chat_server"
#define MAX_CLIENTS         10
#define MAX_MESSAGE_SIZE    256
#define MAX_NAME_LENGTH     32

typedef enum {
    MSG_TYPE_CONNECT = 1,
    MSG_TYPE_DISCONNECT,
    MSG_TYPE_TEXT,
    MSG_TYPE_USER_LIST
} message_type;

typedef struct {
    message_type type;
    char sender[MAX_NAME_LENGTH];
    char text[MAX_MESSAGE_SIZE];
    time_t timestamp;
} chat_message;

typedef struct {
    char name[MAX_NAME_LENGTH];
    mqd_t queue;
    int active;
} client_info;

client_info clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(chat_message *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {            
            mq_send(clients[i].queue, (char*)msg, sizeof(chat_message), 0);
            printf("Broadcast: the message has been sent to %s\n", clients[i].name);
        }
    }    
}

void send_user_list(const char *client_name) {    
    chat_message user_list_msg;
    user_list_msg.type = MSG_TYPE_USER_LIST;
    strcpy(user_list_msg.sender, "SERVER");
    
    char user_list_text[MAX_CLIENTS * MAX_NAME_LENGTH + MAX_CLIENTS] = "";
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            strcat(user_list_text, clients[i].name);
            strcat(user_list_text, "\n");
        }
    }
    strcpy(user_list_msg.text, user_list_text);
    user_list_msg.timestamp = time(NULL);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, client_name) == 0) {
            mq_send(clients[i].queue, (char*)&user_list_msg, sizeof(chat_message), 0);
            printf("Users list is sent to the client %s\n", client_name);
            break;
        }
    }
}

void handle_client_connection(chat_message *msg) {
    int exists = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, msg->sender) == 0) {
            exists = 1;
            break;
        }
    }
    
    if (exists) {
        printf("Client %s already exists\n", msg->sender);
        return;
    }
    
    if (client_count >= MAX_CLIENTS) {
        printf("Maximum clients number is exceeded\n");
        return;
    }
    
    char client_queue_name[MAX_NAME_LENGTH * 2];
    snprintf(client_queue_name, sizeof(client_queue_name), "/chat_client_%s", msg->sender);
        
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(chat_message);
    attr.mq_curmsgs = 0;
    
    mqd_t client_queue = mq_open(client_queue_name, O_WRONLY, 0666, &attr);
    if (client_queue == (mqd_t)-1) {
        printf("client mq open error\n");
        return;
    }

    int first_empty_index ;
    for (first_empty_index = 0; first_empty_index < MAX_CLIENTS; first_empty_index++) {
        if (!clients[first_empty_index].active) {
            break;
        }
    }
    
    strcpy(clients[first_empty_index].name, msg->sender);
    clients[first_empty_index].queue = client_queue;
    clients[first_empty_index].active = 1;
    client_count++;
    
    chat_message notify_msg;
    notify_msg.type = MSG_TYPE_TEXT;
    strcpy(notify_msg.sender, "SERVER");
    snprintf(notify_msg.text, MAX_MESSAGE_SIZE, "User %s has been joined to the chat", msg->sender);
    notify_msg.timestamp = time(NULL);

    printf("Client %s has been joined\n", msg->sender);

    broadcast_message(&notify_msg);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].active)
        {
            send_user_list(clients[i].name);
        }
    }
}

void handle_client_disconnect(chat_message *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, msg->sender) == 0) {
            mq_close(clients[i].queue);
            char client_queue_name[MAX_NAME_LENGTH * 2];
            snprintf(client_queue_name, sizeof(client_queue_name), "/chat_client_%s", msg->sender);
            
            clients[i].active = 0;
            
            chat_message notify_msg;
            notify_msg.type = MSG_TYPE_TEXT;
            strcpy(notify_msg.sender, "SERVER");
            snprintf(notify_msg.text, MAX_MESSAGE_SIZE, "User %s has been disconnected", msg->sender);
            notify_msg.timestamp = time(NULL);
            client_count--;

            broadcast_message(&notify_msg);
            
            printf("Client %s has been disconnected\n", msg->sender);
            break;
        }
    }
}

void handle_text_message(chat_message *msg) {
    msg->timestamp = time(NULL);
    
    int sender_exists = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, msg->sender) == 0) {
            sender_exists = 1;
            break;
        }
    }
    
    if (!sender_exists) {
        printf("Unknown sender: %s\n", msg->sender);
        return;
    }
    
    printf("Recevied text message from %s: %s\n", msg->sender, msg->text);

    broadcast_message(msg);
}

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(chat_message);
    attr.mq_curmsgs = 0;
    
    mqd_t server_queue = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, 0666, &attr);
    if (server_queue == (mqd_t)-1) {
        perror("server mq open error");
        return 1;
    }
    
    printf("Server is running\n");
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
    }
    
    while (1) {
        chat_message msg;
        ssize_t bytes_read = mq_receive(server_queue, (char*)&msg, sizeof(chat_message), NULL);
        
        if (bytes_read < 0) {
            if (errno != EAGAIN) {
                printf("message read error\n");
            }
            continue;
        }
        
        printf("Receved message: type %d, sender %s\n", msg.type, msg.sender);
        
        switch (msg.type) {
            case MSG_TYPE_CONNECT:
                handle_client_connection(&msg);
                break;
                
            case MSG_TYPE_DISCONNECT:
                handle_client_disconnect(&msg);
                break;
                
            case MSG_TYPE_TEXT:
                handle_text_message(&msg);
                break;
                
            default:
                printf("Unknown message type: %d\n", msg.type);
                break;
        }
    }
    
    mq_close(server_queue);
    mq_unlink(SERVER_QUEUE_NAME);
    
    return 0;
}