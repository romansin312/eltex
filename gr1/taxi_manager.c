#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>

#define MAX_DRIVERS 100
#define SHM_NAME "/taxi_shm"
#define SEM_NAME "/taxi_sem"
#define TASK_SEM_PREFIX "/taxi_task_"

typedef struct {
    pid_t pid;
    int busy;
    int task_timer;
    time_t task_end;
    char task_sem_name[50];
} DriverStatus;

typedef struct {
    int num_drivers;
    DriverStatus drivers[MAX_DRIVERS];
} SharedData;

SharedData *shared_data = NULL;
int shm_fd = -1;
sem_t *shm_sem = NULL;

void init_shared_memory() {
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("shm_open error\n");
        exit(1);
    }
    
    if (ftruncate(shm_fd, sizeof(SharedData)) == -1) {
        printf("ftruncate error\n");
        exit(1);
    }
    
    shared_data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_data == MAP_FAILED) {
        printf("mmap error\n");
        exit(1);
    }
    
    shm_sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (shm_sem == SEM_FAILED) {
        printf("sem_open error\n");
        exit(1);
    }
    
    sem_wait(shm_sem);
    if (shared_data->num_drivers == 0) {
        for (int i = 0; i < MAX_DRIVERS; i++) {
            shared_data->drivers[i].pid = 0;
            shared_data->drivers[i].busy = 0;
            shared_data->drivers[i].task_timer = 0;
            shared_data->drivers[i].task_end = 0;
            memset(shared_data->drivers[i].task_sem_name, 0, 50);
        }
    }
    sem_post(shm_sem);
}

void driver_process() {
    printf("Driver started with PID %d\n", getpid());
    
    char task_sem_name[50];
    snprintf(task_sem_name, sizeof(task_sem_name), "%s%d", TASK_SEM_PREFIX, getpid());
    
    sem_t *task_sem = sem_open(task_sem_name, O_CREAT, 0666, 0);
    if (task_sem == SEM_FAILED) {
        printf("sem_open error\n");
        exit(1);
    }
    
    sem_wait(shm_sem);
    int new_index = -1;
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (shared_data->drivers[i].pid == 0) {
            new_index = i;
            shared_data->drivers[i].pid = getpid();
            shared_data->drivers[i].busy = 0;
            shared_data->drivers[i].task_timer = 0;
            shared_data->drivers[i].task_end = 0;
            strcpy(shared_data->drivers[i].task_sem_name, task_sem_name);
            shared_data->num_drivers++;
            break;
        }
    }
    sem_post(shm_sem);
    
    if (new_index == -1) {
        printf("Driver %d: No empty space in shared memory.\n", getpid());
        exit(1);
    }
    
    printf("Driver %d registered at index %d\n", getpid(), new_index);
    
    while (1) {
        sem_wait(task_sem);
        
        sem_wait(shm_sem);
        int task_timer = shared_data->drivers[new_index].task_timer;
        sem_post(shm_sem);
        
        if (task_timer > 0) {
            time_t task_end_time = time(NULL) + task_timer;
            
            sem_wait(shm_sem);
            shared_data->drivers[new_index].busy = 1;
            shared_data->drivers[new_index].task_end = task_end_time;
            sem_post(shm_sem);
            
            sleep(task_timer);
            
            sem_wait(shm_sem);
            shared_data->drivers[new_index].busy = 0;
            shared_data->drivers[new_index].task_timer = 0;
            shared_data->drivers[new_index].task_end = 0;
            sem_post(shm_sem);
        }
    }
}

void create_driver() {
    sem_wait(shm_sem);
    if (shared_data->num_drivers >= MAX_DRIVERS) {
        printf("Maximum number of drivers reached\n");
        sem_post(shm_sem);
        return;
    }
    sem_post(shm_sem);
    
    pid_t pid = fork();
    
    if (pid == 0) {
        driver_process();
        exit(0);
    } else if (pid > 0) {
        printf("New driver created with PID %d\n", pid);
        usleep(100000);
    } else {
        printf("fork error\n");
    }
}

int find_driver_by_pid(pid_t pid) {
    sem_wait(shm_sem);
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (shared_data->drivers[i].pid == pid) {
            sem_post(shm_sem);
            return i;
        }
    }
    sem_post(shm_sem);
    return -1;
}

void send_task(pid_t pid, int task_timer) {
    if (task_timer <= 0) {
        printf("Error: task_timer must be positive\n");
        return;
    }
    
    int driver_index = find_driver_by_pid(pid);
    if (driver_index == -1) {
        printf("Error: Driver with PID %d not found\n", pid);
        return;
    }
    
    sem_wait(shm_sem);
    int is_busy = shared_data->drivers[driver_index].busy;
    int current_timer = shared_data->drivers[driver_index].task_timer;
    char task_sem_name[50];
    strcpy(task_sem_name, shared_data->drivers[driver_index].task_sem_name);
    sem_post(shm_sem);
    
    if (is_busy) {
        printf("Driver %d is busy for %d seconds\n", pid, current_timer);
        return;
    }
    
    if (strlen(task_sem_name) == 0) {
        printf("Error: Driver %d not ready\n", pid);
        return;
    }
    
    sem_wait(shm_sem);
    shared_data->drivers[driver_index].task_timer = task_timer;
    sem_post(shm_sem);
    
    sem_t *task_sem = sem_open(task_sem_name, 0);
    if (task_sem == SEM_FAILED) {
        printf("sem_open error\n");
        return;
    }
    
    if (sem_post(task_sem) == -1) {
        printf("sem_post error\n");
        sem_close(task_sem);
        return;
    }
    
    sem_close(task_sem);
    printf("Task sent to driver %d for %d seconds\n", pid, task_timer);
}

void get_status(pid_t pid) {
    int driver_index = find_driver_by_pid(pid);
    
    if (driver_index == -1) {
        printf("Error: Driver with PID %d not found\n", pid);
        return;
    }
    
    sem_wait(shm_sem);
    int busy = shared_data->drivers[driver_index].busy;
    int timer = shared_data->drivers[driver_index].task_timer;
    time_t task_end = shared_data->drivers[driver_index].task_end;
    sem_post(shm_sem);
    
    if (busy) {
        time_t now = time(NULL);
        if (task_end > now) {
            int remaining = task_end - now;
            printf("Busy for %d seconds\n", remaining);
        } else {
            printf("Available (timer expired)\n");
        }
    } else {
        printf("Available\n");
    }
}

void get_drivers() {
    sem_wait(shm_sem);
    printf("Total drivers: %d\n", shared_data->num_drivers);
    printf("PID\t\tStatus\t\tTime left\n");
    printf("--------------------------------\n");
    
    time_t now = time(NULL);
    
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (shared_data->drivers[i].pid != 0) {
            printf("%d\t\t", shared_data->drivers[i].pid);
            
            if (shared_data->drivers[i].busy) {
                if (shared_data->drivers[i].task_end > now) {
                    int remaining = shared_data->drivers[i].task_end - now;
                    printf("Busy\t\t%d sec\n", remaining);
                } else {
                    shared_data->drivers[i].busy = 0;
                    shared_data->drivers[i].task_timer = 0;
                    printf("Available\t-\n");
                }
            } else {
                printf("Available\t-\n");
            }
        }
    }
    sem_post(shm_sem);
}

void cleanup() {
    sem_wait(shm_sem);
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (shared_data->drivers[i].pid != 0) {
            if (strlen(shared_data->drivers[i].task_sem_name) > 0) {
                sem_unlink(shared_data->drivers[i].task_sem_name);
            }
            kill(shared_data->drivers[i].pid, SIGTERM);
        }
    }
    sem_post(shm_sem);
    
    while (wait(NULL) > 0);
    
    if (shm_sem != NULL) {
        sem_close(shm_sem);
        sem_unlink(SEM_NAME);
    }
    
    if (shared_data != NULL) {
        munmap(shared_data, sizeof(SharedData));
    }
    if (shm_fd != -1) {
        close(shm_fd);
        shm_unlink(SHM_NAME);
    }
}

int main() {
    atexit(cleanup);
    init_shared_memory();
    
    printf("Taxi Management System CLI\n");
    printf("Commands:\n");
    printf("- create_driver\n");
    printf("- send_task <pid> <task_timer>\n");
    printf("- get_status <pid>\n");
    printf("- get_drivers\n");
    printf("- exit\n\n");
    
    char command[256];
    
    while (1) {
        printf("taxi> ");
        fflush(stdout);
        
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        
        command[strcspn(command, "\n")] = 0;
        
        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "create_driver") == 0) {
            create_driver();
        } else if (strcmp(command, "get_drivers") == 0) {
            get_drivers();
        } else {
            pid_t pid;
            int timer;
            if (sscanf(command, "send_task %d %d", &pid, &timer) == 2) {
                send_task(pid, timer);
            } else if (sscanf(command, "get_status %d", &pid) == 1) {
                get_status(pid);
            } else if (strlen(command) > 0) {
                printf("Unknown command: %s\n", command);
                printf("Available commands: create_driver, send_task <pid> <timer>, get_status <pid>, get_drivers, exit\n");
            }
        }
    }
    
    return 0;
}