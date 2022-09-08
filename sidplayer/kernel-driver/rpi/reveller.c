#include <linux/kern_levels.h>

#include <linux/module.h>

#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/irqdomain.h>

#include <linux/ioport.h>
#include <asm/io.h>

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/poll.h>

#include <linux/slab.h>
#include <linux/circ_buf.h>

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/sort.h>

#include <asm/uaccess.h>

#include <asm/delay.h>

#define TIMER_NO 1
#define TIMER_MASK (1 << TIMER_NO)

// IOCTL
enum {
REVELLER_FLUSH = 1024,
REVELLER_PAUSE,
REVELLER_RESUME,
REVELLER_POWER
};

static struct device_node *dn = NULL;
static struct device_node *dn_pwm = NULL;
static struct device_node *dn_timer = NULL;
static struct device_node *dn_gpio = NULL;
static struct device_node *dn_clock = NULL;

static void __iomem *rpi_pwm_base = NULL;
static void __iomem *rpi_gpio_base = NULL;
static void __iomem *rpi_clock_base = NULL;
static void __iomem *rpi_timer_base = NULL;
static int rpi_irq_no = 0;

#define GPIO_OFFSET  0x200000
#define TIMER_OFFSET 0x3000
#define PWM_OFFSET   0x20C000
#define CLOCK_OFFSET 0x101000

MODULE_LICENSE("GPL");

static void __iomem *counter_lo = NULL;
static void __iomem *compare = NULL;

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

static uint8_t c64_sid_register[0x1f];

#define SELFTUNE_TIMEOUT 20000
#define SELFTUNE_N (1000000 / SELFTUNE_TIMEOUT)
static uint selftune_n = 0;
static u32 *selftune_results;
static u32 selftuning_adjust = 0;

static inline void reveller_set_timer(uint next) {
    uint now = readl(counter_lo);
    uint n = next - selftuning_adjust;
    // n may overflow AND require at least 10us before next
    if ((n <= next) && (n > 10)) {
        writel(now + next - selftuning_adjust, compare);
    } else {
        writel(now + 10, compare);
    }
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
	writel(set_pins, gpio0_set);

	// Clear Pins
	clear_pins |= (1 << 17);	// SID read/Write
	clear_pins |= (1 << 4);		// SID CS

	while (readl(gpio0_level) & (1 << 18)); 	// wait for low
	writel(clear_pins, gpio0_clear);
	while (!(readl(gpio0_level) & (1 << 18)));	// wait for high
	while (readl(gpio0_level) & (1 << 18)); 	// wait for low

	set_pins  = (1 << 17);		// SID Read/write
	set_pins |= (1 << 4);		// SID CS
	writel(set_pins, gpio0_set);

	clear_pins = 0x03C0CF8F;	// Clear 0-3, 7-11, 14-15, 22-25
	writel(clear_pins, gpio0_clear);

	c64_sid_register[reg & 0x1f] = data;
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
static uint consume(int avail) {
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

irq_handler_t reveller_interrupt_handler;
static irqreturn_t reveller_interrupt(int irq, void *dev_id) {
    return reveller_interrupt_handler(irq, dev_id);
}

static irqreturn_t reveller_interrupt_playback(int irq, void *dev_id) {
    uint t = readl(rpi_timer_base);

    if (t & TIMER_MASK) {
        int avail;
        uint next;

        writel(TIMER_MASK, rpi_timer_base);
        avail = CIRC_CNT(cb.head, cb.tail, CIRC_BUFFER_SIZE);
        next = consume(avail);

        wake_up_interruptible(&reveller_wq);
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

static int cmp_u32(const void *pa, const void *pb) {
    u32 a = *(u32 *)pa;
    u32 b = *(u32 *)pb;

    if (a < b) {
        return -1;
    } else if (a > b) {
        return 1;
    } else {
        return 0;
    }
}

static irqreturn_t reveller_interrupt_tune(int irq, void *dev_id) {
    uint t = readl(rpi_timer_base);

    if (t & TIMER_MASK) {
        u32 now = readl(counter_lo);
        writel(TIMER_MASK, rpi_timer_base);
        selftune_results[selftune_n] = now;
        selftune_n++;

        if (selftune_n >= SELFTUNE_N) {
            u32 selftune_diffs[SELFTUNE_N];
            int i;

            for (i = 1; i < SELFTUNE_N; ++i) {
                selftune_diffs[i] = selftune_results[i]-selftune_results[i - 1];
            }
            sort(&selftune_diffs[1],
                SELFTUNE_N - 1,
                sizeof(u32),
                cmp_u32,
                NULL);
            selftuning_adjust = selftune_diffs[SELFTUNE_N / 2] - SELFTUNE_TIMEOUT;
            printk(KERN_DEBUG "reveller: selftuned offset: %u\n", selftuning_adjust);
            kfree(selftune_results);

            reveller_interrupt_handler = reveller_interrupt_playback;
        } else {
            writel(now + SELFTUNE_TIMEOUT, compare);
        }

        return IRQ_HANDLED;
    } else {
        return IRQ_NONE;
    }
}

static void reveller_flush(void) {
    cb.head = cb.tail = 0;
    timer_active = 0;
}

static void reveller_power(unsigned int state) {
    u32 fsel = readl(gpio2_fsel);
    // clear 21 and 27
    fsel &= 0x3f1fffc7;
    if (state) {
        // set output enable on 21 and 27
        fsel |= 0x200008;
    }
    writel(fsel, gpio2_fsel);
}

static int reveller_chardev_open(struct inode *inode, struct file *filp) {
    selftuning_adjust = 0;
    selftune_n = 0;
    selftune_results = kmalloc(SELFTUNE_N * sizeof(u32), GFP_KERNEL);
    reveller_interrupt_handler = reveller_interrupt_tune;
    writel(readl(counter_lo) + SELFTUNE_TIMEOUT, compare);
    return 0;
}
static int reveller_chardev_release(struct inode *inode, struct file *filp) {
    reveller_flush();
    sid_write(0x18, 0);
    return 0;
}
static ssize_t reveller_chardev_read(struct file *filp, char *buf, size_t count, loff_t *f_pos) {
    return 0;
}
static long reveller_chardev_ioctl(struct file *filp, uint cmd, unsigned long arg) {
    switch (cmd) {
        case REVELLER_FLUSH:
            reveller_flush();
            break;
        case REVELLER_PAUSE:
            sid_write(0x18, 0);
            writel(0x0, compare);
            timer_active = 0;
            break;
        case REVELLER_RESUME:
            sid_write(0x18, c64_sid_register[0x18]);
            reveller_set_timer(1000);
            timer_active = 1;
            break;
        case REVELLER_POWER:
            reveller_power(arg);
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

static uint reveller_poll(struct file *filp, poll_table *wait) {
    poll_wait(filp, &reveller_wq, wait);
    /* poll for at least 1/8 of CIRC_BUFFER_SIZE before reporting avail
     *
     * because a write cycle writes more than just a couple of bytes,
     * we wait until a relatively large buffer is avail.
     */
    if (CIRC_SPACE(cb.head, cb.tail, CIRC_BUFFER_SIZE) > (CIRC_BUFFER_SIZE / 8)) {
        return (POLLOUT | POLLWRNORM);
    }

    return 0;
}

static struct file_operations reveller_fops = {
  .owner = THIS_MODULE,
  .read = reveller_chardev_read,
  .write = reveller_chardev_write,
  .poll = reveller_poll,
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

   if (rpi_timer_base != NULL) {
       writel(0x0, compare);
       iounmap(rpi_timer_base);
   }
   if (rpi_gpio_base != NULL) {
       iounmap(rpi_gpio_base);
   }
   if (rpi_clock_base != NULL) {
       iounmap(rpi_clock_base);
   }
   if (rpi_pwm_base != NULL) {
       iounmap(rpi_pwm_base);
   }

    if (rpi_irq_no != 0) {
        free_irq(rpi_irq_no, NULL);
    }

    if (dn != NULL) {
        of_node_put(dn);
    }
    if (dn_pwm != NULL) {
        of_node_put(dn_pwm);
    }
    if (dn_gpio != NULL) {
        of_node_put(dn_gpio);
    }
    if (dn_clock != NULL) {
        of_node_put(dn_clock);
    }
    if (dn_timer != NULL) {
        of_node_put(dn_timer);
    }
}

static void reveller_init_gpio(void) {
    int result;
    
    gpio0_fsel = rpi_gpio_base;
    gpio1_fsel = rpi_gpio_base + 4;
    gpio2_fsel = rpi_gpio_base + 8;

    result  = readl(gpio0_fsel);
    result &= 0x1f8000;
    writel(result, gpio0_fsel);
    result |= 0x9201249;
    writel(result, gpio0_fsel);

    result  = readl(gpio1_fsel);
    // clear all except 12, 13, 16 and 19
    result &= 0x381c0fc0;
    //  enable input on 10, 11, 14, 15, 17
    writel(result, gpio1_fsel);
    // enable output on 10, 11, 14, 15, 17
    // alt.fun. 5 on 18
    result |= 0x2209009;
    writel(result, gpio1_fsel);

    // function selection register 2, GPIO 20-29
    result = readl(gpio2_fsel);
    // clear 21, 22-25, 27
    result &= 0x3f1C0007;
    //  enable input on 21, 22-25, 27
    writel(result, gpio2_fsel);
    // enable output on 21, 22-25, 27
    result |= 0x209248;
    writel(result, gpio2_fsel);

    gpio0_set = rpi_gpio_base + 0x1c;
    gpio0_clear = rpi_gpio_base + 0x28;
    gpio0_level = rpi_gpio_base + 0x34;

    writel(0x08200000, gpio0_clear);
}

static void reveller_init_pwm(int use_osc) {
    uint pwm_pwd = (0x5A << 24);
    void __iomem *rng1 = rpi_pwm_base + 0x10;
    void __iomem *dat1 = rpi_pwm_base + 0x14;

    void __iomem *clock_cntl = rpi_clock_base + 0xa0;
    void __iomem *clock_div = rpi_clock_base + 0xa4;

    // Stop PWM clock
    writel(pwm_pwd | 0x01, clock_cntl);
    udelay(110);

    // Wait for the clock to be not busy
    while ((readl(clock_cntl) & 0x80) != 0) {
        udelay(1);
    }

    // set the clock divider and enable PWM clock
    // hardcoded to PAL freqs. We get a clock on PWM of
    //
    // On Pi4: Dervied from OSC (54MHz) / 27 / 2 = 1MHz
    // Others: PLLD (500MHz) / 254 / 2 = 0.984MHz

    if (use_osc) {
        writel(pwm_pwd | (27 << 12), clock_div);
        writel(pwm_pwd | 0x11, clock_cntl);
    } else {
        writel(pwm_pwd | (254 << 12), clock_div);
        writel(pwm_pwd | 0x16, clock_cntl);
    }

    writel(0x80 | 1, rpi_pwm_base);

    writel(2, rng1);
    writel(1, dat1);
}

static int reveller_init(void) {
    int result = 0;
    int use_osc = 0;
    const char* model;

    // RPi 1
    if ((dn = of_find_compatible_node(NULL, NULL, "brcm,bcm2835")) != NULL) {
        dn_timer = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-system-timer");
        if (dn_timer == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi system timer\n");
            goto fail;
        }
        rpi_timer_base = of_iomap(dn_timer, 0);
        rpi_irq_no = irq_of_parse_and_map(dn_timer, TIMER_NO);
        if (rpi_irq_no <= 0) {
            pr_err("Can't parse IRQ\n");
            result = -EINVAL;
            goto fail;
        }

        dn_gpio = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-gpio");
        if (dn_gpio == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi gpio\n");
            goto fail;
        }
        rpi_gpio_base = of_iomap(dn_gpio, 0);

        dn_clock = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-cprman");
        if (dn_clock == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi clock\n");
            goto fail;
        }
        rpi_clock_base = of_iomap(dn_clock, 0);
    // RPi 2, RPi 3
    } else if (((dn = of_find_compatible_node(NULL, NULL, "brcm,bcm2836")) != NULL) ||
               ((dn = of_find_compatible_node(NULL, NULL, "brcm,bcm2837")) != NULL)) {
        uint mmc_interrupt;
        struct device_node *dn_mmc = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-sdhost");
        if (dn_mmc == NULL) {
            printk(KERN_WARNING "reveller: unable to find brcm,bcm2835-sdhost\n");
            goto fail;
        }

        dn_gpio = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-gpio");
        if (dn_gpio == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi gpio\n");
            goto fail;
        }
        rpi_gpio_base = of_iomap(dn_gpio, 0);

        dn_clock = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-cprman");
        if (dn_clock == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi clock\n");
            goto fail;
        }
        rpi_clock_base = of_iomap(dn_clock, 0);

        // this has just been found by trial and error
        dn_clock = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-cprman");

        // because the brcm,bcm2835-system-timer is not exposed on 2836 and 2837
        // we can find the correct irq via one of the other devices exposed on the
        // same interrupt controller. we use sdhost here, which is irq no. 56
        // locally (ref. https://datasheets.raspberrypi.org/bcm2711/bcm2711-peripherals.pdf page 84)
        mmc_interrupt = irq_of_parse_and_map(dn_mmc, 0);
        of_node_put(dn_mmc);
        rpi_irq_no = mmc_interrupt - 56 + TIMER_NO;

        // similar, we need to directly call ioremap to map the timer registers
        rpi_timer_base = ioremap(0x3f000000 + TIMER_OFFSET, 4096);
    // RPi 4
    } else if ((dn = of_find_compatible_node(NULL, NULL, "brcm,bcm2711")) != NULL) {
        dn_timer = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-system-timer");
        if (dn_timer == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi system timer\n");
            goto fail;
        }
        rpi_timer_base = of_iomap(dn_timer, 0);
        rpi_irq_no = irq_of_parse_and_map(dn_timer, TIMER_NO);
        if (rpi_irq_no <= 0) {
            pr_err("Can't parse IRQ\n");
            result = -EINVAL;
            goto fail;
        }

        dn_gpio = of_find_compatible_node(NULL, NULL, "brcm,bcm2711-gpio");
        if (dn_gpio == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi gpio\n");
            goto fail;
        }
        rpi_gpio_base = of_iomap(dn_gpio, 0);

        dn_clock = of_find_compatible_node(NULL, NULL, "brcm,bcm2711-cprman");
        if (dn_clock == NULL) {
            printk(KERN_WARNING "reveller: unable to detect raspberry pi clock\n");
            goto fail;
        }
        rpi_clock_base = of_iomap(dn_clock, 0);
        use_osc = 1;
    }

    if (dn == NULL) {
        printk(KERN_WARNING "reveller: unable to detect raspberry pi platform. running on an old kernel? "
                            "this module requires a kernel with device tree support\n");
        goto fail;
    }

    dn_pwm = of_find_compatible_node(NULL, NULL, "brcm,bcm2835-pwm");
    if (dn_pwm == NULL) {
        printk(KERN_WARNING "reveller: unable to detect raspberry pi pwm\n");
        goto fail;
    }
    rpi_pwm_base = of_iomap(dn_pwm, 0);

    of_property_read_string(dn, "model", &model);
    printk("reveller: initializing on %s\n", model);

    cb.buf = kzalloc(CIRC_BUFFER_SIZE, GFP_KERNEL);
    counter_lo = rpi_timer_base + 0x4;
    compare = rpi_timer_base + 0xc + (4 * TIMER_NO);// + 4;

    result = request_irq(rpi_irq_no, reveller_interrupt, 0, "Reveller", NULL);
    if (result < 0) {
        printk(KERN_DEBUG "reveller: failed to register irq handler\n");
        goto fail;
    }

    reveller_init_gpio();
    reveller_init_pwm(use_osc);

    /* Get a range of minor numbers (starting with 0) to work with */
    result = alloc_chrdev_region(&reveller_dev, 0, 1, "reveller");
    if (result < 0) {
        printk(KERN_WARNING "reveller: alloc_chrdev_region() failed\n");
        goto fail;
    }
    reveller_major = MAJOR(reveller_dev);

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
    printk(KERN_DEBUG "reveller: exit()\n");
}


module_init(reveller_init);
module_exit(reveller_exit);

