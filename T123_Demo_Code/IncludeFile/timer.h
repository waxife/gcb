/**
 *  @file   timer.h
 *  @brief  head file for i8051 timer
 *  $Id: timer.h,v 1.2 2012/12/04 03:11:45 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _TIMER_H
#define _TIMER_H

extern bit TIMER0_OUT_FLAG;
extern bit TIMER1_OUT_FLAG;
extern unsigned long data TIMER0_CN;
extern unsigned long data TIMER1_CN;
extern unsigned long data TIMER0_OUT;
extern unsigned long data TIMER1_OUT;
extern unsigned short data TIMER0_COUNT;
extern unsigned short data TIMER1_COUNT;

extern void timer0_init(unsigned short timerData);
extern void tr0_open(unsigned long count);
extern void tr0_close(void);
extern void tr0_cut_start(void);
extern void tr0_cut_stop(void);
extern void timer1_init(unsigned short timerData);
extern void tr1_open(unsigned long count);
extern void tr1_close(void);
extern void tr1_cut_start(void);
extern void tr1_cut_stop(void);

#endif	/* _TIMER_H */
