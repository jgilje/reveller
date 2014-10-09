#ifndef _BCM2835_H_
#define _BCM2835_H_

/*
 Pin mapping: 
    GPIO0-3   D0-D1 (RPi v1 0-1, RPi v2 2-3)
    GPIO4     CS
    GPIO7-11  A0-A4
    GPIO14-15 D2-D3
    GPIO17    RW
    GPIO18    CLK
    GPIO22-25 D4-D7
    
    GPIO21 ReadCLK (v1)
    GPIO27 ReadCLK (v2)
*/

#define REG (volatile uint32_t *)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define GPIO_BASE		(BCM2835_BASE + 0x200000)
#define PWM_BASE		(BCM2835_BASE + 0x20C000)
#define CLOCK_BASE		(BCM2835_BASE + 0x101000)

// Section 9.3.1

// 128kB (8kB?)
#define CONTROL_MODULE		0x44E10000

#define CM_PER			0x44E00000
#define GPIO_0			0x44E07000
#define GPIO_1			0x4804C000
#define GPIO_2			0x481AC000
#define GPIO_3			0x481AE000

#define PWM_1			0x48302000
#define EPWM_OFFSET		(0x200/4)

// PWM Config
// page 182

// PWMSS_CTRL 9.3.1.31

// Section 25 - GPIO offsets
#define GPIO_CLEAR_OFFSET		(0x190 / 4)
#define GPIO_SET_OFFSET			(0x194 / 4)
#define GPIO_OE_OFFSET			(0x134 / 4)

// Section 8.1.12 - CM_PER offsets
#define CM_PER_GPIO1_CLKCTRL		(0xAC / 4)
#define CM_PER_GPIO2_CLKCTRL		(0xB0 / 4)
#define CM_PER_GPIO3_CLKCTRL		(0xB4 / 4)

#define CM_PER_EPWMSS0_CLKCTRL		(0xD4 / 4)
#define CM_PER_EPWMSS1_CLKCTRL		(0xCC / 4)
#define CM_PER_EPWMSS2_CLKCTRL		(0xD8 / 4)

#define CLKCTRL_MODE_ENABLE		2
#define CLKCTRL_MODE_DISABLE		0
#define CLKCTRL_STATUS_DISABLED		0x00030000

typedef struct am335x_registers {
    volatile uint32_t* control_module;
    
    volatile uint32_t* clock_manager;

    volatile uint32_t* gpio_0;
    volatile uint32_t* gpio_0_clear;
    volatile uint32_t* gpio_0_set;
    volatile uint32_t* gpio_0_oe;
    volatile uint32_t* gpio_1;
    volatile uint32_t* gpio_1_clear;
    volatile uint32_t* gpio_1_set;
    volatile uint32_t* gpio_1_oe;
    volatile uint32_t* gpio_2;
    volatile uint32_t* gpio_2_clear;
    volatile uint32_t* gpio_2_set;
    volatile uint32_t* gpio_2_oe;

    volatile uint16_t* pwm_tbcnt;
} am335x_registers_t;
am335x_registers_t am335x_registers;

#endif
