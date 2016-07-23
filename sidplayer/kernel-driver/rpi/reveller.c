#include <linux/kern_levels.h>

#include <linux/module.h>

#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/irqdomain.h>

#include <linux/ioport.h>
#include <asm/io.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <linux/slab.h>
#include <linux/circ_buf.h>

#include <linux/sched.h>
#include <linux/wait.h>

#include <asm/uaccess.h>

#include <asm/delay.h>
/*

#include <linux/irq.h>
#include <plat/regs-timer.h>
*/

#define WAIT 1000000
#define TIMER_NO 1
#define TIMER_MASK (1 << TIMER_NO)
/* This 'magic' number has been found by listening to Turbo_Outrun and comparing with actual
 * recordings from Stone Oakvalley. This value adjusts for the delay between the requested timer
 * interrupt and actual time of interrupt.
 */
#define TIMER_SUBTRACT 5

// IOCTL
enum {
REVELLER_CLEAR = 1024,
REVELLER_PAUSE
};

static int rpi_peri_base = 0;
static int rpi_irq_no = 0;
#define GPIO_OFFSET  0x200000
#define TIMER_OFFSET 0x3000
#define PWM_OFFSET   0x20C000
#define CLOCK_OFFSET 0x101000

MODULE_LICENSE("GPL");

static void __iomem *timer = NULL;
static void __iomem *counter_lo = NULL;
static void __iomem *compare = NULL;

static void __iomem *gpio = NULL;
static void __iomem *gpio0_fsel = NULL;
static void __iomem *gpio1_fsel = NULL;
static void __iomem *gpio2_fsel = NULL;

static void __iomem *gpio0_set = NULL;
static void __iomem *gpio0_level = NULL;
static void __iomem *gpio0_clear = NULL;

static dev_t reveller_dev = 0;
static int reveller_major = 0;
static struct class *reveller_class = NULL;
static struct cdev *reveller_cdev = NULL;
static struct device *reveller_device = NULL;

static int timer_active = 0;
static DECLARE_WAIT_QUEUE_HEAD(reveller_wq);

#define CIRC_BUFFER_SIZE (1 << 16)
static struct circ_buf cb;

static inline void reveller_set_timer(unsigned int next) {
    unsigned int now = readl_relaxed(counter_lo);
    writel_relaxed(now + next - TIMER_SUBTRACT, compare);
}

static void sid_write(uint8_t reg, uint8_t data) {
	uint8_t data_01 = data & 0x3;
	uint8_t data_23 = (data & 0xC) >> 2;
	uint8_t data_4567 = (data & 0xF0) >> 4;
	
	uint32_t set_pins = 0, clear_pins = 0;
	set_pins |= data_01;		// compatability with v1
	set_pins |= data_01 << 2;	// and v2
	set_pins |= data_23 << 14;
	set_pins |= data_4567 << 22;

	set_pins |= (reg & 0x1F) << 7;	// SID Address
	
	// SET Pins
	writel_relaxed(set_pins, gpio0_set);
	
	// Clear Pins
	clear_pins |= (1 << 17);	// SID read/Write
	clear_pins |= (1 << 4);		// SID CS
	
	while (readl_relaxed(gpio0_level) & (1 << 18)); 	// wait for low
	writel_relaxed(clear_pins, gpio0_clear);
	while (!(readl_relaxed(gpio0_level) & (1 << 18)));	// wait for high
	while (readl_relaxed(gpio0_level) & (1 << 18)); 	// wait for low
	
	clear_pins = 0x03C0CF8F;	// Clear 0-3, 7-11, 14-15, 22-25
	set_pins  = (1 << 17);		// SID Read/write
	set_pins |= (1 << 4);		// SID CS
	writel_relaxed(set_pins, gpio0_set);
	writel_relaxed(clear_pins, gpio0_clear);
}

static void inline advance_tail(void) {
    cb.tail = (cb.tail + 1) & (CIRC_BUFFER_SIZE - 1);
}

static uint32_t get_uint24_t(void) {
    uint32_t ret = 0;

    ret |= (cb.buf[cb.tail] & 0xff) << 16;
    advance_tail();

    ret |= (cb.buf[cb.tail] & 0xff) << 8;
    advance_tail();

    ret |= (cb.buf[cb.tail] & 0xff);
    advance_tail();

    return ret;
}

// always four available
static unsigned int consume(int avail) {
    while (avail > 4) {
        uint32_t cmd = cb.buf[cb.tail];
        advance_tail();
        avail -= 1;

        switch (cmd & 0xe0) {
        case 0x20:
            cmd = get_uint24_t();

            if (cmd < 20) {
                avail -= 3;
                continue;
            }

            return cmd;
        case 0:
            sid_write(cmd & 0x1f, cb.buf[cb.tail]);
            advance_tail();
            avail -= 1;

            break;
        default:
            printk(KERN_DEBUG "reveller: Unhandled command from producer: %hhx\n", (uint8_t) (cmd & 0xe0));
        }
    }

    return 0;
}

static irqreturn_t reveller_interrupt(int irq, void *dev_id) {
    unsigned int t = readl_relaxed(timer);

    if (t & TIMER_MASK) {
        int avail;
        unsigned int next;

        writel_relaxed(TIMER_MASK, timer);

        avail = CIRC_CNT(cb.head, cb.tail, CIRC_BUFFER_SIZE);
        next = consume(avail);

        wake_up(&reveller_wq);
        if (next == 0) {
            timer_active = 0;
            return IRQ_HANDLED;
        }

        reveller_set_timer(next);
        return IRQ_HANDLED;
    } else {
        return IRQ_NONE;
    }
}

static void reveller_clear(void) {
    cb.head = cb.tail = 0;
    timer_active = 0;
}

static int reveller_chardev_open(struct inode *inode, struct file *filp) {
    return 0;
}
static int reveller_chardev_release(struct inode *inode, struct file *filp) {
    reveller_clear();
    sid_write(0x18, 0);
    return 0;
}
static ssize_t reveller_chardev_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
    return 0;
}
static long reveller_chardev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
        case REVELLER_CLEAR:
            reveller_clear();
            break;
        default:
            return 1;
    }

    return 0;
}



static ssize_t reveller_chardev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
    size_t consumed = 0;

    while (consumed < count) {
        size_t space = CIRC_SPACE(cb.head, cb.tail, CIRC_BUFFER_SIZE);

        if (space == 0) {
            if (wait_event_interruptible_timeout(reveller_wq, (space = CIRC_SPACE(cb.head, cb.tail, CIRC_BUFFER_SIZE)) > 0, msecs_to_jiffies(1000)) == 0) {
                printk(KERN_WARNING "reveller: timeout while waiting for free buffer. restarting IRQ\n");
                reveller_set_timer(10000);
            }
        } else {
            size_t remaining = count - consumed;
            size_t to_end = CIRC_SPACE_TO_END(cb.head, cb.tail, CIRC_BUFFER_SIZE);
            size_t bytes = (to_end > remaining) ? remaining : to_end;
            if (copy_from_user(&cb.buf[cb.head], &buf[consumed], bytes)) {
                return -EFAULT;
            }

            consumed += bytes;
            cb.head = (cb.head + bytes) & (CIRC_BUFFER_SIZE - 1);
        }
    }

    if (! timer_active) {
        timer_active = 1;
        reveller_set_timer(1000);
    }

    return 0;
}

static struct file_operations reveller_fops = {
  .owner = THIS_MODULE,
  .read = reveller_chardev_read,
  .write = reveller_chardev_write,
  .unlocked_ioctl = reveller_chardev_ioctl,
  .open = reveller_chardev_open,
  .release = reveller_chardev_release
};

static void reveller_cleanup(void) {
    // this is the order we initialize
    // major
    // class
    // cdev
    // device

    if (reveller_device != NULL) {
        device_destroy(reveller_class, reveller_dev);
    }

    if (reveller_cdev != NULL) {
        cdev_del(reveller_cdev);
        kfree(reveller_cdev);
    }

    if (reveller_class != NULL) {
        class_destroy(reveller_class);
    }

    if (reveller_major != 0) {
        unregister_chrdev_region(reveller_dev, 1);
    }

    if (cb.buf != NULL) {
        kfree(cb.buf);
    }

    if (timer != NULL) {
        writel(0x0, compare);
        iounmap(timer);
    }

    if (gpio != NULL) {
        iounmap(gpio);
    }
}

static void reveller_init_gpio(void) {
    int result;
    
    gpio = ioremap(rpi_peri_base + GPIO_OFFSET, 4096);
    gpio0_fsel = gpio;
    gpio1_fsel = gpio + 4;
    gpio2_fsel = gpio + 8;

    result  = readl_relaxed(gpio0_fsel);
    result &= 0x1f8000;
    writel_relaxed(result, gpio0_fsel);
    result |= 0x9201249;
    writel_relaxed(result, gpio0_fsel);

    result  = readl_relaxed(gpio1_fsel);
    // clear all except 12, 13, 16 and 19
    result &= 0x381c0fc0;
    //  enable input on 10, 11, 14, 15, 17
    writel_relaxed(result, gpio1_fsel);
    // enable output on 10, 11, 14, 15, 17
    // alt.fun. 5 on 18
    result |= 0x2209009;
    writel_relaxed(result, gpio1_fsel);

    // function selection register 2, GPIO 20-29
    result = readl_relaxed(gpio2_fsel);
    // clear 21, 22-25, 27
    result &= 0x3f1C0007;
    //  enable input on 21, 22-25, 27
    writel_relaxed(result, gpio2_fsel);
    // enable output on 21, 22-25, 27
    result |= 0x209248;
    writel_relaxed(result, gpio2_fsel);

    gpio0_set = gpio + 0x1c;
    gpio0_clear = gpio + 0x28;
    gpio0_level = gpio + 0x34;

    writel_relaxed(0x08200000, gpio0_clear);
}

static void reveller_init_pwm(void) {
    unsigned int pwm_pwd = (0x5A << 24);
    void __iomem *pwm = ioremap(rpi_peri_base + PWM_OFFSET, 4096);
    void __iomem *rng1 = pwm + 0x10;
    void __iomem *dat1 = pwm + 0x14;

    void __iomem *clock = ioremap(rpi_peri_base + CLOCK_OFFSET, 4096);
    void __iomem *clock_cntl = clock + 0xa0;
    void __iomem *clock_div = clock + 0xa4;

    // Stop PWM clock
    writel_relaxed(pwm_pwd | 0x01, clock_cntl);
    udelay(110);

    // Wait for the clock to be not busy
    while ((readl_relaxed(clock_cntl) & 0x80) != 0) {
        udelay(1);
    }

    // set the clock divider and enable PWM clock
    // hardcoded to PAL freqs. We get a clock on PWM of PLLD (500MHz) / 254 / 2= 0.984MHz,
    writel_relaxed(pwm_pwd | (254 << 12), clock_div);
    writel_relaxed(pwm_pwd | 0x16, clock_cntl);

    writel_relaxed(0x80 | 1, pwm);

    writel_relaxed(2, rng1);
    writel_relaxed(1, dat1);

    iounmap(clock);
    iounmap(pwm);
}

static int reveller_init(void) {
    int result = 0;
    const char* model;
    struct device_node *dn;

    if ((dn = of_find_compatible_node(NULL, NULL, "brcm,bcm2708")) != NULL) {
        rpi_peri_base = 0x20000000;
        rpi_irq_no = 24 + TIMER_NO;
    } else if ((dn = of_find_compatible_node(NULL, NULL, "brcm,bcm2709")) != NULL) {
        rpi_peri_base = 0x3f000000;
        rpi_irq_no = 30 + TIMER_NO;
    }

    if (dn == NULL) {
        printk(KERN_WARNING "reveller: unable to detect raspberry pi platform. running on an old kernel? "
                            "this module requires a kernel with device tree support\n");
        goto fail;
    }

    of_property_read_string(dn, "model", &model);
    printk("reveller: initializing on %s\n", model);
    of_node_put(dn);

    cb.buf = kzalloc(CIRC_BUFFER_SIZE, GFP_KERNEL);
    // timer_resource = request_mem_region(TIMER_BASE, 4096, "Reveller");
    timer = ioremap(rpi_peri_base + TIMER_OFFSET, 4096);
    if (timer == NULL) {
        goto fail;
    }

    counter_lo = timer + 0x4;
    compare = timer + 0xc + (4 * TIMER_NO);// + 4;

    // unsigned int mapped_irq = irq_create_mapping(NULL, IRQ_NO);
    // printk(KERN_DEBUG "reveller: mapped %d to %d\n", IRQ_NO, mapped_irq);

    result = request_irq(rpi_irq_no, reveller_interrupt, 0, "Reveller", NULL);
    if (result < 0) {
        printk(KERN_DEBUG "reveller: failed to register irq handler\n");
        goto fail;
    }

    reveller_init_gpio();
    reveller_init_pwm();

    /* Get a range of minor numbers (starting with 0) to work with */
    result = alloc_chrdev_region(&reveller_dev, 0, 1, "reveller");
    if (result < 0) {
        printk(KERN_WARNING "reveller: alloc_chrdev_region() failed\n");
        goto fail;
    }
    reveller_major = MAJOR(reveller_dev);

    printk(KERN_DEBUG "reveller: registered with major %d\n", reveller_major);

    /* Create device class (before allocation of the array of devices) */
    reveller_class = class_create(THIS_MODULE, "reveller");
    if (IS_ERR(reveller_class)) {
        result = PTR_ERR(reveller_class);
        goto fail;
    }

    reveller_cdev = cdev_alloc();
    cdev_init(reveller_cdev, &reveller_fops);
    reveller_cdev->owner = THIS_MODULE;

    result = cdev_add(reveller_cdev, reveller_dev, 1);
    if (result) {
        printk(KERN_WARNING "reveller: Error %d while trying to add %s%d",
               result, "reveller", 0);
        goto fail;
    }

    reveller_device = device_create(reveller_class, NULL, /* no parent device */ 
        reveller_dev, NULL, /* no additional data */
        "reveller");

    if (IS_ERR(reveller_device)) {
        result = PTR_ERR(reveller_device);
        printk(KERN_WARNING "reveller: Error %d while trying to create %s%d",
            result, "reveller", 0);
        goto fail;
    }

    return 0;

    fail:
    reveller_cleanup();
    return result;
}

static void reveller_exit(void) {
    reveller_cleanup();

    /*
    if (timer_resource != NULL) {
        release_mem_region(TIMER_BASE, 4096);
    }
    */

    free_irq(rpi_irq_no, NULL);
    printk(KERN_DEBUG "reveller: exit()\n");
}


module_init(reveller_init);
module_exit(reveller_exit);

