#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <curses.h>

#define SERVER_QUEUE_NAME   "/chat_server"
#define MAX_MESSAGE_SIZE    256
#define MAX_NAME_LENGTH     32
#define CHAT_WIDTH_PERCENT 0.8
#define MAX_CHAT_HISTORY 1000

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

WINDOW *chat_win, *users_win, *input_win;
char client_name[MAX_NAME_LENGTH];
mqd_t server_queue, client_queue;
pthread_t receive_thread;
int running = 1;
char *chat_history[MAX_CHAT_HISTORY] = {0};
int chat_messages_count = 0;

void render_input_window(char input[MAX_MESSAGE_SIZE]){
    box(input_win, 0, 0);
    mvwprintw(input_win, 0, 2, " Type your message here ");

    wmove(input_win, 1, 1);
    wclrtoeol(input_win);
    if (input == NULL)
    {
        input = "";
    }
    
    mvwprintw(input_win, 1, 1, "> %s", input);

    wrefresh(input_win);
}

void render_chat_window() {
    werase(chat_win);
    box(chat_win, 0, 0);
    mvwprintw(chat_win, 0, 2, " Chat ");
    for (int i = 0; i < chat_messages_count; i++) {
        mvwprintw(chat_win, i + 2, 1, "%s", chat_history[i]);
        wrefresh(chat_win);
    }

    wrefresh(chat_win);
}

void init_ncurses() {
    initscr();
    cbreak();
    noecho();
    curs_set(1);
    keypad(stdscr, TRUE);
    
    int max_x, max_y;
    getmaxyx(stdscr, max_x, max_y);
    
    int chat_width = max_y * CHAT_WIDTH_PERCENT;
    chat_win = newwin(max_x - 3, chat_width, 0, 0);
    
    int users_width = max_y - chat_width;
    users_win = newwin(max_x - 3, users_width, 0, chat_width);
    
    input_win = newwin(3, max_y, max_x - 3, 0);
}

void add_message_to_chat(const char *sender, const char *text, time_t timestamp) {
    struct tm *tm_info = localtime(&timestamp);
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
    
    chat_history[chat_messages_count] = malloc(sizeof(char) * MAX_MESSAGE_SIZE);
    sprintf(chat_history[chat_messages_count], "[%s] %s: %s", time_str, sender, text);

    chat_messages_count++;

    render_chat_window();
}

void update_users_list(char *user_list_text) {
    werase(users_win);
    box(users_win, 0, 0);
    mvwprintw(users_win, 0, 2, " Participants ");

    if (user_list_text != NULL) {
        char *text_copy = strdup(user_list_text);
        char *token = strtok(text_copy, "\n");
        int row = 1;
        
        while (token != NULL && row < getmaxy(users_win) - 1) {
            mvwprintw(users_win, row, 1, "%s", token);
            row++;
            token = strtok(NULL, "\n");
        }
        
        free(text_copy);
    }

    wrefresh(users_win);
}

void* receive_messages(void* arg) {
    chat_message msg;
    
    while (running) {
        ssize_t bytes_read = mq_receive(client_queue, (char*)&msg, sizeof(chat_message), NULL);
        
        if (bytes_read > 0) {
            switch (msg.type) {
                case MSG_TYPE_TEXT:
                    add_message_to_chat(msg.sender, msg.text, msg.timestamp);
                    break;
                    
                case MSG_TYPE_USER_LIST:
                    update_users_list(msg.text);
                    break;
            }
        }
    }

    return NULL;
}

void send_message_to_server(chat_message *msg) {
    if (mq_send(server_queue, (char*)msg, sizeof(chat_message), 0) == -1) {
        printf("error on send message to server\n");
    }
}

void cleanup() {
    running = 0;
    
    chat_message disconnect_msg;
    disconnect_msg.type = MSG_TYPE_DISCONNECT;
    strcpy(disconnect_msg.sender, client_name);
    send_message_to_server(&disconnect_msg);
    
    mq_close(server_queue);
    mq_close(client_queue);
    
    char client_queue_name[MAX_NAME_LENGTH * 2];
    snprintf(client_queue_name, sizeof(client_queue_name), "/chat_client_%s", client_name);
    mq_unlink(client_queue_name);
    
    endwin();
}

void signal_handler(int sig) {
    cleanup();
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <user_name>\n", argv[0]);
        return 1;
    }
    
    strncpy(client_name, argv[1], MAX_NAME_LENGTH);
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    server_queue = mq_open(SERVER_QUEUE_NAME, O_WRONLY);
    if (server_queue == (mqd_t)-1) {
        printf("server queue open error\n");
        return 1;
    }
    
    char client_queue_name[MAX_NAME_LENGTH * 2];
    snprintf(client_queue_name, sizeof(client_queue_name), "/chat_client_%s", client_name);
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(chat_message);
    attr.mq_curmsgs = 0;
    
    client_queue = mq_open(client_queue_name, O_CREAT | O_RDONLY, 0600, &attr);
    if (client_queue == (mqd_t)-1) {
        printf("client mq open error");
        mq_close(server_queue);
        return 1;
    }
    
    pthread_create(&receive_thread, NULL, receive_messages, NULL);

    init_ncurses();
    update_users_list(NULL);     
    render_chat_window();

    chat_message connect_msg;
    connect_msg.type = MSG_TYPE_CONNECT;
    strcpy(connect_msg.sender, client_name);
    send_message_to_server(&connect_msg);

    char input_buffer[MAX_MESSAGE_SIZE] = "";
    int input_pos = 0;


    while (running) {        
        render_input_window(input_buffer);

        int ch = wgetch(input_win);
        
        if (ch == '\n' || ch == KEY_ENTER) {
            if (input_pos > 0) {
                input_buffer[input_pos] = '\0';
                
                chat_message text_msg;
                text_msg.type = MSG_TYPE_TEXT;
                strcpy(text_msg.sender, client_name);
                strncpy(text_msg.text, input_buffer, MAX_MESSAGE_SIZE - 1);
                text_msg.text[MAX_MESSAGE_SIZE - 1] = '\0';
                
                send_message_to_server(&text_msg);
                                
                input_pos = 0;
                input_buffer[0] = '\0';
            }
        } else if (ch == 127 || ch == KEY_BACKSPACE) {
            if (input_pos > 0) {
                input_pos--;
                input_buffer[input_pos] = '\0';
            }
        } else if (ch >= 32 && ch <= 126 && input_pos < MAX_MESSAGE_SIZE - 1) {
            input_buffer[input_pos++] = ch;
            input_buffer[input_pos] = '\0';
        } else if (ch == 3) {
            break;
        }
    }
    
    cleanup();
    pthread_join(receive_thread, NULL);
    
    return 0;
}