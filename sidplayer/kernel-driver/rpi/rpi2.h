#define PERI_BASE   0x3f000000
#define GPIO_BASE  (PERI_BASE + 0x200000)
#define TIMER_BASE (PERI_BASE + 0x3000)
#define PWM_BASE   (PERI_BASE + 0x20C000)
#define CLOCK_BASE (PERI_BASE + 0x101000)

#define IRQ_NO 30 + TIMER_NO