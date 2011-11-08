#include <linux/module.h>

#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/irq.h>

#include <linux/fs.h>

#include <asm/uaccess.h>

MODULE_LICENSE("GPL");

// 240 - 254 is reserved for experimental
int sid_timer_major = 240;

static int sid_timer_irq_triggered = 0;
static DECLARE_WAIT_QUEUE_HEAD(sid_timer_wq);

static irqreturn_t sid_timer_interrupt(int irq, void *dev_id) {
	sid_timer_irq_triggered = 1;
	wake_up_all(&sid_timer_wq);
	return IRQ_HANDLED;
}

/*
static struct irqaction sid_timer_irq = {
	.name		= "SID Timer Tick",
	.flags		= 0,
	.handler	= sid_timer_interrupt,
};
*/

int sid_chardev_open(struct inode *inode, struct file *filp) {
	return 0;
}
int sid_chardev_release(struct inode *inode, struct file *filp) {
	return 0;
}
ssize_t sid_chardev_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
	wait_event_interruptible(sid_timer_wq, sid_timer_irq_triggered == 1);
	copy_to_user(buf, "1", 1);
	sid_timer_irq_triggered = 0;
	return 1;
}
ssize_t sid_chardev_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos) {
	return 0;
}

struct file_operations sid_timer_fops = {
  read: sid_chardev_read,
  write: sid_chardev_write,
  open: sid_chardev_open,
  release: sid_chardev_release
};

static int sid_timer_init(void) {
	int result;
	
	printk("<1>sid_timer_init\n");
	// setup_irq(IRQ_TIMER1, &sid_timer_irq);
	result = request_irq(IRQ_TIMER1, sid_timer_interrupt, 0, "SID Timer Tick", NULL);
	if (result < 0) {
	    printk("<1>sid_timer: failed to register irq handler\n");
    	return result;
  	}
	
	result = register_chrdev(sid_timer_major, "sid_timer", &sid_timer_fops);
	if (result < 0) {
	    printk("<1>sid_timer: cannot obtain major number %d\n", sid_timer_major);
    	return result;
  	}
	
	return 0;
}

static void sid_timer_exit(void) {
	unregister_chrdev(sid_timer_major, "sid_timer");
	free_irq(IRQ_TIMER1, NULL);
	printk("<1>sid_timer_exit\n");
}


module_init(sid_timer_init);
module_exit(sid_timer_exit);

