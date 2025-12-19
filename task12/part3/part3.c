#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CMD_INPUT 256
#define MAX_ARGS 20
#define MAX_PIPES 20

char** get_args(char* command)
{
    char** args = malloc(sizeof(char*) * (MAX_ARGS + 1));
    int argsCount = 0;
    char cmd_copy[MAX_CMD_INPUT];

    char* token = strtok(command, " ");
    while (token != NULL && argsCount < MAX_ARGS)
    {
        args[argsCount] = malloc(strlen(token) + 1);
        strcpy(args[argsCount], token);
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

char*** parse_pipeline(char* input, int* cmd_count)
{
    char** commands = malloc(sizeof(char*) * MAX_PIPES);
    *cmd_count = 0;

    char input_cpy[MAX_CMD_INPUT];
    strcpy(input_cpy, input);

    char* cmd_token = strtok(input_cpy, "|");
    while (cmd_token != NULL && *cmd_count < MAX_PIPES)
    {
        while (*cmd_token == ' ')
            cmd_token++;

        char* end = cmd_token + strlen(cmd_token) - 1;
        while (end > cmd_token && *end == ' ')
        {
            *end = '\0';
            end--;
        }

        if (strlen(cmd_token) > 0)
        {
            char cmd[MAX_CMD_INPUT];
            commands[*cmd_count] = malloc(sizeof(char) * MAX_CMD_INPUT);
            strcpy(commands[*cmd_count], cmd_token);
            (*cmd_count)++;
        }

        cmd_token = strtok(NULL, "|");
    }

    char*** cmd_args_pipeline = malloc(sizeof(char**) * MAX_PIPES);
    for (int i = 0; i < *cmd_count; i++)
    {
        cmd_args_pipeline[i] = get_args(commands[i]);
        free(commands[i]);
    }
    free(commands);

    return cmd_args_pipeline;
}

void free_commands(char*** commands, int cmd_count)
{
    for (int i = 0; i < cmd_count; i++)
    {
        if (commands[i] != NULL) {
            for (int j = 0; commands[i][j] != NULL; j++) {
                free(commands[i][j]);
            }

            free(commands[i]);
        }
    }
    free(commands);
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

        int cmd_count;
        char*** commands = parse_pipeline(command, &cmd_count);
        if (cmd_count == 1)
        {
            args = commands[0];

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
                free_commands(commands, cmd_count);
                exit(EXIT_FAILURE);
            }
            else if (child_pid > 0)
            {
                wait(NULL);
                free_commands(commands, cmd_count);
            }
            else
            {
                free_commands(commands, cmd_count);
                printf("fork error\n");
            }
        } else {
            int pipes[MAX_PIPES - 1][2];
            pid_t pids[MAX_PIPES];
            
            for (int i = 0; i < cmd_count - 1; i++)
            {
                if (pipe(pipes[i]) == -1)
                {
                    printf("pipe error\n");
                    free_commands(commands, cmd_count);
                    continue;
                }
            }

            for (int i = 0; i < cmd_count; i++)
            {
                pids[i] = fork();

                if (pids[i] == 0)
                {
                    if (i > 0)
                    {
                        dup2(pipes[i - 1][0], STDIN_FILENO);
                    }

                    if (i < cmd_count - 1)
                    {
                        dup2(pipes[i][1], STDOUT_FILENO);
                    }

                    for (int j = 0; j < cmd_count - 1; j++)
                    {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }

                    execvp(commands[i][0], commands[i]);
                    printf("command execution error: %s\n", commands[i][0]);
                    free_commands(commands, cmd_count);
                    exit(EXIT_FAILURE);
                }
                else if (pids[i] < 0)
                {
                    printf("fork error\n");
                    free_commands(commands, cmd_count);
                    exit(EXIT_FAILURE);
                }
            }

            for (int i = 0; i < cmd_count - 1; i++)
            {
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            for (int i = 0; i < cmd_count; i++)
            {
                wait(NULL);
            }

            free_commands(commands, cmd_count);
        }
    }
    return 0;
}