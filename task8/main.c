#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>  
#include "plugin_loader.h"

struct plugin *plugins;
int plugins_num = 0;
int next_menu_num = 2;

void print_menu() {
    printf("Menu\n");
    printf("1 - Load plugin\n");
    for (int i = 0; i < plugins_num; i++) {
        printf("%d - %s\n", i + 2, plugins[i].menu_text);
    }

    printf("%d - Exit\n", plugins_num + 2);
}

void load_new_plugin() {
    char plugin_path[256];
    
    printf("Enter a name of plugin: ");
    scanf("%255s", plugin_path);
    
    struct plugin* new_plugin = load_plugin(plugin_path);
    if (!new_plugin) {
        printf("Load plugin error.\n");
        return;
    }
    
    struct plugin* new_plugins = realloc(plugins, (plugins_num + 1) * sizeof(struct plugin));
    if (!new_plugins) {
        printf("Memory allocation error.\n");
        dlclose(new_plugin->handle);
        free(new_plugin->menu_text);
        free(new_plugin);
        return;
    }
    
    plugins = new_plugins;
    
    plugins[plugins_num].menu_text = new_plugin->menu_text;
    plugins[plugins_num].operation_func = new_plugin->operation_func;
    plugins[plugins_num].menu_number = next_menu_num++;
    plugins[plugins_num].handle = new_plugin->handle;
    
    free(new_plugin);
    
    plugins_num++;
    printf("Plugin has been loaded successfully. You can choose it with option %d\n", plugins[plugins_num - 1].menu_number);

}

void cleanup() {
    for (int i = 0; i < plugins_num; i++) {
        unload_plugin(&plugins[i]);
    }
    
    if (plugins) {
        free(plugins);
        plugins = NULL;
    }
}

int get_operand()
{
    int operand;
    while (scanf("%d", &operand) != 1)
    {
        while (getchar() != '\n');
        printf("Invalid input. Please enter a number: ");
    }

    return operand;
}

int main() {
    char option;
    int operand1;
    int operand2;    
    int result;
    char printResult;

    while(1) {
        printResult = 0;
        print_menu();
        
        printf("Please enter an option number: ");
        scanf(" %c", &option);

        int option_num = option - '0';

        if (option_num == 1) {
            load_new_plugin();
            continue;
        }
        
        if (option_num == plugins_num + 2) {
            break;
        }

        if (option_num >= 2 && option_num < plugins_num + 2) {
            int plgIndex = -1;
            for (int i = 0; i < plugins_num; i++) {
                if (plugins[i].menu_number == option_num) {
                    plgIndex = i;
                    break;
                }
            }
            
            if (plgIndex == -1) {
                printf("Incorrect option number.\n");
                continue;
            }
            
            printf("Enter the 1st operand: ");
            operand1 = get_operand();
            
            printf("Enter the 2nd operand: ");
            operand2 = get_operand();
            
            result = plugins[plgIndex].operation_func(operand1, operand2);
            printf("Result: %d\n", result);
            
        } else {
            printf("Incorrect option number.\n");
        }
        
        printf("\n");
    }

    cleanup();
    return 0;
}