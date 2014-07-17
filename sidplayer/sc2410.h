#ifndef SC2410_H
#define SC2410_H

#define REG (volatile uint32_t *)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define CS_ALL 0xC	// CS_LCD is active HIGH
#define CS_CLK 0x2
#define CS_SID 0x4
#define CS_BTN 0x8
#define CS_LCD 0x10

// GPIO
#define GPACON 0x56000000
#define GPBCON 0x56000010
#define GPCCON 0x56000020
#define GPECON 0x56000040
#define GPGCON 0x56000060

#define GPCDAT 0x56000024
#define GPEDAT 0x56000044
#define GPGDAT 0x56000064

#define GPCPULLUP 0x56000028

// PWM
#define  TCFG0 0x51000000
#define  TCFG1 0x51000004 
#define   TCON 0x51000008
#define TCNTB0 0x5100000C
#define TCMPB0 0x51000010
#define TCNTB1 0x51000018

#define DATA_BUS_SHIFT 8

typedef struct s3c2410_registers_t {
	void* v_gpio_base;
	
	void* v_gpio_b_conf;
	void* v_gpio_c_conf;
	void* v_gpio_e_conf;
	void* v_gpio_g_conf;
	
	void* v_gpio_c_data;
	void* v_gpio_e_data;
	void* v_gpio_g_data;
	
	void* v_gpio_c_pullup;
	
	void* t_base;
	void* v_tcfg1;
	void* v_tcon;
	void* v_tcntb0;
	void* v_tcmpb0;
	void* v_tcntb1;
} s3c2410_registers_t;
extern s3c2410_registers_t s3c2410_registers;

#endif

