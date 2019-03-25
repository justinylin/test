#include <linux/module.h>
#include <linux/init.h>

int hello_init(void)
{
    printk(KERN_ALERT "Hello World\n");
    return 0;
}

void hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(hello_init);
module_exit(hello_exit);

