#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SHM_NAME "/task14shm"
#define SEM_SERVER_READY "/sem_server_ready"
#define SEM_CLIENT_READY "/sem_client_ready"
#define BUFFER_SIZE 256

int main() {
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("shared memory error\n");
        return 1;
    }
    
    ftruncate(shm_fd, BUFFER_SIZE);
    
    char *shm_ptr = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        printf("nmap error\n");
        return 1;
    }
    
    sem_t *sem_server_ready = sem_open(SEM_SERVER_READY, O_CREAT, 0666, 0);
    sem_t *sem_client_ready = sem_open(SEM_CLIENT_READY, O_CREAT, 0666, 0);
    
    if (sem_server_ready == SEM_FAILED || sem_client_ready == SEM_FAILED) {
        printf("semaphore open error\n");
        return 1;
    }
    
    strcpy(shm_ptr, "Hi!");
    printf("Message has been sent to client\n");
    sem_post(sem_server_ready);
    
    printf("Waiting for client response...\n");
    sem_wait(sem_client_ready);
    
    printf("Received from client: %s\n", shm_ptr);
    
    munmap(shm_ptr, BUFFER_SIZE);
    close(shm_fd);
    
    shm_unlink(SHM_NAME);
    
    sem_close(sem_server_ready);
    sem_close(sem_client_ready);
    sem_unlink(SEM_SERVER_READY);
    sem_unlink(SEM_CLIENT_READY);
    
    return 0;
}