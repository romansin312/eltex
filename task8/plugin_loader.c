#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include "plugin_loader.h"

typedef const char* (*get_menu_text_func_t)(void);
typedef int (*operation_func_t)(int, int);

struct plugin* load_plugin(const char* path) {
    char local_path[256];
    sprintf(local_path, "./%s.so", path);
    void* handle = dlopen(local_path, RTLD_LAZY);
    if (!handle) {
        printf("Error loading plugin: %s\n", dlerror());
        return NULL;
    }
    
    get_menu_text_func_t get_menu_text = dlsym(handle, "get_plugin_menu_text");
    if (!get_menu_text) {
        printf("Plugin doesn't have get_plugin_menu_text function\n");
        dlclose(handle);
        return NULL;
    }
    
    operation_func_t operation_func = dlsym(handle, "plugin_operation");
    if (!operation_func) {
        printf("Plugin doesn't have plugin_operation function\n");
        dlclose(handle);
        return NULL;
    }
    
    const char* menu_text = get_menu_text();
    if (!menu_text) {
        printf("Plugin menu text is empty\n");
        dlclose(handle);
        return NULL;
    }
    
    struct plugin* plg = malloc(sizeof(struct plugin));
    if (!plg) {
        printf("malloc error for plugin\n");
        dlclose(handle);
        return NULL;
    }

    plg->menu_text = malloc(strlen(menu_text) + 1);
    if (!plg->menu_text) {
        printf("malloc error for menu text\n");
        free(plg);
        dlclose(handle);
        return NULL;
    }

    strcpy(plg->menu_text, menu_text);
    
    plg->operation_func = operation_func;
    plg->handle = handle;
    plg->menu_number = 0;
    
    return plg;
}

void unload_plugin(struct plugin* plg) {
    if (!plg) return;
    
    free(plg->menu_text);
    
    if (plg->handle) {
        dlclose(plg->handle);
    }
    
    plg->menu_text = NULL;
    plg->operation_func = NULL;
    plg->handle = NULL;
}
