/**
 *  @file   tw_timer.h
 *  @brief  head file for tw timer
 *  $Id: tw_timer.h,v 1.2 2012/12/04 03:06:04 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _TW_TIMER_H
#define _TW_TIMER_H

extern bit TW_TIMER_OUT_FLAG;

extern void tw_timer_open(unsigned long count);
extern void tw_timer_close(void);
extern void tw_timer_isr(void);
extern unsigned char TP_TIME;

#endif /* _TW_TIMER_H */
