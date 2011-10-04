#ifndef SC2410_H
#define SC2410_H

#define REG (volatile uint32_t *)

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

#define CS_ALL 0x1C
#define CS_CLK 0x2
#define CS_SID 0x4

// GPIO
#define GPACON 0x56000000
#define GPBCON 0x56000010
#define GPCCON 0x56000020
#define GPECON 0x56000040
#define GPGCON 0x56000060

#define GPCDAT 0x56000024
#define GPEDAT 0x56000044
#define GPGDAT 0x56000064

// PWM
#define  TCFG0 0x51000000
#define   TCON 0x51000008
#define TCNTB0 0x5100000C
#define TCMPB0 0x51000010

#endif

