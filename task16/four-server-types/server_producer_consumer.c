#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024
#define MAX_QUEUE_SIZE 100

typedef struct {
    int client_fd;
    char request[BUFFER_SIZE];
    size_t request_len;
} request_t;

typedef struct {
    int client_fd;
    char response[BUFFER_SIZE];
    size_t response_len;
} response_t;

typedef struct {
    request_t requests[MAX_QUEUE_SIZE];
    int request_front, request_rear, request_count;
    response_t responses[MAX_QUEUE_SIZE];
    int response_front, response_rear, response_count;
    pthread_mutex_t mutex;
    pthread_cond_t request_cond;
    pthread_cond_t response_cond;
} queue_t;

queue_t shared_queue;

void init_queue(queue_t* q) {
    q->request_front = q->request_rear = q->request_count = 0;
    q->response_front = q->response_rear = q->response_count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->request_cond, NULL);
    pthread_cond_init(&q->response_cond, NULL);
}

void enqueue_request(queue_t* q, request_t req) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->request_count >= MAX_QUEUE_SIZE) {
        pthread_cond_wait(&q->response_cond, &q->mutex);
    }
    
    q->requests[q->request_rear] = req;
    q->request_rear = (q->request_rear + 1) % MAX_QUEUE_SIZE;
    q->request_count++;
    pthread_cond_signal(&q->request_cond);
    
    pthread_mutex_unlock(&q->mutex);
}

request_t dequeue_request(queue_t* q) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->request_count == 0) {
        pthread_cond_wait(&q->request_cond, &q->mutex);
    }
    
    request_t req = q->requests[q->request_front];
    q->request_front = (q->request_front + 1) % MAX_QUEUE_SIZE;
    q->request_count--;
    
    pthread_mutex_unlock(&q->mutex);
    return req;
}

void enqueue_response(queue_t* q, response_t resp) {
    pthread_mutex_lock(&q->mutex);
    
    q->responses[q->response_rear] = resp;
    q->response_rear = (q->response_rear + 1) % MAX_QUEUE_SIZE;
    q->response_count++;
    pthread_cond_signal(&q->response_cond);
    
    pthread_mutex_unlock(&q->mutex);
}

response_t dequeue_response(queue_t* q) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->response_count == 0) {
        pthread_cond_wait(&q->response_cond, &q->mutex);
    }
    
    response_t resp = q->responses[q->response_front];
    q->response_front = (q->response_front + 1) % MAX_QUEUE_SIZE;
    q->response_count--;
    
    pthread_mutex_unlock(&q->mutex);
    return resp;
}

void* producer(void* arg) {
    int server_fd = *(int*)arg;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            printf("accept error\n");
            continue;
        }
        
        printf("Producer: New client %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));
        
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        
        while ((bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            
            request_t req;
            req.client_fd = client_fd;
            memcpy(req.request, buffer, bytes_read);
            req.request_len = bytes_read;
            
            printf("Producer: Enqueued request from client %d\n", client_fd);
            enqueue_request(&shared_queue, req);
        }
        
        close(client_fd);
        printf("Producer: Client %d disconnected\n", client_fd);
    }
    
    return NULL;
}

void* consumer(void* arg) {
    while (1) {
        request_t req = dequeue_request(&shared_queue);
        
        printf("Consumer: Processing request from client %d\n", req.client_fd);
        
        response_t resp;
        resp.client_fd = req.client_fd;
        memcpy(resp.response, req.request, req.request_len);
        resp.response_len = req.request_len;
        
        enqueue_response(&shared_queue, resp);
    }
    
    return NULL;
}

void* responder(void* arg) {
    while (1) {
        response_t resp = dequeue_response(&shared_queue);
        
        printf("Responder: Sending response to client %d\n", resp.client_fd);
        
        write(resp.client_fd, resp.response, resp.response_len);
    }
    
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr;
    pthread_t producer_thread, consumer_thread, responder_thread;
    
    init_queue(&shared_queue);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        printf("socket error\n");
        exit(1);
    }
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("bind error\n");
        exit(1);
    }
    
    if (listen(server_fd, 10) < 0) {
        printf("listen error\n");
        exit(1);
    }
    
    printf("Producer-Consumer server listening on port %d...\n", PORT);
    
    pthread_create(&producer_thread, NULL, producer, &server_fd);
    pthread_create(&consumer_thread, NULL, consumer, NULL);
    pthread_create(&responder_thread, NULL, responder, NULL);
    
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);
    pthread_join(responder_thread, NULL);
    
    close(server_fd);
    return 0;
}