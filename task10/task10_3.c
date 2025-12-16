#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CMD_INPUT 256
#define MAX_ARGS 20

char** get_args(char* command)
{
    char **args = malloc(sizeof(char*) * (MAX_ARGS + 1));
    int argsCount = 0;
    char* token = strtok(command, " ");
    while (token != NULL && argsCount < MAX_ARGS)
    {
        args[argsCount] = token;
        token = strtok(NULL, " ");
        argsCount++;
    }
    args[argsCount] = NULL;

    if (argsCount == 0)
    {
        free(args);
        return NULL;
    }

    return args;
}

int main()
{
    char command[MAX_CMD_INPUT];
    char** args;

    while (1)
    {
        printf("> ");

        if (fgets(command, sizeof(command), stdin) == NULL)
        {
            printf("input error");
        }

        command[strcspn(command, "\n")] = '\0';

        if (strcmp("exit", command) == 0)
        {
            break;
        }

        args = get_args(command);
        
        if (args == NULL)
        {
            printf("empty command\n");
            continue;
        }

        pid_t child_pid = fork();
        if (child_pid == 0)
        {
            execvp(args[0], args);

            printf("command execution error\n");
            free(args);
            exit(EXIT_FAILURE);
        }
        else if (child_pid > 0)
        {
            wait(NULL);
            free(args);
        }
        else
        {
            free(args);
            printf("fork error\n");
        }
    }
    return 0;
}