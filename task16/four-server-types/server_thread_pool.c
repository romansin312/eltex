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
#define THREAD_POOL_SIZE 4
#define MAX_QUEUE_SIZE 100

typedef struct {
    int client_fd;
    struct sockaddr_in client_addr;
} task_t;

typedef struct {
    task_t tasks[MAX_QUEUE_SIZE];
    int front, rear, count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} task_queue_t;

task_queue_t queue;
pthread_t thread_pool[THREAD_POOL_SIZE];

void init_queue(task_queue_t* q) {
    q->front = q->rear = q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void enqueue(task_queue_t* q, task_t task) {
    pthread_mutex_lock(&q->mutex);
    
    if (q->count < MAX_QUEUE_SIZE) {
        q->tasks[q->rear] = task;
        q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
        q->count++;
        pthread_cond_signal(&q->cond);
    }
    
    pthread_mutex_unlock(&q->mutex);
}

task_t dequeue(task_queue_t* q) {
    pthread_mutex_lock(&q->mutex);
    
    while (q->count == 0) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    
    task_t task = q->tasks[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->count--;
    
    pthread_mutex_unlock(&q->mutex);
    return task;
}

void* worker_thread(void* arg) {
    char buffer[BUFFER_SIZE];
    
    while (1) {
        task_t task = dequeue(&queue);
        
        printf("Thread %lu handling client %s:%d\n", 
               pthread_self(),
               inet_ntoa(task.client_addr.sin_addr),
               ntohs(task.client_addr.sin_port));
        
        ssize_t bytes_read;
        while ((bytes_read = read(task.client_fd, buffer, BUFFER_SIZE - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("Thread %lu received: %s", pthread_self(), buffer);
            write(task.client_fd, buffer, bytes_read);
        }
        
        close(task.client_fd);
        printf("Thread %lu: Client disconnected\n", pthread_self());
    }
    
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    init_queue(&queue);
    
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, worker_thread, NULL);
    }
    
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
    
    printf("Thread pool server listening on port %d...\n", PORT);
    
    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            printf("accept error\n");
            continue;
        }
        
        printf("New connection from %s:%d\n", 
               inet_ntoa(client_addr.sin_addr), 
               ntohs(client_addr.sin_port));
        
        task_t task;
        task.client_fd = client_fd;
        task.client_addr = client_addr;
        enqueue(&queue, task);
    }
    
    close(server_fd);
    return 0;
}