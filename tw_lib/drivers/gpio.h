/**
 *  @file   gpio.h
 *  @brief  head file for GPIO driver
 *  $Id: gpio.h,v 1.3 2013/01/16 02:23:41 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.3 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _GPIO_H
#define _GPIO_H

#define INPUT  	0
#define OUTPUT 	1

#define ON     	1
#define OFF    	0

#define HIGH   	1
#define LOW    	0

// GPIO Register
#define GPIOA 	0x80
#define GPIOB 	0x88
#define GPIOC 	0x90
#define GPIOD 	0x98
#define GPIOE 	0x50

#define GPIO_OUTPUT_ENABLE 0x00
#define GPIO_OUTPUT_VALUE  0x01
#define GPIO_STATUS_VALUE  0x02
#define GPIO_INT_STATUS    0x03
#define GPIO_INT_RISING    0x04
#define GPIO_INT_FALLING   0x05
#define GPIO_OPEN_DRAIN    0x06

typedef struct
{
	unsigned char gpio_sel:1;//input 0 output 1
	unsigned char gpio_fall_edge:1;//on
	unsigned char gpio_rise_edge:1;//off 0 on 1
	unsigned char gpio_out_val:1;//low 0 high 1
	unsigned char gpio_pin :4; //select pin number 0~7
	unsigned char gpio_num :8;//Using the define value ex:GPIOA GPIOB
}GPIO;
/*  Example
	GPIO gpio_set;
	gpio_set.gpio_fall_edge=OFF;
	gpio_set.gpio_rise_edge=OFF;
	gpio_set.gpio_sel=OUTPUT;
	gpio_set.gpio_num=GPIOC;
	gpio_set.gpio_out_val=LOW;
	setgpio(gpio_set,num);
*/
extern void setgpio(GPIO gpio_set,unsigned char num);
extern bit gpio_status(unsigned char gpio_sel,unsigned char num);
extern void gpio_isr(void);

#endif	/* _GPIO_H */
