#include<linux/init.h>
#include<linux/module.h>
#include<linux/irq.h>
#include<linux/interrupt.h>
#include<linux/fs.h>
#include<linux/signal.h>

static int dev_open(struct inode * ,struct file * );

int ela=1;

static irq_handler_t my_isr(int num, void *dev_id,struct pt_regs *regs)
{


printk("~~~~~~~~~~~~~~~mouse interrupt~~~~~~~~~~~~");

return (irq_handler_t) IRQ_HANDLED;

}


static struct file_operations fops =
{
.open=dev_open,

};


static int dev_open(struct inode *inod,struct file *fil)
{
int ret;


printk("~~~~~~~~~~~~~~~~~hello in open function~~~~~~~~~~~~~~~~~~~~");


};


static int ela_init()
{

int t,ret;

printk(KERN_ALERT "~~~~~~~~~~~~~~~~~~~~~~~~~~~~  IRQ -  Driver inserted ... hai ~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

t=register_chrdev(79,"elango",&fops);

//ret=request_irq(1,my_isr,IRQF_SHARED,"irq",NULL);

ret=request_irq (12, (irq_handler_t) my_isr, IRQF_SHARED, "mouse_irq", (void *)(my_isr));

if(ret)
printk("cant share interrupt for mouse");



return 0;

}

static void ela_exit()
{

printk(KERN_ALERT "~~~~~~~~~~~~~~~~~~~~~~~~~~~~  IRQ -  Driver removed .... bye ~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

 free_irq(12, (void *)(my_isr));

}


module_init(ela_init);
module_exit(ela_exit);
