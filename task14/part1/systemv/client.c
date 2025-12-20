#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 1234
#define SEM_KEY 5678
#define BUFFER_SIZE 256

void sem_wait(int semid) {
    struct sembuf op;
    op.sem_flg = 0;
    op.sem_num = 0;
    op.sem_op = -1;
    semop(semid, &op, 1);
}

void sem_signal(int semid) {
    struct sembuf op;
    op.sem_flg = 0;
    op.sem_num = 0;
    op.sem_op = 1;
    semop(semid, &op, 1);
}

int main() {
    int shmid = shmget(SHM_KEY, BUFFER_SIZE, 0666 | IPC_CREAT);
    if (shmid == -1) {
        printf("shmget error\n");
        return 1;
    }
    
    char *shm_ptr = (char*)shmat(shmid, NULL, 0);
    if (shm_ptr == (char*)-1) {
        printf("shmat error\n");
        return 1;
    }
    
    int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        printf("semget\n");
        return 1;
    }
    
    semctl(semid, 0, SETVAL, 0);
    
    printf("Waiting for server message...\n");
    sem_wait(semid);
    
    printf("Received from server: %s\n", shm_ptr);
    
    strcpy(shm_ptr, "Hello!");
    sem_signal(semid);
    
    shmdt(shm_ptr);
    
    return 0;
}