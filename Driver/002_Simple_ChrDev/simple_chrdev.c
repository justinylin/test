#include<linux/init.h>
#include<linux/module.h>
#include<linux/uaccess.h>
#include<linux/fs.h>


static unsigned char simple_inc = 0; //character device only contain one char value
static unsigned char demoBuffer[4096] = {0}; /* One Page */
unsigned int simple_MAJOR = 224;

int simple_open(struct inode *inode, struct file *filp)
{
	if (simple_inc > 0)
		return -ERESTARTSYS;

	simple_inc++;

	return 0;
}

int simple_release(struct inode *inode, struct file *filp)
{
	simple_inc--;

	return 0;
}

ssize_t simple_read(struct file *filp, char __user *buf, size_t count, loff_t *f_ops)
{
	if (copy_to_user(buf, demoBuffer, count))
	{
		count = -EFAULT;
	}

	return count;
}

ssize_t simple_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_ops)
{
	if (copy_from_user(demoBuffer + *f_ops, buf, count))
	{
		count = -EFAULT;
	}

	return count;
}

struct file_operations simple_fops = {
	.owner = THIS_MODULE,
	.read = simple_read,
	.write = simple_write,
	.open = simple_open,
	.release = simple_release,
};

int simple_init_module(void)
{
	int ret = 0;
	
	ret = register_chrdev(simple_MAJOR, "simple", &simple_fops);
	
	if(ret < 0)
	{
		printk("Unable to register character device %d!\n", simple_MAJOR);
		return ret;
	}
	
	return 0;
}

void simple_cleanup_module(void)
{
	unregister_chrdev(simple_MAJOR, "simple");
	printk("simple_cleanup_module!\n");
}

module_init(simple_init_module);
module_exit(simple_cleanup_module);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("<justin.y.lin@hotmail.com>");
MODULE_DESCRIPTION("Simple Characterise Device Driver v1.0, 2014/03/25");
