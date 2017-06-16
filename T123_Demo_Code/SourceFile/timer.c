/**
 *  @file   timer.c
 *  @brief  i8051 timer handling
 *  $Id: timer.c,v 1.2 2012/12/04 03:11:46 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include <reg51.h>
#include "timer.h"

#define ENABLE		1
#define DISABLE		0

bit TIMER0_OUT_FLAG=0;
bit TIMER1_OUT_FLAG=0;
unsigned long data TIMER0_OUT=0;
unsigned long data TIMER1_OUT=0;
unsigned long data TIMER0_CN=0;
unsigned long data TIMER1_CN=0;
unsigned short data TIMER0_COUNT=0;
unsigned short data TIMER1_COUNT=0;

/* timer0 initial */
void timer0_init(unsigned short timerData)
{
	TIMER0_COUNT = timerData;
	TL0=TIMER0_COUNT&0x00FF;
	TH0=TIMER0_COUNT>>8;
}

/* timer0 open */
void tr0_open(unsigned long count)
{
	TIMER0_OUT=count;
	TIMER0_CN=0;
	TIMER0_OUT_FLAG=0;
	tr0_cut_start();	
}

/* timer0 close */
void tr0_close(void)
{
	TIMER0_OUT=0;
	TIMER0_CN=0;
	TIMER0_OUT_FLAG=0;
	tr0_cut_stop();	
}

/* timer0 count start */
void tr0_cut_start(void)
{
	TR0=ENABLE;
}

/* timer0 count stop */
void tr0_cut_stop(void)
{
	TR0=DISABLE;
}

/* timer1 initial */
void timer1_init(unsigned short timerData)
{
	TIMER1_COUNT = timerData;
	TL1=TIMER1_COUNT&0x00FF;
	TH1=TIMER1_COUNT>>8;
}

/* timer1 open */
void tr1_open(unsigned long count)
{
	TIMER1_OUT=count;
	TIMER1_CN=0;
	TIMER1_OUT_FLAG=0;
	tr1_cut_start();	
}

/* timer1 close */
void tr1_close(void)
{
	TIMER1_OUT=0;
	TIMER1_CN=0;
	TIMER1_OUT_FLAG=0;
	tr1_cut_stop();	
}

/* timer1 count start */
void tr1_cut_start(void)
{
	TR1=ENABLE;
}

/* timer1 count stop */
void tr1_cut_stop(void)
{
	TR1=DISABLE;
}
