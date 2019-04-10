#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>

#define HELLO_DEV_NAME "hello"

struct hello_dev {
    struct cdev cdev;
    dev_t devno;
    struct class *my_class;
    struct device *my_device;
};

struct hello_dev *mydev = NULL;

struct file_operations hello_fops = {
    .owner = THIS_MODULE,
};

static int __init hello_init (void)
{
    int ret = 0;
    dev_t devno = 0;
    int major = 0;

    ret = alloc_chrdev_region(&devno, 0, 1, HELLO_DEV_NAME);
    if (ret < 0)
    {
        printk(KERN_ERR "alloc_chrdev_region failed\n");
        return ret;
    }

    major = MAJOR(devno);

    mydev = kmalloc(sizeof(struct hello_dev), GFP_KERNEL);
    if (NULL == mydev)
    {
        unregister_chrdev_region(devno, 1);
        return -ENOMEM;
    }

    cdev_init(&mydev->cdev, &hello_fops);
    mydev->cdev.owner = THIS_MODULE;
    mydev->devno = devno;

    ret = cdev_add(&mydev->cdev, devno, 1);

    /* class create */
    mydev->my_class = class_create(THIS_MODULE, "my_class");
    if (IS_ERR(mydev->my_class))
    {
        printk("class_create failed\n");
        cdev_del(&mydev->cdev);
        unregister_chrdev_region(devno, 1);
        kfree(mydev);
        return -1;
    }

    /* device create */
    mydev->my_device = device_create(mydev->my_class, NULL, devno, NULL, "hello");
    if (IS_ERR(mydev->my_device))
    {
        printk(KERN_ERR "device_create failed\n");
        class_destroy(mydev->my_class);
        cdev_del(&mydev->cdev);
        unregister_chrdev_region(devno, 1);
        kfree(mydev);
        return -1;
    }

    return 0;
}

static void __exit hello_exit (void)
{
    device_destroy(mydev->my_class, mydev->devno);
    class_destroy(mydev->my_class);
    cdev_del(&mydev->cdev);
    unregister_chrdev_region(mydev->devno, 1);
    kfree(mydev);
}

module_init (hello_init);
module_exit (hello_exit);

MODULE_LICENSE ("Dual BSD/GPL");
