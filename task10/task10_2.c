#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void print_process_info(char *process_name) {
    printf("%s pid: %d, %s ppid: %d\n", process_name, getpid(), process_name, getppid());
}

int main()
{
    print_process_info("Main");

    pid_t process1_pid = fork();
    if (process1_pid == 0)
    {
        print_process_info("Process1");
        pid_t process3_pid = fork();
        if (process3_pid == 0)
        {
            print_process_info("Process3");
            exit(EXIT_SUCCESS);
        }
        else if (process3_pid > 0)
        {
            wait(NULL);
        }

        pid_t process4_pid = fork();
        if (process4_pid == 0)
        {
            print_process_info("Process4");
        }
        else if (process4_pid > 0)
        {
            wait(NULL);
        }
    }
    else if (process1_pid > 0)
    {
        pid_t process2_pid = fork();
        if (process2_pid == 0)
        {
            print_process_info("Process2");
            pid_t process5_pid = fork();
            if (process5_pid == 0)
            {
                print_process_info("Process5");
                exit(EXIT_SUCCESS);
            }
            else if (process5_pid > 0)
            {
                wait(NULL);
            }
        } else if (process2_pid > 0) {
            wait(NULL);
            wait(NULL);
        }
    }
    
    return 0;
}