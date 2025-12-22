#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include "common.h"

shared_memory *shm;
sem_t *server_sem;
chat_message history[MAX_CHAT_HISTORY];
int history_count = 0;
int running = 1;

void broadcast_message(chat_message *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shm->clients[i].active) {
            int count = shm->clients[i].message_count;
            shm->clients[i].messages[count] = *msg;
            count++;
            shm->clients[i].message_count = count;

            sem_post(&shm->clients[i].client_sem);
        }
    }
}

void send_user_list_and_chat_history_to_client(int client_index) {
    char user_list_text[MAX_CLIENTS * MAX_NAME_LENGTH + MAX_CLIENTS] = "";
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shm->clients[i].active) {
            strcat(user_list_text, shm->clients[i].name);
            strcat(user_list_text, "\n");
        }
    }

    chat_message user_list_msg;
    strcpy(user_list_msg.sender, shm->clients[client_index].name);
    strcpy(user_list_msg.text, user_list_text);
    user_list_msg.timestamp = time(NULL);
    shm->clients[client_index].message_count++;
    shm->clients[client_index].messages[shm->clients[client_index].message_count] = user_list_msg;

    for (int i = 0; i < history_count; i++) {
        shm->clients[client_index].message_count++;
        shm->clients[client_index].messages[shm->clients[client_index].message_count] = history[i];
    }

    sem_post(&shm->clients[client_index].client_sem);
}

void handle_client_connection(const char *client_name) {
    int exists = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shm->clients[i].active && strcmp(shm->clients[i].name, client_name) == 0) {
            exists = 1;
            break;
        }
    }
    
    if (exists) {
        printf("Client %s already exists\n", client_name);
        return;
    }
    
    if (shm->client_count >= MAX_CLIENTS) {
        printf("Maximum clients number is exceeded\n");
        return;
    }
    
    int first_empty_index = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!shm->clients[i].active) {
            first_empty_index = i;
            break;
        }
    }
    
    if (first_empty_index == -1) {
        printf("No free slots for new client\n");
        return;
    }
    
    strcpy(shm->clients[first_empty_index].name, client_name);
    sem_init(&shm->clients[first_empty_index].client_sem, 1, 0);
    shm->clients[first_empty_index].active = 1;
    shm->client_count++;
    
    chat_message notify_msg;
    notify_msg.type = MSG_TYPE_TEXT;
    strcpy(notify_msg.sender, "SERVER");
    snprintf(notify_msg.text, MAX_MESSAGE_SIZE, "User %s has been joined to the chat", client_name);
    notify_msg.timestamp = time(NULL);
    
    printf("Client %s has been joined\n", client_name);
    
    broadcast_message(&notify_msg);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shm->clients[i].active) {
            send_user_list_and_chat_history_to_client(i);
        }
    }
}

void handle_client_disconnect(const char *client_name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shm->clients[i].active && strcmp(shm->clients[i].name, client_name) == 0) {
            sem_destroy(&shm->clients[i].client_sem);
            shm->clients[i].active = 0;
            shm->client_count--;
            
            chat_message notify_msg;
            notify_msg.type = MSG_TYPE_TEXT;
            strcpy(notify_msg.sender, "SERVER");
            snprintf(notify_msg.text, MAX_MESSAGE_SIZE, "User %s has been disconnected", client_name);
            notify_msg.timestamp = time(NULL);
            
            broadcast_message(&notify_msg);
            
            printf("Client %s has been disconnected\n", client_name);
            break;
        }
    }
}

void handle_text_message(chat_message *msg) {
    msg->timestamp = time(NULL);
    
    int sender_exists = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shm->clients[i].active && strcmp(shm->clients[i].name, msg->sender) == 0) {
            sender_exists = 1;
            break;
        }
    }
    
    if (!sender_exists) {
        printf("Unknown sender: %s\n", msg->sender);
        return;
    }
    
    printf("Recevied text message from %s: %s\n", msg->sender, msg->text);
    history[history_count] = *msg;
    history_count++;
    broadcast_message(msg);
}

void signal_handler(int sig) {
    running = 0;
}

int main() {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("shm_open error\n");
        return 1;
    }
    
    if (ftruncate(shm_fd, sizeof(shared_memory)) == -1) {
        printf("ftruncate error\n");
        return 1;
    }
    
    shm = mmap(NULL, sizeof(shared_memory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm == MAP_FAILED) {
        printf("mmap failed\n");
        return 1;
    }
    
    sem_init(&shm->server_sem, 1, 0);
    shm->client_count = 0;
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        shm->clients[i].active = 0;
        shm->clients[i].message_count = 0;
    }
    
    printf("Server is running\n");
    
    while (running) {
        sem_wait(&shm->server_sem);
        
        int new_message_index = -1;
        for (int i = 0; i < shm->server_messages_count; i++) {
            if (shm->server_messages[i].type == MSG_TYPE_CONNECT ||
                shm->server_messages[i].type == MSG_TYPE_DISCONNECT ||
                shm->server_messages[i].type == MSG_TYPE_TEXT) {
                new_message_index = i;
                break;
            }
        }
        
        if (new_message_index == -1) {
            continue;
        }
        
        chat_message *msg = &shm->server_messages[new_message_index];
        
        printf("Receved message: type %d, sender %s\n", msg->type, msg->sender);

        switch (msg->type) {
            case MSG_TYPE_CONNECT:
                handle_client_connection(msg->sender);
                break;
                
            case MSG_TYPE_DISCONNECT:
                handle_client_disconnect(msg->sender);
                break;
                
            case MSG_TYPE_TEXT:
                handle_text_message(msg);
                break;
                
            default:
                printf("Unknown message type: %d\n", msg->type);
                break;
        }
        
        for (int i = new_message_index; i < shm->server_messages_count - 1; i++) {
            shm->server_messages[i] = shm->server_messages[i + 1];
        }
        shm->server_messages_count--;
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (shm->clients[i].active) {
            sem_destroy(&shm->clients[i].client_sem);
        }
    }
    
    sem_destroy(&shm->server_sem);
    munmap(shm, sizeof(shared_memory));
    shm_unlink(SHM_NAME);
    
    printf("Server stopped\n");
    
    return 0;
}