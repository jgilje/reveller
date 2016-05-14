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

#define BCM2835_BASE 		0x20000000
#define BCM2836_BASE 		0x3F000000
#define GPIO_OFFSET		0x200000
#define PWM_OFFSET		0x20C000
#define CLOCK_OFFSET		0x101000

#define GPIO_OUTPUT 0x1

typedef struct bcm2835_registers {
    volatile uint32_t* gpio_base;
    volatile uint32_t* gpio_fsel0;
    volatile uint32_t* gpio_fsel1;
    volatile uint32_t* gpio_fsel2;
    volatile uint32_t* gpio_output_set0;
    volatile uint32_t* gpio_output_set1;
    volatile uint32_t* gpio_output_clear0;
    volatile uint32_t* gpio_output_clear1;
    volatile uint32_t* gpio_level0;

    volatile uint32_t* pwm_base;
    volatile uint32_t* pwm_rng1;
    volatile uint32_t* pwm_dat1;

    volatile uint32_t* clock_base;
    volatile uint32_t* clock_pwm_cntl;
    volatile uint32_t* clock_pwm_div;
} bcm2835_registers_t;
bcm2835_registers_t bcm2835_registers;

#endif
