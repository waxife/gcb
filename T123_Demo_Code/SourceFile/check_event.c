/**
 *  @file   check_event.c
 *  @brief  check event queue
 *  $Id: check_event.c,v 1.2 2012/12/04 03:11:46 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "config.h"
#include "sys.h"
#include "tw_widget_sys.h"
#include "serial.h"
#include "timer.h"
#include "tw_timer.h"
#include "check_event.h"
#include "videodetect.h"
#include "panel.h"
#include "sarkey.h"
#include "nvp6124.h"
#include "video.h"
//#include "mcu_if.h"

void check_event(void)
{
		TW_EVENT s_event;
		unsigned char rc=0x00;
		unsigned char ir=0x00;
		unsigned char sar=0x00;
		unsigned char uart=0x00;
		static unsigned char signal_active=0;
		static unsigned char signal_std=0;
		nvp6124_input_videofmt pvideofmt={0};

		rc=uartevent(&uart);
		if(rc!=0)
		{
			s_event.type=TW_EVENT_TYPE_UART;
			s_event.uart.ch=uart;
			put_event(&s_event);
			//DBG_PRINT("uart=%d\r\n",(int)uart);		
		}

		rc = SARscan(&sar);
		if(rc != 0)
		{
		    s_event.type = TW_EVENT_TYPE_KEY_DOWN;
			s_event.keystroke.ch = sar;
			put_event(&s_event);
		}

		if(TIMER0_OUT_FLAG)
		{
			TIMER0_OUT_FLAG=0;
			s_event.type = TW_EVENT_TYPE_TIMEOUT;
			s_event.timer.tid = 0;
			put_event(&s_event);
		}
		
		if(TIMER1_OUT_FLAG)
		{
			TIMER1_OUT_FLAG=0;
			s_event.type = TW_EVENT_TYPE_TIMEOUT;
			s_event.timer.tid = 1;
			put_event(&s_event);
		}

		if(TW_TIMER_OUT_FLAG)
		{
			TW_TIMER_OUT_FLAG=0;
			s_event.type = TW_EVENT_TYPE_TIMEOUT;
			s_event.timer.tid = 2;
			put_event(&s_event);
		}
		video_fmt_det(&pvideofmt);
		SingalDetect();
		if((sigstate.SigActive!=signal_active)||((sigstate.SigActive==Signal_Set)&&(sigstate.SigStd!=signal_std)))
		{
			s_event.type=TW_EVENT_TYPE_SINGNAL;
			signal_active=sigstate.SigActive;
			signal_std=sigstate.SigStd;
			s_event.signal.SigActive=sigstate.SigActive;
			s_event.signal.SigSrc=sigstate.SigSrc;
			s_event.signal.SigStd=sigstate.SigStd;
			put_event(&s_event);
			dbg(2, ("1.SIGNAL ACT=%bd	SRC=%bd	STD=%bd\r\n",s_event.signal.SigActive,s_event.signal.SigSrc,s_event.signal.SigStd));
		}
}
