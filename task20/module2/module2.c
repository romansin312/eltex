#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#define MAX_SIZE 50

static int major = 0;
static DEFINE_MUTEX(lock);
static char test_string[MAX_SIZE] = "This is a message from module.\0";

ssize_t test_read(struct file *fd, char __user *buff, size_t size, loff_t *off)
{
    size_t rc;

    mutex_lock(&lock);
    rc = simple_read_from_buffer(buff, size, off, test_string, MAX_SIZE);
    mutex_unlock(&lock);

    return rc;
}

ssize_t test_write(struct file *fd, const char __user *buff, size_t size, loff_t *off)
{
    size_t rc = 0;
    if (size > MAX_SIZE)
        return -EINVAL;

    mutex_lock(&lock);
    rc = simple_write_to_buffer(test_string, MAX_SIZE, off, buff, size);
    mutex_unlock(&lock);

    return rc;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = test_read,
    .write = test_write,
};

int init_module(void)
{
    pr_info("Test module is loaded.\n");
    major = register_chrdev(major, "module2", &fops);

    if (major < 0)
        return major;
    pr_info("module2: Major number is %d.\n", major);

    return 0;
}

void cleanup_module(void)
{
    unregister_chrdev(major, "module2");
}

MODULE_LICENSE("GPL");
