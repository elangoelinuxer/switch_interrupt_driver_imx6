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
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/signal.h>
#include<linux/ioctl.h>

#include <asm/siginfo.h>        //siginfo    
#include <linux/sched.h>        //find_task_by_pid_type         


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/skbuff.h> 




//--------------------
#define SIG_TEST 44
struct siginfo info;        
struct task_struct *t;             
int ret=0;                


#define NETLINK_USER 31 

struct sock *nl_sk = NULL; 


//char *msg ;

static void hello_nl_recv_msg(struct sk_buff *skb);


         
//---------------------

char my_data[80]="hi from kernel"; 


 char  driver_name[] = "switch_interrupt";
int status=0;


struct switch_interrupt 
{
	struct cdev   cdev_own;
	struct class *class_own;
	dev_t devt;
};
struct switch_interrupt switch_interrupt;


// -----------for creating own attributes ----------------

struct kobject *kobj;
/*

static ssize_t my_show(struct kobject *kobj1, struct attribute *attr,char *buf)
{
    struct my_attr *a = container_of(attr, struct my_attr, attr);
    return scnprintf(buf, PAGE_SIZE, "%d\n", a->value);
}
*/

/*
static ssize_t my_store(struct kobject *kobj, struct attribute *attr,const char *buf, size_t len)
{
    struct my_attr *a = container_of(attr, struct my_attr, attr);
    sscanf(buf, "%d", &a->value);
    return sizeof(int);
}
*/

static struct sysfs_ops my_ops=
{

//	.show = my_show,
//	.store = my_store,

};


struct my_attr
{
	struct attribute attr;
	int value;

};


static struct my_attr my_first=
{

	.attr.name="first",
	.attr.mode=0664,
	.value=1,

};

static struct my_attr my_second=
{

	.attr.name="second",
	.attr.mode=0664,
	.value=2,

};

static struct attribute *my_attr[]=
{
	&my_first.attr,
	&my_second.attr,
	NULL

};


static struct kobj_type my_type=
{

	.sysfs_ops = &my_ops,
//	.default_attr = my_attr,

};


static int own_attr()
{

	kobj=kzalloc(sizeof(*kobj),GFP_KERNEL);

	if(kobj)
	{
		kobject_init(kobj,&my_type);


	}




}

//--------------------- end of dealing attributes-------------------------------------

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

static const struct file_operations irq_fops = 
{

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

static k=1;

static irq_handler_t my_isr(int num, void *dev_id,struct pt_regs *regs)
{

	printk("~~~~~~~~~~~~~~~mouse interrupt~~~~~~~~~~~~");
	
         //-------passing signal to user app
     
        
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

        //own_attr();

//----------------netlink-----------------------------       

nl_sk=netlink_kernel_create(&init_net, NETLINK_USER, 0,hello_nl_recv_msg, NULL, THIS_MODULE);  
    if(!nl_sk)  
    {   
            printk(KERN_ALERT "Error creating socket.\n");  
            return -10;  
    }  


//----------------------------------------------------   

        return 0;

fail_2:
        cdev_del(&switch_interrupt.cdev_own);
        unregister_chrdev_region(switch_interrupt.devt, 1);

fail_1:
        return -1;

}

//-----------netlink

static void hello_nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    int res;


    char *msg = " NO INTERRUPT since last check.... ";

    char *msg1 = " INTERRUPT OCCURED after your last check.... ";



//    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

if(status==0)
{
    msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data;

    printk(KERN_INFO "\n Driver Received an msg from User app......:  %s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid;  /*pid of sending process */
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);

    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");
}
else
{
   status=0; 

   msg_size = strlen(msg1);

    nlh = (struct nlmsghdr *)skb->data;

    printk(KERN_INFO "\n Driver Received an msg from User app......:  %s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid;  /*pid of sending process */
    skb_out = nlmsg_new(msg_size, 0);
    if (!skb_out)
    {
        printk(KERN_ERR "Failed to allocate new skb\n");
        return;
    }
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh), msg1, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);

    if (res < 0)
        printk(KERN_INFO "Error while sending bak to user\n");

}



} 

//----------------


static int  __exit  irq_exit1(void)
{

	int ret;

	printk(KERN_ALERT "~~~~~~~~~~~~~~~~~~~~~~~~~~~~  IRQ -  Driver removed .... bye ~~~~~~~~~~~~~~~~~~~~~~~~~~~ \n");

	cdev_del(&switch_interrupt.cdev_own);
	unregister_chrdev_region(switch_interrupt.devt,1);

	device_destroy(switch_interrupt.class_own, switch_interrupt.devt);
	class_destroy(switch_interrupt.class_own);

	free_irq(12, (void *)(my_isr));

        netlink_kernel_release(nl_sk);


}


module_init(irq_init);
module_exit(irq_exit1);

MODULE_AUTHOR("Elango_Palanismay");
MODULE_DESCRIPTION("interrupt driver for switch");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");


