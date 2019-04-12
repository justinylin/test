#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/ioctl.h>

#define GLOBAL_VAR "globalvar"
#define GLOBAL_VAR_CLS "globalvar"
#define GLOBAL_VAR_DEV "globalvar"

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

/* ioctl opcode */
#define GLOBAL_VAR_TYPE 'g'
#define GLOBAL_VAR_GET_SIZE_NR (1)
#define GLOBAL_GET_SIZE _IOR(GLOBAL_VAR_TYPE, GLOBAL_VAR_GET_SIZE_NR, int)

struct globalvar_dev
{
	char global_var[PAGE_SIZE];
	dev_t devno;
	struct cdev cdev;
	struct class *global_var_cls;
	struct device *global_var_dev;
	struct semaphore sem;
};

struct globalvar_dev *my_dev;

int globalvar_open(struct inode *inode, struct file *filp)
{
	struct globalvar_dev *dev;

	dev = container_of(inode->i_cdev, struct globalvar_dev, cdev);
	filp->private_data = dev;

	return 0;
}

int globalvar_release(struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t globalvar_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	size_t read_count;
	struct globalvar_dev *dev = filp->private_data;

	/* try to obtain semaphore */
	if (down_interruptible(&dev->sem))
	{
		/* semaphore unavailable */
		return -ERESTARTSYS;
	}

	/* check offset */
	if (*offp >= sizeof(dev->global_var))
	{
		up(&dev->sem);
		return -EFAULT;
	}

	/* check count */
	if (*offp + count >= sizeof(dev->global_var))
	{
		read_count = sizeof(dev->global_var) - *offp;
	}
	else
	{
		read_count = count;
	}

	if (copy_to_user(buf, &dev->global_var[*offp], read_count))
	{
		up(&dev->sem);
		return -EFAULT;
	}

	/* release semaphore */
	up(&dev->sem);

	return read_count;
}

/* TODO: Is the last byte '\0' or not? */
ssize_t globalvar_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
	size_t write_count;
	struct globalvar_dev *dev = filp->private_data;

	/* try to obtain semaphore */
	if (down_interruptible(&dev->sem))
	{
		/* semaphore unavailable */
		return -ERESTARTSYS;
	}

	/* check offset */
	if (*offp >= sizeof(dev->global_var))
	{
		up(&dev->sem);
		return -EFAULT;
	}

	/* check count */
	if (*offp + count >= sizeof(dev->global_var))
	{
		write_count = sizeof(dev->global_var) - *offp;
	}
	else
	{
		write_count = count;
	}

	if (copy_from_user(&dev->global_var[*offp], buf, write_count))
	{
		up(&dev->sem);
		return -EFAULT;
	}

	/* release semaphore */
	up(&dev->sem);

	return write_count;
}

static long globalvar_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int size = 0;
	struct globalvar_dev *dev = filp->private_data;

	/* try to obtain semaphore */
	if (down_interruptible(&dev->sem))
	{
		/* semaphore unavailable */
		return -ERESTARTSYS;
	}

	/* check type */
	if (GLOBAL_VAR_TYPE != _IOC_TYPE(cmd))
	{
		printk(KERN_ERR "wrong type\n");
		up(&dev->sem);
		return -ENOTTY;
	}

	switch (_IOC_NR(cmd))
	{
	case GLOBAL_VAR_GET_SIZE_NR:
		/* get size */
		if (_IOC_READ != _IOC_DIR(cmd))
		{
			printk(KERN_ERR "wrong direction\n");
			up(&dev->sem);
			return -ENOTTY;
		}
		else
		{
			if(access_ok(VERIFY_WRITE, (void __user *)&arg, sizeof(size)))
			{
				/* access ok */

				size = sizeof(dev->global_var);

				if (0 != copy_to_user((void __user *)&arg, (void *)&size, sizeof(size)))
				{
					/* copy_to_user failed */
					up(&dev->sem);
					return -EFAULT;
				}
			}
			else
			{
				/* access failed */
				printk(KERN_ERR "user space addr can't access\n");
				up(&dev->sem);
				return -EFAULT;
			}
		}
		break;

	default:
		up(&dev->sem);
		return -ENOTTY;
		break;
	}

	/* release semaphore */
	up(&dev->sem);

	return 0;
}

struct file_operations globalvar_fops = {
	.owner = THIS_MODULE,
	.open = globalvar_open,
	.release = globalvar_release,
	.read = globalvar_read,
	.write = globalvar_write,
        .unlocked_ioctl = globalvar_ioctl,
#ifdef CONFIG_COMPAT
        .compat_ioctl   = globalvar_ioctl,
#endif
};

static int __init globalvar_init(void)
{
	int ret = 0;
	dev_t devno = 0;
	int dev_major = 0;

	/* allocate devno */
	ret = alloc_chrdev_region(&devno, 0, 1, GLOBAL_VAR);
	if (ret < 0)
	{
		printk(KERN_ERR "allocate devno failed\n");
		return -EINVAL;
	}

	dev_major = MAJOR(devno);

	/* allocate memory */
	my_dev = kmalloc(sizeof(struct globalvar_dev), GFP_KERNEL);
	if (NULL == my_dev)
	{
		ret = -ENOMEM;
		printk(KERN_ERR "allocate memory failed\n");
		goto __unregister_devno;
	}

	/* semaphore init, unlocked */
	sema_init(&my_dev->sem, 1);

	/* create cdev */
	memset(my_dev->global_var, 0, sizeof(my_dev->global_var));

	my_dev->devno = devno;

	cdev_init(&my_dev->cdev, &globalvar_fops);

	my_dev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&my_dev->cdev, devno, 1);
	if ( ret < 0)
	{
		printk(KERN_ERR "add device failed\n");
		goto __kfree;
	}

	/* class_create & device_create */
	my_dev->global_var_cls = class_create(THIS_MODULE, GLOBAL_VAR_CLS);
	if (IS_ERR(my_dev->global_var_cls))
	{
		printk(KERN_ERR "create class failed\n");
		ret = PTR_ERR(my_dev->global_var_cls);
		goto __cdev_del;
	}

	my_dev->global_var_dev = device_create(my_dev->global_var_cls, NULL, devno, NULL, GLOBAL_VAR_DEV);
	if (IS_ERR(my_dev->global_var_dev))
	{
		printk(KERN_ERR "create device failed\n");
		ret = PTR_ERR(my_dev->global_var_dev);
		goto __class_destroy;
	}

	return ret;

__class_destroy:
	class_destroy(my_dev->global_var_cls);
__cdev_del:
	cdev_del(&my_dev->cdev);
__kfree:
	kfree(my_dev);
__unregister_devno:
	unregister_chrdev_region(devno, 1);

	return ret;
}

static void __exit globalvar_exit(void)
{
	dev_t devno = my_dev->devno;

	device_destroy(my_dev->global_var_cls, devno);
	class_destroy(my_dev->global_var_cls);
	cdev_del(&my_dev->cdev);
	kfree(my_dev);
	unregister_chrdev_region(devno, 1);
}

module_init(globalvar_init);
module_exit(globalvar_exit);

MODULE_LICENSE("Dual BSD/GPL");
