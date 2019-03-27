#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

#define PROC_DIR_NAME "test"
#define PROC_NODE_NAME "test"
#define PROC_NODE_PATH "test/test"

static bool flag = 0;
struct proc_dir_entry *proc_test_dir = NULL;
struct proc_dir_entry *proc_test_file = NULL;

static int proc_test_show(struct seq_file *m, void *v)
{
    seq_printf(m, "%s\n", flag?"true":"false");

    return 0;
}

static ssize_t proc_test_write(struct file *file, const char __user *buffer, size_t count, loff_t *pos)
{
    char mode = 0;

    if (0 < count)
    {
        if (get_user(mode, buffer))
        {
            return -EFAULT;
        }
        flag = (mode != '0');
    }

    return count;
}

static int proc_test_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_test_show, NULL);
}

static const struct file_operations proc_test_fops = {
    .owner = THIS_MODULE,
    .open = proc_test_open,
    .read = seq_read,
    .write = proc_test_write,
    .llseek = seq_lseek,
    .release = single_release,
};

static int __init proc_test_init(void)
{

    /* create /proc/test directory */
    proc_test_dir = proc_mkdir(PROC_DIR_NAME, NULL);
    if (NULL == proc_test_dir)
        return -ENOMEM;

    /* create /proc/test/test node */
    proc_test_file = proc_create(PROC_NODE_NAME, 0644, proc_test_dir, &proc_test_fops);
    if (NULL == proc_test_file)
    {
        /* on failure */
        proc_remove(proc_test_dir);
        return -ENOMEM;
    }

    return 0;
}

static void __exit proc_test_exit(void)
{
    /* remove /proc/test/test node */
    proc_remove(proc_test_file);

    /* remove /proc/test directory */
    proc_remove(proc_test_dir);
}

module_init(proc_test_init);
module_exit(proc_test_exit);

MODULE_LICENSE("Dual BSD/GPL");
