#include <linux/module.h>
#include <linux/kernel.h>

int init_module(void) {
    pr_info("The module1 has been loaded.\n");

    return 0;
}

void cleanup_module(void) {
    pr_info("The module1 has been unloaded.\n");
}

MODULE_LICENSE("GPL");