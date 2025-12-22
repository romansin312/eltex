#ifndef SHARED_CHAT_H
#define SHARED_CHAT_H

#include <time.h>
#include <pthread.h>

#define SHM_NAME "/chat_shm"
#define MAX_MESSAGE_SIZE 256
#define MAX_NAME_LENGTH 32
#define CHAT_WIDTH_PERCENT 0.8
#define MAX_CHAT_HISTORY 1000
#define MAX_CLIENTS 10

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
    sem_t client_sem;
    int active;
    int message_count;
    chat_message messages[MAX_CHAT_HISTORY];
} client_info;

typedef struct {
    sem_t server_sem;
    int client_count;
    int server_messages_count;
    chat_message server_messages[MAX_CHAT_HISTORY];
    client_info clients[MAX_CLIENTS];
} shared_memory;

#endif