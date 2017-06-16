/**
 *  @file   get_eq.c
 *  @brief  get event queue
 *  $Id: get_eq.c,v 1.4 2013/04/02 05:42:18 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "tw_widget_sys.h"

/* Event Queue */
#define EQ_SIZE	16
TW_EVENT event_queue[EQ_SIZE];
unsigned char event_queue_idx = 0;
unsigned char event_queue_cnt = 0;
/*
 * 摘要 :void put_event (TW_EVENT* event)
 *
 * 描述 :放置event到event buffer
 *
 * 参数 :TW_EVENT * event=>TW_EVENT结构指标
 *
 * 回传值 :回传TW_EVENT结构指标
 *
 * 注意 : 无
*/
void put_event (TW_EVENT* event)
{
	unsigned char idx;

	if (event_queue_cnt >= EQ_SIZE) {
		WARN (("event queue full; data lost!\n"));
		return;
	}

	idx = (event_queue_idx + event_queue_cnt) % EQ_SIZE;
	event_queue[idx] = *event;
	event_queue_cnt++;
	dbg(2, ("event:%bd, EQ<< cnt=%bd\n", event->type, (char)event_queue_cnt));
}
/*
 * 摘要 :TW_EVENT* get_event (void)
 *
 * 描述 :取出EVENT BUFFER内的TW_EVENT结构指标
 *
 * 参数 :无
 *
 * 回传值 :回传TW_EVENT结构指标
 *
 * 注意 : 无
*/
TW_EVENT* get_event (void)
{
	unsigned char id;

	if (event_queue_cnt == 0) {
		/* queue empty; no available data */
		event_queue[0].type = TW_EVENT_TYPE_NONE;
		return &event_queue[0];
	}

	id = event_queue_idx;
	event_queue_idx = (event_queue_idx + 1) % EQ_SIZE;
	event_queue_cnt--;
	dbg(2, ("event: %bd, EQ>> cnt=%bd\n", event_queue[id].type, (char)event_queue_cnt));

	return &event_queue[id];
}
/*
 * 摘要 :void clear_event(void)
 *
 * 描述 :清除EVENT BUFFER
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void clear_event(void)
{
	event_queue_cnt= 0;	
}
