#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/limits.h>

#define MAX_FILES 1000
#define LEFT_PANEL_X 0

struct FileInfo {
    char name[NAME_MAX];
    int is_dir;
    long size;
};

struct Panel {
    char path[PATH_MAX];
    struct FileInfo files[MAX_FILES];
    int files_count;
    int selected;
    int scroll_offset;
};

struct Panel left_panel, right_panel;
struct Panel *active_panel;


void init_panel(struct Panel *panel, const char *initial_path) {
    strncpy(panel->path, initial_path, PATH_MAX);
    panel->files_count = 0;
    panel->selected = 0;
    panel->scroll_offset = 0;
}

void load_directory(struct Panel *panel) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char full_path[PATH_MAX + FILENAME_MAX];

    panel->files_count = 0;

    if (strcmp(panel->path, "/") != 0) {
        strcpy(panel->files[panel->files_count].name, "..");
        panel->files[panel->files_count].is_dir = 1;
        panel->files[panel->files_count].size = 0;
        panel->files_count++;
    }

    dir = opendir(panel->path);

    if (dir == NULL) {
        strcpy(panel->files[panel->files_count].name, "An error occurred");
        panel->files[panel->files_count].is_dir = 0;
        panel->files[panel->files_count].size = 0;
        panel->files_count++;
        return;
    }

    while ((entry = readdir(dir)) != NULL && panel->files_count < MAX_FILES) {
        if (entry->d_name[0] == '.' || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        snprintf(full_path, PATH_MAX + FILENAME_MAX, "%s/%s", panel->path, entry->d_name);
        
        if (stat(full_path, &file_stat) == 0) {
            strncpy(panel->files[panel->files_count].name, entry->d_name, NAME_MAX);
            panel->files[panel->files_count].is_dir = S_ISDIR(file_stat.st_mode);
            panel->files[panel->files_count].size = file_stat.st_size;
            panel->files_count++;
        }
    }
    
    closedir(dir);
}

void draw_panel(WINDOW *win, struct Panel *panel, int is_active) {
    int max_y, max_x;
    getmaxyx(win, max_y, max_x);
    
    werase(win);
    
    box(win, 0, 0);
    
    mvwprintw(win, 0, 2, " %s ", panel->path);
    
    int visible_lines = max_y - 2;
    int end_index = panel->scroll_offset + visible_lines;
    if (end_index > panel->files_count) {
        end_index = panel->files_count;
    }
    
    for (int i = panel->scroll_offset; i < end_index; i++) {
        int display_line = i - panel->scroll_offset + 1;
        
        if (i == panel->selected && is_active) {
            wattron(win, A_REVERSE);
        }
        
        if (panel->files[i].is_dir) {
            wattron(win, A_BOLD);
            mvwprintw(win, display_line, 1, "[%s]", panel->files[i].name);
            wattroff(win, A_BOLD);
        } else {
            mvwprintw(win, display_line, 1, " %s", panel->files[i].name);
        }
        
        if (i == panel->selected && is_active) {
            wattroff(win, A_REVERSE);
        }
    }
    
    mvwprintw(win, max_y - 1, 1, " Files: %d ", panel->files_count);
    
    wrefresh(win);
}

void navigate_into(struct Panel *panel) {
    if (panel->files_count == 0) return;
    
    struct FileInfo *selected_file = &panel->files[panel->selected];
    char new_path[PATH_MAX + FILENAME_MAX];
    
    if (strcmp(selected_file->name, "..") == 0) {
        char *last_slash = strrchr(panel->path, '/');
        if (last_slash != NULL) {
            if (last_slash == panel->path) {
                strcpy(panel->path, "/");
            } else {
                *last_slash = '\0';
            }
        }
    } else if (selected_file->is_dir) {
        snprintf(new_path, PATH_MAX + FILENAME_MAX, "%s/%s", panel->path, selected_file->name);
        realpath(new_path, panel->path);
    }
    
    load_directory(panel);
    panel->selected = 0;
    panel->scroll_offset = 0;
}

void redraw(WINDOW *left_win, WINDOW *right_win, WINDOW *status_win) {
    draw_panel(left_win, &left_panel, active_panel == &left_panel);
    draw_panel(right_win, &right_panel, active_panel == &right_panel);
    
    werase(status_win);

    mvwprintw(status_win, 0, 0, " TAB: change panel | ENTER: open | q: exit ");
    
    wattroff(status_win, A_REVERSE);
    wrefresh(status_win);
}

void init_ncurses() {
    initscr();
    nodelay(stdscr, 1);
    raw();
    keypad(stdscr, 1);
    noecho();
    curs_set(0);
}

int main() {
    init_ncurses();

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int panel_width = (max_x - 10) / 2;

    init_panel(&left_panel, getenv("HOME"));
    init_panel(&right_panel, "/");
    active_panel = &left_panel;

    WINDOW *left_win = newwin(max_y - 1, panel_width, 0, LEFT_PANEL_X);
    WINDOW *right_win = newwin(max_y - 1, panel_width, 0, panel_width + 2);
    WINDOW *status_win = newwin(1, max_x, max_y - 1, 0);

    load_directory(&left_panel);
    load_directory(&right_panel);
    
    redraw(left_win, right_win, status_win);

    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                if (active_panel->selected > 0) {
                    active_panel->selected--;
                    
                    if (active_panel->selected < active_panel->scroll_offset) {
                        active_panel->scroll_offset--;
                    }
                }
                break;
                
            case KEY_DOWN:
                if (active_panel->selected < active_panel->files_count - 1) {
                    active_panel->selected++;
                    
                    int max_y, max_x;
                    getmaxyx(stdscr, max_y, max_x);
                    int visible_lines = max_y - 3;
                    if (active_panel->selected >= active_panel->scroll_offset + visible_lines) {
                        active_panel->scroll_offset++;
                    }
                }
                break;
                
            case '\t': 
                if (active_panel == &left_panel) {
                    active_panel = &right_panel;
                } else {
                    active_panel = &left_panel;
                }
                break;
                
            case '\n': 
                navigate_into(active_panel);
                break;
                
            case KEY_RESIZE:
                endwin();
                clear();
                
                getmaxyx(stdscr, max_y, max_x);
                panel_width = (max_x - 10) / 2;
                wresize(left_win, max_y - 1, panel_width);
                wresize(right_win, max_y - 1, panel_width);
                mvwin(right_win, 0, panel_width + 2);
                
                wresize(status_win, 1, max_x);
                mvwin(status_win, max_y - 1, 0);

                break;
        }
                
        redraw(left_win, right_win, status_win);
    }
    
    delwin(left_win);
    delwin(right_win);
    delwin(status_win);
    endwin();
}