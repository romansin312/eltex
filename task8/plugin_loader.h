#ifndef __PLUGIN_LOADER_H__
#define __PLUGIN_LOADER_H__

struct plugin {
    char *menu_text;
    int (*operation_func)(int, int);
    int menu_number;
    void* handle;
};

struct plugin* load_plugin(const char* path);
void unload_plugin(struct plugin* plg);

#endif