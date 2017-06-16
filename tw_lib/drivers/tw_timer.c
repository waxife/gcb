/**
 *  @file   tw_timer.h
 *  @brief  terawins 10ms timer driver
 *  $Id: tw_timer.c,v 1.4 2013/04/02 05:42:18 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "tw_timer.h"
#include "tw_irq.h"

bit TW_TIMER_OUT_FLAG=0;
static unsigned long data TW_TIMER_OUT=0;
static unsigned long data TW_TIMER_CN=0;
unsigned char TP_TIME=0x00;
/*
 * 摘要 :tw_timer_start(void)
 *
 * 描述 :tw_timer 启动(中断开启)
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
static void tw_timer_start(void)
{
	enable_tw_irq(IRQ_10MS_TIME);
}
/*
 * 摘要 :tw_timer_stop(void)
 *
 * 描述 :tw_timer 关闭(中断关闭)
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
static void tw_timer_stop(void)
{
	disable_tw_irq(IRQ_10MS_TIME);
}
/*
 * 摘要 :void tw_timer_open(unsigned long count)
 *
 * 描述 :开启twtimer初始设定 并且设定中断时间 以10ms为单位 中断开启
 *
 * 参数 :unsigned long count(以10ms second设定时间)
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void tw_timer_open(unsigned long count)
{
	TW_TIMER_OUT=count;
	TW_TIMER_CN=0;
	TW_TIMER_OUT_FLAG=0;
	tw_timer_start();	
}
/*
 * 摘要 :tw_timer_close(void)
 *
 * 描述 :清除tw timer初始设定 并且关闭中断
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void tw_timer_close(void)
{
	TW_TIMER_OUT=0;
	TW_TIMER_CN=0;
	TW_TIMER_OUT_FLAG=0;
	tw_timer_stop();	
}
/*
 * 摘要 :tw_timer_isr(void)
 *
 * 描述 :tw timer 中断处理程序
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void tw_timer_isr(void)
{
	TW_TIMER_CN++;
	TP_TIME++;
	if(TW_TIMER_CN>=TW_TIMER_OUT)
	{
		TW_TIMER_CN=0;
		TW_TIMER_OUT_FLAG=1;
	}
	clear_tw_irq(IRQ_10MS_TIME);
}
