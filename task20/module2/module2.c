#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MAX_SIZE 50
static int major = 0;
static char test_string[MAX_SIZE] = "This is a message from module.";

ssize_t test_read(struct file *fd, char __user *buff, size_t size, loff_t *off)
{
    size_t len = strlen(test_string);
    
    pr_info("module2: test_read: size=%zu, off=%lld, len=%zu\n", size, *off, len);
    
    if (*off >= len)
        return 0;
    
    if (size > len - *off)
        size = len - *off;
    
    if (copy_to_user(buff, test_string + *off, size))
        return -EFAULT;
    
    *off += size;
    return size;
}

ssize_t test_write(struct file *fd, const char __user *buff, size_t size, loff_t *off)
{
    if (size > MAX_SIZE)
        return -EINVAL;
    
    if (copy_from_user(test_string, buff, size))
        return -EFAULT;
    
    if (size < MAX_SIZE)
        test_string[size] = '\0';
    
    pr_info("module2: test_write: wrote %zu bytes\n", size);
    
    return size;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = test_read,
    .write = test_write,
};

int init_module(void)
{
    pr_info("module2: Test module is loaded.\n");
    major = register_chrdev(major, "module2", &fops);
    
    if (major < 0)
        return major;
    
    pr_info("module2: Major number is %d.\n", major);
    return 0;
}

void cleanup_module(void)
{
    unregister_chrdev(major, "module2");
    pr_info("module2: Module unloaded.\n");
}

MODULE_LICENSE("GPL");
