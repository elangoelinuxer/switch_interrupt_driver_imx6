#include<linux/init.h>
#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/string.h>
#include<linux/device.h>
#include<linux/types.h>
#include <linux/slab.h>
#include<linux/stat.h>
#include<linux/fcntl.h>

const char  driver_name[] = "switch_interrupt";

int status=0;

struct switch_interrupt 
{
struct cdev   cdev_own;
struct class *class_own;
dev_t devt;
};
struct switch_interrupt switch_interrupt;

static ssize_t irq_read(struct file *filp, char __user *buff, size_t count,loff_t *offp)
{

printk(KERN_ALERT " Read operation ....\n");
 
if(status)
{

printk(KERN_ALERT " Interrupt occured after last check .... \n");
status=0;

}
else
{

printk(KERN_ALERT " No interrupt occurence since last check .... \n");


}



return 0;
}

static ssize_t irq_write(struct file* F, const char *buf[], size_t count, loff_t *f_pos)
{

printk(KERN_ALERT " write operation ......\n");

return 0;
}

static int irq_open( struct inode *inode, struct file *filp)
{

printk(KERN_ALERT " file opened .......\n");

return 0;
}

static const struct file_operations irq_fops = {

.owner = THIS_MODULE,
.read =  irq_read,
.write = irq_write,
.open =  irq_open,

};

static int __init  irq_cdev(void)
{

int ret;

switch_interrupt.devt=MKDEV(0,0);

ret=alloc_chrdev_region(&switch_interrupt.devt,0,1,driver_name);

if (ret < 0) 
{
     printk(KERN_ALERT "alloc_chrdev_region() failed: %d \n", ret);
     return -1;
}

cdev_init(&switch_interrupt.cdev_own,&irq_fops); 
switch_interrupt.cdev_own.owner = THIS_MODULE;

ret = cdev_add(&switch_interrupt.cdev_own, switch_interrupt.devt, 1);

if(ret) 
{
printk(KERN_ALERT "cdev_add() failed: %d\n", ret);
unregister_chrdev_region(switch_interrupt.devt, 1);
return -1;
}

return 0;

}

static int __init  irq_class(void)
{

switch_interrupt.class_own = class_create(THIS_MODULE,driver_name); 

if (!switch_interrupt.class_own) 
        {
                printk(KERN_ALERT "class_create() failed\n");
                return -1;
        }

if (!device_create(switch_interrupt.class_own, NULL, switch_interrupt.devt, NULL,driver_name))
{
                printk(KERN_ALERT "device_create(.....%s) failed\n",driver_name);
                class_destroy(switch_interrupt.class_own);
                return -1;
}
        return 0;

}

static irq_handler_t my_isr(int num, void *dev_id,struct pt_regs *regs)
{

printk("~~~~~~~~~~~~~~~mouse interrupt~~~~~~~~~~~~");

status=1;

return (irq_handler_t) IRQ_HANDLED;

}

static int __init  irq_init(void)
{

int ret;

printk(KERN_ALERT "~~~~~~~~~~~~~~~~~~~~~~~~~~~~  IRQ -  Driver inserted ... hai ~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");
memset(&switch_interrupt,0,sizeof(switch_interrupt));

ret=request_irq (12, (irq_handler_t) my_isr, IRQF_SHARED, "mouse_irq", (void *)(my_isr));

        if (irq_cdev() < 0)       //function to create device nodes
                goto fail_1;

        if (irq_class() < 0)      //function to create class
                goto fail_2;

        return 0;

fail_2:
        cdev_del(&switch_interrupt.cdev_own);
        unregister_chrdev_region(switch_interrupt.devt, 1);

fail_1:
        return -1;

}

static int  __exit  irq_exit1(void)
{

int ret;

printk(KERN_ALERT "~~~~~~~~~~~~~~~~~~~~~~~~~~~~  IRQ -  Driver removed .... bye ~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

cdev_del(&switch_interrupt.cdev_own);
unregister_chrdev_region(switch_interrupt.devt,1);

device_destroy(switch_interrupt.class_own, switch_interrupt.devt);
class_destroy(switch_interrupt.class_own);

free_irq(12, (void *)(my_isr));

}


module_init(irq_init);
module_exit(irq_exit1);

MODULE_AUTHOR("Elango_Palanismay");
MODULE_DESCRIPTION("interrupt driver for switch");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");


