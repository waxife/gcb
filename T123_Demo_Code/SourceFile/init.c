/**
 *  @file   init.c
 *  @brief  system init entry
 *  $Id: init.c,v 1.8 2013/08/01 03:35:58 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.8 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include <reg51.h>
#include "config.h"
#include "sys.h"
#include "ir.h"
#include "sarkey.h"
#include "serial.h"
#include "gpio.h"
#include "timer.h"
#include "spirw.h"
#include "cq.h"
#include "tw_irq.h"
#include "osd2api.h"
#include "videodetect.h"
#include "panel.h"
#include "gpio.h"

extern const char code ver[];

#if 0
char data GLOBAL_DATA_S_BYTE_BUFFER[4]={0};
unsigned char data GLOBAL_DATA_BYTE_BUFFER[16]={0};
unsigned short data GLOBAL_DATA_WORD_BUFFER[8]={0};
unsigned long data GLOBAL_DATA_DWORD_BUFFER[6]={0};
#endif

static void interrupt_init(void)
{
	EA 		= ALL_INTERRUPT;
	EX0 	= EXTERNAL0_INTERRUPT;
	ES		= SERIAL_INTERRUPT;
	ET0		= TIMER0_INTERRUPT;
	ET1		= TIMER1_INTERRUPT;		
}

void init(void)
{	
	/* driver and system init */
	
	tw_init();

	enable_quad_io();

	init_IRQ();

	interrupt_init();

	timer0_init(1);

	timer1_init(1);
	
	serial_init();
	serial_interrupt();
	//serial1_interrupt();

	//ir_init();

	sarkey_init();
	
	cq_init();

	InitStateVal();

	auto_detection_init(&IML0_CVBS_TAB,&IML0_YUV_TAB,&IML0_VGA_TAB);

	VideoSet(VGA_UNKNOW,CVBS_UNKNOW,YUV_UNKNOW,0,0,0);
	//VideoSet(CVBS_UNKNOW,YUV_UNKNOW,0,0,0);

	//vivi_init_slow();

	/* logo */
	DBG_PRINT("\n\nWelcome to Helen %s\n", HELEN_VER);
  DBG_PRINT("\rCopyright 2012 Terawins Inc.\n");
  DBG_PRINT("\rRelease Version V%bu.%bu\n",ver[0],ver[1]);
  DBG_PRINT("\r\n");
}



