#include <linux/module.h>

#include <asm/io.h>

MODULE_LICENSE("GPL");

static int sid_init(void) {
	void *mmapped;
	/*
	unsigned int value;
	*/

	printk("<1>sid_init\n");
	/* switch mode on pwm pin */
	mmapped = ioremap(0x44E10848, 0x4);
	iowrite32(0x6, mmapped);
	
	/*
	mmapped = ioremap(0x481AC134, 4);
	value = ioread32(mmapped);
	printk("<1>value from regs: %u\n", value);
	*/
	
	return 0;
}

static void sid_exit(void) {
	printk("<1>sid_exit\n");
}


module_init(sid_init);
module_exit(sid_exit);

