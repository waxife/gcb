/**
 *  @file   interrupt.c
 *  @brief  i8051 interrupt entry
 *  $Id: interrupt.c,v 1.2 2012/12/04 03:11:46 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include <reg51.h>
#include "sys.h"
#include "ir.h"
#include "serial.h"
#include "sarkey.h"
#include "gpio.h"
#include "timer.h"
#include "cq.h"
#include "tw_timer.h"
#include "tw_irq.h"

#define ENABLE		1
#define DISABLE		0

void TIMER0_INT(void) interrupt 1
{
	TR0=DISABLE;	// disable Timer0 INT	

	TL0=TIMER0_COUNT&0x00FF;
	TH0=TIMER0_COUNT>>8;

	TIMER0_CN++;
	if(TIMER0_CN>=TIMER0_OUT)
	{
		TIMER0_CN=0;
		TIMER0_OUT_FLAG=1;
	}

#if 0
	if(SarRepeat)
	{
		sarcount++;
		if(sarcount==255)
		{
			CheckSAR();
			sarcount=0;
		}
	}
#endif

	TR0=ENABLE;		// enable Timer0 INT
}

void TIMER1_INT(void) interrupt 3
{	
	TR1=DISABLE;	// disable Timer1 INT	

	TL1=TIMER1_COUNT&0x00FF;
	TH1=TIMER1_COUNT>>8;

	TIMER1_CN++;
	if(TIMER1_CN>=TIMER1_OUT)
	{
		TIMER1_CN=0;
		TIMER1_OUT_FLAG=1;
	}

	TR1=ENABLE;		// enable Timer1 INT		
}

void EXTERNAL_INT(void) interrupt 0
{
	unsigned char data irq_nr=0;
	unsigned short data irq_flag;
		
	/* Need check this function if use command queue */
	if (cq_busy ())
		return;

	/* Disable External Interrupt */
	EX0 = DISABLE;

	irq_flag = tw_irq_dispatch();
	
	//INFO(("EXTERNAL INTERRUPT STATUS: 0x%X\n", irq_flag));

	for(irq_nr=0; irq_nr<IRQ_TOTAL_NUM; irq_nr++)
	{
		if(!(irq_flag&(1<<irq_nr)))
			continue;		
		
		switch(irq_nr) {
			case IRQ_VSYNC_MISS:
				break;
			
			case IRQ_HSYNC_MISS:
				break;
			
			case IRQ_VSYNC_CHG:
				break;
			
			case IRQ_HSYNC_CHG:
				break;
			
			case IRQ_10MS_TIME:
				tw_timer_isr();
				break;
			
			case IRQ_SPI_DMA:
				break;
			
			case IRQ_IR_PACKET:
				//CheckIR();
				break;
			
			case IRQ_SAR0_TOGGLE:	
				
				break;
			
			case IRQ_SAR1_TOGGLE:
				CheckSAR();
				break;
			
			case IRQ_SHORT_VS:
				break;
			
			case IRQ_SHDN_WAR:
				break;
			
			case IRQ_UART0_INT:
				getuart();
				break;
			
			case IRQ_UART1_INT:
				//uart1_recv();
				//getuart1();
				break;
			
			case IRQ_GPIO_INT:
				break;
			
			case IRQ_CQ_INT:
				cq_isr ();
				break;
		}
	}
	
	/* Enable External Interrupt */
	EX0 = ENABLE;
}
