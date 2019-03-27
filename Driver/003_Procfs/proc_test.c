#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#define PROC_DIR_NAME "test/test"

static int value = 0;
module_param(value, int, S_IRUGO);

static int proc_test_read(char *page, char **start, off_t offset, int count, int *eof, void *data)
{
    sprintf(page, "value=%d", value);
    printk(KERN_INFO "%s: value=%d\n", __func__, value);

    return 0;
}

static int proc_test_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
    sscanf(buffer, "%d", &value);
    printk(KERN_INFO "%s: value=%d\n", __func__, value);

    return count;
}

static int __init proc_test_init(void)
{
    int ret = 0;
    struct proc_dir_entry *entry = NULL;

    printk(KERN_INFO "%s entry\n", __func__);

    /* create procfs entry point under /proc */
    entry = create_proc_entry(PROC_DIR_NAME, 0666, NULL);
    if (entry)
    {
        entry->read_proc = proc_test_read;
        entry->write_proc = proc_test_write;
    }

    printk(KERN_INFO "%s exit\n", __func__);

    return ret;
}

static void __exit proc_test_exit(void)
{
    printk(KERN_INFO "%s entry\n", __func__);

    /* remove procfs entry point */
    remove_proc_entry(PROC_DIR_NAME, NULL);
}

module_init(proc_test_init);
module_exit(proc_test_exit);

MODULE_LICENSE("Dual BSD/GPL");
