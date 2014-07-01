/*
6 Enable	GPC4 (VM)
5 R/_W		GPC12
4 RS		GPC13

11 data0	GPC8
12 data1	GPC9
13 data2	GPC10
14 data3	GPC11
*/

#include <stdio.h>

#include "lcd-hammer.h"
#include "sc2410.h"

#include "platform-support.h"

uint32_t lcd_e_delay_t = 200; // 400
uint32_t lcd_r_delay_t = 500; // 500

void lcd_e_delay(void) {
    platform_usleep(lcd_e_delay_t);
}

void lcd_delay(uint32_t t) {
    platform_usleep(lcd_r_delay_t);
}

void lcd_write_raw(uint32_t data) {
	uint32_t gpc_data = (data << DATA_BUS_SHIFT) | CS_ALL;
	// printf("lcd_write_raw(0x%x) => 0x%x\n", data, gpc_data);
	
	*(REG s3c2410_registers.v_gpio_c_data) = gpc_data;
	lcd_e_delay();
	*(REG s3c2410_registers.v_gpio_c_data) = gpc_data | CS_LCD;
	lcd_e_delay();
	*(REG s3c2410_registers.v_gpio_c_data) = gpc_data;
	lcd_e_delay();
	*(REG s3c2410_registers.v_gpio_c_data) = CS_ALL;
	lcd_e_delay();
}

static void lcd_write(uint8_t data, uint8_t rs) {
    uint32_t gpc_data = 0;
    uint32_t rs_reg = 0;
    
    if (rs) {   /* write data        (RS=1, RW=0) */
       rs_reg = 0x2000;		// GPC13
    } else {    /* write instruction (RS=0, RW=0) */
       rs_reg = 0x0;
    }
    // lcd_rw_low(); always low...

	/*
	printf("lcd_write(0x%x, %d) => 0x%x && 0x%x\n", data, rs, 
			((data >> 4) << DATA_BUS_SHIFT) | CS_ALL | CS_LCD | rs_reg,
			((data & 0xf) << DATA_BUS_SHIFT) | CS_ALL | CS_LCD | rs_reg
	);
	*/
	
    /* output high nibble first */
    gpc_data = ((data >> 4) << DATA_BUS_SHIFT) | CS_ALL | rs_reg;
    //*(REG s3c2410_registers.v_gpio_c_data) = gpc_data;
    //lcd_e_delay();
    *(REG s3c2410_registers.v_gpio_c_data) = gpc_data | CS_LCD;
    lcd_e_delay();
    *(REG s3c2410_registers.v_gpio_c_data) = gpc_data;
    lcd_e_delay();
    if (! rs) {
        platform_usleep(5000);
    }
    /*
    *(REG s3c2410_registers.v_gpio_c_data) = CS_ALL;
    lcd_e_delay();
    */
    
    /* output low nibble */
    gpc_data = ((data & 0xf) << DATA_BUS_SHIFT) | CS_ALL | rs_reg;
    //*(REG s3c2410_registers.v_gpio_c_data) = gpc_data;
    //lcd_e_delay();
    *(REG s3c2410_registers.v_gpio_c_data) = gpc_data | CS_LCD;
    lcd_e_delay();
    *(REG s3c2410_registers.v_gpio_c_data) = gpc_data;
    lcd_e_delay();
    // lcd_e_delay();
    if (! rs) {
        platform_usleep(5000);
    }
    
    *(REG s3c2410_registers.v_gpio_c_data) = CS_ALL;
}

void lcd_command(uint8_t cmd) {
    lcd_write(cmd, 0);
}

void lcd_gotoxy(uint8_t x, uint8_t y) {
    if ( y==0 ) {
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    } else if ( y==1) {
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
    } else if ( y==2) {
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE3+x);
    } else {
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE4+x);
    }
}

void lcd_putc(char c) {
    lcd_write(c, 1);
}

void lcd_puts(const char *s) {
    char c;

    while ((c = *s++)) {
        lcd_putc(c);
    }
}

void lcd_reinit(uint32_t e_delay_t, uint32_t r_delay_t) {
	lcd_e_delay_t = e_delay_t;
	lcd_r_delay_t = r_delay_t;
	lcd_init();
}

void lcd_clear(void) {
    lcd_command(LCD_DISP_CLEAR);
}

void lcd_print_lines(char** line_v, int lines) {
	int i;
	
	lcd_clear();
	for (i = 0; i < lines; i++) {
		char line[21];
		snprintf(line, 21, "%s", line_v[i]);
		lcd_gotoxy(0, i);
		lcd_puts(line);
		// printf("%s\n", line);
	}
}

void lcd_init(void) {
    /*
     *  Initialize LCD to 4 bit I/O mode
     */
    lcd_delay(16000);        /* wait 16ms or more after power-on       */
    
    /* initial write to lcd is 8bit */
    lcd_write_raw(0x3);
    lcd_delay(15000);		/* min. 5 ms. */
   
    /* repeat last command */ 
    lcd_write_raw(0x3);
    lcd_delay(1600);		/* min. 160 us. */
    
    /* repeat last command a third time */
    lcd_write_raw(0x3);
    lcd_delay(1600);		/* min. 160 us. */
    
    /* now configure for 4bit mode */
    lcd_write_raw(0x2);
    lcd_delay(1600);           /* some displays need this additional delay */
    
    /* from now the LCD only accepts 4 bit I/O, we can use lcd_command() */    
    lcd_command(LCD_FUNCTION_DEFAULT);      /* function set: display lines  */
    lcd_command(LCD_DISP_OFF);              /* display off                  */
    lcd_clear();
    lcd_command(LCD_ENTRY_INC);				/* entry mode - entry incr.		*/
    lcd_command(LCD_DISP_ON);  				/* display/cursor control       */
    lcd_command(LCD_MOVE_CURSOR_HOME);
    
    /*
    lcd_gotoxy(0, 0);  
	lcd_puts("Hello, SID!");
	*/
	/*
	char* l0 = "Hello, SID!Hello, SID!Hello, SID!Hello, SID!Hello, SID!";
	char* l[4]; l[0] = l0; l[1] = l0; l[2] = l0; l[3] = l0;
	lcd_print_lines(l, 4);
	*/
}


