#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>

#define MAX_DRIVERS 100
#define SHM_NAME "/taxi_shm"
#define SOCKET_PATH "/tmp/taxi_socket_%d"

#define STATUS_REQUEST "STATUS"

typedef struct {
    pid_t pid;
    int busy;
    int task_timer;
    time_t task_end;
    int timer_fd;
    int socket_fd;
} DriverStatus;

typedef struct {
    int num_drivers;
    DriverStatus drivers[MAX_DRIVERS];
} SharedData;

SharedData *shared_data = NULL;
int shm_fd = -1;

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
    
    static int initialized = 0;
    if (!initialized) {
        shared_data->num_drivers = 0;
        for (int i = 0; i < MAX_DRIVERS; i++) {
            shared_data->drivers[i].pid = 0;
            shared_data->drivers[i].timer_fd = -1;
            shared_data->drivers[i].socket_fd = -1;
        }
        initialized = 1;
    }
}

void setup_timer(int timer_fd, int seconds) {
    struct itimerspec timer_spec;
    timer_spec.it_value.tv_sec = seconds;
    timer_spec.it_value.tv_nsec = 0;
    timer_spec.it_interval.tv_sec = 0;
    timer_spec.it_interval.tv_nsec = 0;
    
    if (timerfd_settime(timer_fd, 0, &timer_spec, NULL) < 0) {
        printf("timerfd_settime error\n");
    }
}

void handle_socket_command(int client_fd, int timer_fd, int driver_index, int *task_active, int *task_timer) {
    char buffer[256];
    int bytes = read(client_fd, buffer, sizeof(buffer)-1);
    if (bytes <= 0) {
        return;
    }
    
    buffer[bytes] = '\0';
    
    if (strncmp(buffer, "TASK ", 5) == 0) {
        int requested_timer = atoi(buffer + 5);
        
        if (*task_active) {
            char response[256];
            snprintf(response, sizeof(response), "BUSY %d", *task_timer);
            write(client_fd, response, strlen(response));
        } else {
            *task_active = 1;
            *task_timer = requested_timer;
            
            setup_timer(timer_fd, requested_timer);
            
            shared_data->drivers[driver_index].busy = 1;
            shared_data->drivers[driver_index].task_timer = requested_timer;
            shared_data->drivers[driver_index].task_end = time(NULL) + requested_timer;
            
            write(client_fd, "OK", 2);
        }
    } else if (strcmp(buffer, STATUS_REQUEST) == 0) {
        char response[256];
        if (*task_active) {
            snprintf(response, sizeof(response), "BUSY %d", *task_timer);
        } else {
            strcpy(response, "AVAILABLE");
        }
        write(client_fd, response, strlen(response));
    }
}

void handle_timer_event(int timer_fd, int my_index, int *task_active, int *task_timer) {
    uint64_t expirations;
    read(timer_fd, &expirations, sizeof(expirations));
    
    *task_active = 0;
    *task_timer = 0;
    
    shared_data->drivers[my_index].busy = 0;
    shared_data->drivers[my_index].task_timer = 0;
    shared_data->drivers[my_index].task_end = 0;
    
    printf("Driver %d: Task completed\n", getpid());
}

int create_driver_socket() {
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("socket error\n");
        exit(1);
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), SOCKET_PATH, getpid());
    
    unlink(addr.sun_path);
    
    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("bind error\n");
        exit(1);
    }
    
    listen(sock_fd, 5);
    return sock_fd;
}

int create_driver_timer() {
    int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    if (timer_fd < 0) {
        printf("timerfd_create error\n");
        exit(1);
    }
    return timer_fd;
}

int setup_epoll(int sock_fd, int timer_fd) {
    int epoll_fd = epoll_create(0);
    if (epoll_fd < 0) {
        printf("epoll_create error\n");
        exit(1);
    }
    
    struct epoll_event event;
    
    event.events = EPOLLIN;
    event.data.fd = sock_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event) < 0) {
        printf("epoll_ctl sock_fd error\n");
        exit(1);
    }
    
    event.events = EPOLLIN;
    event.data.fd = timer_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, timer_fd, &event) < 0) {
        printf("epoll_ctl timer_fd error\n");
        exit(1);
    }
    
    return epoll_fd;
}

int register_driver_in_shm(int sock_fd, int timer_fd) {
    int my_index = -1;
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (shared_data->drivers[i].pid == 0) {
            my_index = i;
            shared_data->drivers[i].pid = getpid();
            shared_data->drivers[i].busy = 0;
            shared_data->drivers[i].task_timer = 0;
            shared_data->drivers[i].task_end = 0;
            shared_data->drivers[i].timer_fd = timer_fd;
            shared_data->drivers[i].socket_fd = sock_fd;
            shared_data->num_drivers++;
            break;
        }
    }
    return my_index;
}

void driver_process() {
    printf("Driver started with PID %d\n", getpid());
    
    int sock_fd = create_driver_socket();
    int timer_fd = create_driver_timer();
    
    int my_index = register_driver_in_shm(sock_fd, timer_fd);
    if (my_index == -1) {
        printf("Driver %d: No space in shared memory\n", getpid());
        exit(1);
    }
    
    printf("Driver %d registered\n", getpid());
    
    int epoll_fd = setup_epoll(sock_fd, timer_fd);
    struct epoll_event events[10];
    
    int task_active = 0;
    int task_timer = 0;
    
    while (1) {
        int nfds = epoll_wait(epoll_fd, events, 10, -1);
        if (nfds < 0) {
            printf("epoll_wait error\n");
            break;
        }
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == sock_fd) {
                int client_fd = accept(sock_fd, NULL, NULL);
                if (client_fd < 0) {
                    printf("accept error\n");
                    continue;
                }
                
                handle_socket_command(client_fd, timer_fd, my_index, &task_active, &task_timer);
                close(client_fd);
            }
            else if (events[i].data.fd == timer_fd) {
                handle_timer_event(timer_fd, my_index, &task_active, &task_timer);
            }
        }
    }
    
    close(sock_fd);
    close(timer_fd);
    close(epoll_fd);
    
    char socket_path[256];
    snprintf(socket_path, sizeof(socket_path), SOCKET_PATH, getpid());
    unlink(socket_path);
}

void create_driver() {
    if (shared_data->num_drivers >= MAX_DRIVERS) {
        printf("Maximum number of drivers reached\n");
        return;
    }

    pid_t pid = fork();
    
    if (pid == 0) {
        driver_process();
        exit(0);
    } else if (pid > 0) {
        printf("New driver created with PID %d\n", pid);
        usleep(50000);
    } else {
        printf("fork error\n");
    }
}

int find_driver_by_pid(pid_t pid) {
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (shared_data->drivers[i].pid == pid) {
            return i;
        }
    }
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
    
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("socket error\n");
        return;
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), SOCKET_PATH, pid);
    
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("connect error\n");
        close(sock_fd);
        return;
    }
    
    char command[256];
    snprintf(command, sizeof(command), "TASK %d", task_timer);
    write(sock_fd, command, strlen(command));
    
    char response[256];
    int bytes = read(sock_fd, response, sizeof(response)-1);
    if (bytes > 0) {
        response[bytes] = '\0';
        
        if (strcmp(response, "OK") == 0) {
            printf("Task sent to driver %d for %d seconds\n", pid, task_timer);
            
            shared_data->drivers[driver_index].busy = 1;
            shared_data->drivers[driver_index].task_timer = task_timer;
            shared_data->drivers[driver_index].task_end = time(NULL) + task_timer;
        } else if (strncmp(response, "BUSY ", 5) == 0) {
            int remaining = atoi(response + 5);
            printf("Driver %d is busy for %d seconds\n", pid, remaining);
        }
    }
    
    close(sock_fd);
}

void get_status(pid_t pid) {
    int driver_index = find_driver_by_pid(pid);
    if (driver_index == -1) {
        printf("Error: Driver with PID %d not found\n", pid);
        return;
    }
    
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        printf("socket error\n");
        return;
    }
    
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), SOCKET_PATH, pid);
    
    if (connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        if (shared_data->drivers[driver_index].busy) {
            time_t now = time(NULL);
            if (shared_data->drivers[driver_index].task_end > now) {
                int remaining = shared_data->drivers[driver_index].task_end - now;
                printf("Busy for %d seconds\n", remaining);
            } else {
                printf("Available\n");
            }
        } else {
            printf("Available\n");
        }
        close(sock_fd);
        return;
    }
    
    write(sock_fd, STATUS_REQUEST, 6);
    
    char response[256];
    int bytes = read(sock_fd, response, sizeof(response)-1);
    if (bytes > 0) {
        response[bytes] = '\0';
        printf("%s\n", response);
    }
    
    close(sock_fd);
}

void get_drivers() {
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
}

void cleanup() {
    for (int i = 0; i < MAX_DRIVERS; i++) {
        if (shared_data->drivers[i].pid != 0) {
            kill(shared_data->drivers[i].pid, SIGTERM);
            
            char socket_path[256];
            snprintf(socket_path, sizeof(socket_path), SOCKET_PATH, shared_data->drivers[i].pid);
            unlink(socket_path);
        }
    }
    
    while (wait(NULL) > 0);
    
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
    
    printf("Taxi Management System\n");
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