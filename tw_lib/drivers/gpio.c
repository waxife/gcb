/*
 *  @file   gpio.c
 *  @brief  GPIO driver 
 *  $Id: gpio.c,v 1.6 2013/08/01 02:55:38 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.6 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/06/11  yc New file.
 *
 */

#include "sys.h"
#include "iccontrol.h"
#include "reg_tw.h"
#include "tw_irq.h"
#include "gpio.h"
/*
 * 摘要 :void setgpio(GPIO gpio_set,unsigned char num)
 *
 * 描述 :设定GPIO
 *
 * 参数 :GPIO gpio_set =>根据GPIO STRUCTURE 设定
 *       unsigned char num =>GPIO 0!7
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void setgpio(GPIO gpio_set,unsigned char num)
{
	if(gpio_set.gpio_sel==OUTPUT)
	{
		IC_WritByte(TWIC_P1,gpio_set.gpio_num+GPIO_OUTPUT_ENABLE,IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_OUTPUT_ENABLE)|(1<<num));//set OUTPUT
		IC_WritByte(TWIC_P1,gpio_set.gpio_num+GPIO_OUTPUT_VALUE,gpio_set.gpio_out_val==1?(IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_OUTPUT_VALUE)|1<<num):(IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_OUTPUT_VALUE)&~(1<<num)));//set OUTPUT value
		IC_WritByte(TWIC_P1,gpio_set.gpio_num+GPIO_OPEN_DRAIN,IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_OPEN_DRAIN)&~(1<<num));//set_open drain
	}
	else
	{
		IC_WritByte(TWIC_P1,gpio_set.gpio_num+GPIO_OUTPUT_ENABLE,IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_OUTPUT_ENABLE)&~(1<<num));//set INPUT
		IC_WritByte(TWIC_P1,gpio_set.gpio_num+GPIO_INT_RISING,gpio_set.gpio_rise_edge==1?(IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_INT_RISING)|(1<<num)):(IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_INT_RISING)&~(1<<num)));//set rise_edge value
		IC_WritByte(TWIC_P1,gpio_set.gpio_num+GPIO_INT_FALLING,gpio_set.gpio_fall_edge==1?(IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_INT_FALLING)|1<<num):(IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_INT_FALLING)&~(1<<num)));//set fall_edge value
		IC_WritByte(TWIC_P1,gpio_set.gpio_num+GPIO_OPEN_DRAIN,IC_ReadByte(TWIC_P1,gpio_set.gpio_num+GPIO_OPEN_DRAIN)|(1<<num));//set_open drain
	}
}
/*
 * 摘要 :bit gpio_status(unsigned char gpio_num,unsigned char num)
 *
 * 描述 :取得GPIO STATUS
 *
 * 参数 :unsigned char gpio_num => GPIOA=0x80 GPIOB=0x88 GPIOC=0x90 GPIOD=0x98 GPIOE=0x50
 *       unsigned char num=> 选取GPIO 0~7
 *
 * 回传值 : 0=>low 1=>high
 *
 * 注意 : 无
*/
bit gpio_status(unsigned char gpio_num,unsigned char num)//1.select GPIOA=1 b=2 c=3 d=4 e=5 f 2.select gpio 0~7
{
	bit value;
	value=(IC_ReadByte(TWIC_P1,gpio_num+GPIO_STATUS_VALUE)&(1<<num))!=0?1:0;
	return value;
}
/*
 * 摘要 :void gpio_isr(void)
 *
 * 描述 :gpio处理程序
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 暂无开放
*/
void gpio_isr(void)
{
	IC_WritByte(TWIC_P1,0x83,0xFF);//CLEAR GPIOA INTERRUPT
	
	clear_tw_irq(IRQ_GPIO_INT);	//CLEAR GPIO INTERRUPT
}