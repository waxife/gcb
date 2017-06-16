/**
 *  @file   serial.c
 *  @brief  serial driver for hellen
 *  $Id: serial.c,v 1.10 2013/06/17 04:08:18 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.10 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include <reg51.h>
#include "sys.h"
#include "reg_tw.h"
#include "tw_irq.h"
#include "iccontrol.h"
#include "serial.h"
#include "config.h"

#ifndef UART_BUFF_SIZE
	#define UART_BUFF_SIZE	10
#endif

extern unsigned char xdata CONFIG_REGS[];

/* USE EXTERNAL UART */
static unsigned char xdata *UART_BASE = &CONFIG_REGS[0x400];
static unsigned char xdata *UART1_BASE = &CONFIG_REGS[0x500];

//#define UART_BUFF_SIZE	10
unsigned char uqueue[UART_BUFF_SIZE];
unsigned char ufront=UART_BUFF_SIZE-1; 
unsigned char urear=UART_BUFF_SIZE-1;

#ifdef I2C_DEBUG_MODE 
unsigned char i2c_count=0x00;
#endif

#define UART_RBR            (0x00)
#define UART_THR            (0x00)
#define UART_DLL            (0x00)
#define UART_DLH            (0x04)
#define UART_FCR         	(0x08)
#define UART_LCR            (0x0c)
#define UART_MCR         	(0x10)
#define UART_LSR            (0x14)
#define UART_SCR            (0x1c)
#define UART_IER            (0x04)
#define UART_SWR            (0x88)
#define MAX_FIFO			32
#define CONFIG_APB_CLK		27000000L

#ifdef  HW_UART_BAUD
#define UART_CLKDIV  		((CONFIG_APB_CLK+(HW_UART_BAUD*8))/(HW_UART_BAUD*16))
#else
#define UART_BAUD           38400L
#define UART_CLKDIV  		((CONFIG_APB_CLK+(UART_BAUD*8))/(UART_BAUD*16))
#endif

#ifdef  HW_UART1_BAUD
#define UART1_CLKDIV  		((CONFIG_APB_CLK+(HW_UART1_BAUD*8))/(HW_UART1_BAUD*16))
#else
#define UART1_BAUD           38400L
#define UART1_CLKDIV  		((CONFIG_APB_CLK+(UART1_BAUD*8))/(UART1_BAUD*16))
#endif
/* APB: 27Mhz, UART baud rate: 19200, UART_CLKDIV: 88 */


#ifdef FIFO
static unsigned char fifo[MAX_FIFO];
static unsigned char qf = 0;
static unsigned char qt = 0;
#endif
/*
 * 摘要 :void serial_init_tw(void)
 *
 * 描述 :uart 0 初始化
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void serial_init_tw(void)
{
	UART_BASE[UART_SWR] = 0x07;//software reset
	UART_BASE[UART_LCR] = 0x83;
	UART_BASE[UART_DLH] = (UART_CLKDIV >> 8);
	UART_BASE[UART_DLL] = (UART_CLKDIV & 0xff);
	UART_BASE[UART_LCR] = 3;	// N-8-1
	UART_BASE[UART_FCR] = 0xC1;
   	UART_BASE[UART_IER] = 1;
}
/*
 * 摘要 :void serial_interrupt(void)
 *
 * 描述 :uart 0 中断开启
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void serial_interrupt(void)
{
	UART_BASE[UART_FCR] = 0xC7;
	UART_BASE[UART_IER] = 1;

	enable_tw_irq(IRQ_UART0_INT);
}
/*
 * 摘要 :void serial1_init_tw(void)
 *
 * 描述 :uart 1 初始化
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
#if 1
void serial1_init_tw(void)
{
	UART1_BASE[UART_SWR] = 0x07;//software reset
	UART1_BASE[UART_LCR] = 0x83;
	UART1_BASE[UART_DLH] = (UART1_CLKDIV >> 8);
	UART1_BASE[UART_DLL] = (UART1_CLKDIV & 0xff);
	UART1_BASE[UART_LCR] = 3;	// N-8-1
#ifdef ENABLE_UART1_AFC	
	UART1_BASE[UART_MCR] = 0x20;
#endif	
	UART1_BASE[UART_FCR] = 0xC1;
	UART1_BASE[UART_IER] = 1;
}
/*
 * 摘要 :void serial1_interrupt(void)
 *
 * 描述 :uart 1 中断开启
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void serial1_interrupt(void)
{
	UART1_BASE[UART_FCR] = 0xC7;
	UART1_BASE[UART_IER] = 1;

	enable_tw_irq(IRQ_UART1_INT);
}

#endif

#if 0
char putc_t(char a0)
{	
	unsigned short k1;

	if(UART_BASE[UART_SCR] == 0)
		k1 = 200;
	else
		k1 = 6000;

	while(1)
	{
		k1--;
		if(k1 == 0)
			goto EXIT;

		if(UART_BASE[UART_LSR] & 0x40 == 0)
			continue;

		UART_BASE[UART_THR] = a0;
		return 0;
	}

EXIT:
	UART_BASE[UART_SCR] = 1;					// mark timeout
	UART_BASE[UART_THR] = a0;
	return 0;
}

char puts_t(char* s0)
{
	char* idx = s0;
	while( *(idx))
	{
		putc_t(*idx);
		idx++;
	}

	return 0;
}
#endif
/*
 * 摘要 :static void fill_fifo(void)
 *
 * 描述 :uart 使用fifo buffer
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
#ifdef FIFO
static void fill_fifo(void)
{
	unsigned char xdata q;
	q = qf+1;

	if(q>MAX_FIFO)
		q = 0;

	while(q != qt && UART_BASE[UART_LSR] & 0x01)
	{
		fifo[qf] = UART_BASE[UART_RBR];
		qf = q;

		q++;
		if(q >= MAX_FIFO)
			q = 0;
	}
}
#endif
/*
 * 摘要 :void putb2_t(const char c)
 *
 * 描述 :uart 0 输出一个byte
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void putb2_t(const char c)
{
	unsigned char i;
	//EA=0;
	for(i = 0; i < 255; i++)
	{
		if(UART_BASE[UART_LSR] & 0x40)
			break;
#ifdef FIFO
		fill_fifo();
#endif
	}

	UART_BASE[UART_THR] = c;
	//EA=1;
}
/*
 * 摘要 :void put1b_t(unsigned char c)
 *
 * 描述 :uart 1 输出一个byte
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void put1b_t(unsigned char c)
{
	unsigned char i;
	
	for(i = 0; i < 255; i++)
	{
		if(UART1_BASE[UART_LSR] & 0x40)
			break;
	}

	UART1_BASE[UART_THR] = c;

}
/*
 * 摘要 :void putb_t(const char c)
 *
 * 描述 :uart输出一个字元
 *
 * 参数 :const char c=>要输出的字元
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
#ifndef I2C_DEBUG_MODE
void putb_t(const char c)  
{
	if(c == '\n')
		putb2_t('\r');

	putb2_t(c);
}
#endif
/*
 * 摘要 :void putb_t(const char c)
 *
 * 描述 :i2c debug使用
 *
 * 参数 :const char c=>要输出的字元
 *
 * 回传值 :无
 *
 * 注意 : 须搭配额外软体
*/
#ifdef 	I2C_DEBUG_MODE
void putb_t(const char c)
{
	  if(i2c_count==0x00)
	  {
	  	 IC_WritByte(TWIC_P1,0xE0+i2c_count,0x80);
	  }
	  i2c_count++;
	  if(i2c_count>=16)
	  {
	  	
	  }
	  else if(c == '\n')
	  {
		   IC_WritByte(TWIC_P1,(0xE0+i2c_count),'\r');
		   IC_WritByte(TWIC_P1,(0xE0+i2c_count+1),'\n');
		   IC_WritByte(TWIC_P1,(0xE0+i2c_count+2),0);
	  }
	  else
	  {
	  	IC_WritByte(TWIC_P1,0xE0+i2c_count,c);
	  }
}
#endif
/*
 * 摘要 :char getb2_t(void)
 *
 * 描述 :轮询取得uart所传的资料
 *
 * 参数 :无
 *
 * 回传值 :-1 => 无资料 c=>uart所取得的字元
 *
 * 注意 : 无
*/
char getb2_t(void)
{
#ifdef FIFO
	char xdata c;
	fill_fifo();
	if(qt == qf)
		return -1;

	c = fifo[qt];
	qt++;
	if(qt > MAX_FIFO)
		qt = 0;

	return c;
#else
	if(UART_BASE[UART_LSR] & 0x01)
		return UART_BASE[UART_RBR];

	return -1;
#endif
}
/*
 * 摘要 :char getb_t(void)
 *
 * 描述 :轮询取得uart所传得资料
 *
 * 参数 :无
 *
 * 回传值 :回传uart得到的资料
 *
 * 注意 : 无
*/
char getb_t(void)
{
	char xdata c;
	while(1)
	{
		c = getb2_t();
		if(c >= 0)
			break;
	}
	return c;
}
/*
 * 摘要 :void getuart(void)
 *
 * 描述 :uart0 将uart中断的资料储存到 buffer内
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void getuart(void)
{
	unsigned char a=0x00;
	unsigned short count=0x00;		
	while(UART_BASE[0x14] & 0x01)
	{
		a=UART_BASE[0x00];
		urear=(urear+1)%UART_BUFF_SIZE;
		if(urear==ufront)
		{
			if(urear==0)
			{
				urear=UART_BUFF_SIZE-1;//max value
			}
			else
			{
				urear=urear-1;
			}
		}
		else
		{
			uqueue[urear]=a;
		}
		count++;
		if(count>0xff00)
			break;	
	}
	clear_tw_irq(IRQ_UART0_INT);
}
/*
 * 摘要 :unsigned char uartevent(unsigned char *value)
 *
 * 描述 :将uart buffer内的资料取出
 *
 * 参数 :unsigned char *value =>资料会给到这个指标
 *
 * 回传值 :0=>没资料 1=>有资料
 *
 * 注意 : 无
*/
unsigned char uartevent(unsigned char *value)
{
	unsigned char temp=0x00;
	if(ufront==urear)
	{
		return 0;
	}
	else
	{
		ufront=(ufront+1)%UART_BUFF_SIZE;
		temp=uqueue[ufront];
		*value=temp;
		return 1;
	}
}
/*
 * 摘要 :void getuart1(void)
 *
 * 描述 :uart1 将uart中断的资料储存到 buffer内
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void getuart1(void)
{
	unsigned char a=0x00;
	unsigned short count=0x00;		
	while(UART1_BASE[0x14] & 0x01)
	{
		a=UART1_BASE[0x00];
		urear=(urear+1)%UART_BUFF_SIZE;
		if(urear==ufront)
		{
			if(urear==0)
			{
				urear=UART_BUFF_SIZE-1;//max value
			}
			else
			{
				urear=urear-1;
			}
		}
		else
		{
			uqueue[urear]=a;
		}
		count++;
		if(count>0xff00)
			break;			
	}
	clear_tw_irq(IRQ_UART1_INT);
}

/******************************************************************************
*
* serial_init - initialize a channel
*
* set the selected baud rate and use timer1.
*
* RETURNS: N/A
*/

/*
 * 摘要 :void serial_init (void)
 *
 * 描述 :uart 0 and uart1 初始化
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void serial_init (void)
{
	/* USE EXTERNAL UART */
	// use UART0
	serial_init_tw();
	// use UART1
	serial1_init_tw();
	// select external UART
	IC_WritByte(TWIC_P1,0xFD,0x01);
	// enable all io pads
	IC_WritByte(TWIC_P0, PW_MGRCTRL_REG, (IC_ReadByte(TWIC_P0, PW_MGRCTRL_REG)|PWRDN_TOTALPAD));
	// select UART0 RS232RX/RS232TX pin function
	IC_WritByte(TWIC_P1, PIN_FUNCTION_REG, ((IC_ReadByte(TWIC_P1, PIN_FUNCTION_REG)&(~UART0_MASK))|UART0_EN));	
	// select UART1 RS232RX/RS232TX pin function
	//IC_WritByte(TWIC_P1, PIN_FUNCTION_REG, (IC_ReadByte(TWIC_P1,PIN_FUNCTION_REG)|UART1_EN));
#ifdef ENABLE_UART1_AFC	
	// select UART1 flow control RTS/CTS pin function
	IC_WritByte(TWIC_P1, PIN_FUNCTION_REG, ((IC_ReadByte(TWIC_P1,PIN_FUNCTION_REG)&(~RLUD_CPUINT_MASK))|RS232_AFC_EN));
#endif		
	IC_WritByte(TWIC_P0,0xE0,0x98); // enable all io pads
}
/*
 * 摘要 :void set_baudrate0(unsigned long baudrate)
 *
 * 描述 :uart 0 设定 baudrate 
 *
 * 参数 :unsigned long baudrate => 设定baudrate 的值
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void set_baudrate0(unsigned long baudrate)
{
	unsigned short uart_clock=0x00;
	uart_clock= ((CONFIG_APB_CLK+(baudrate*8))/(baudrate*16));
	UART_BASE[UART_LCR] = 0x83;
	UART_BASE[UART_DLH] = (uart_clock >> 8);
	UART_BASE[UART_DLL] = (uart_clock & 0xff);
	UART_BASE[UART_LCR] = 3;	// N-8-1
	UART_BASE[UART_FCR] = 0xC1;
   	UART_BASE[UART_IER] = 1;
}
/*
 * 摘要 :void set_baudrate1(unsigned long baudrate)
 *
 * 描述 :uart 1 设定 baudrate 
 *
 * 参数 :unsigned long baudrate => 设定baudrate 的值
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void set_baudrate1(unsigned long baudrate)
{
	unsigned short uart_clock=0x00;
	uart_clock= ((CONFIG_APB_CLK+(baudrate*8))/(baudrate*16));
	UART1_BASE[UART_LCR] = 0x83;
	UART1_BASE[UART_DLH] = (uart_clock >> 8);
	UART1_BASE[UART_DLL] = (uart_clock & 0xff);
	UART1_BASE[UART_LCR] = 3;	// N-8-1
	UART1_BASE[UART_FCR] = 0xC1;
   	UART1_BASE[UART_IER] = 1;
}
