/**
 *  @file   tw_init.c
 *  @brief  tw system init entry
 *  $Id: tw_init.c,v 1.11 2013/11/21 09:03:44 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.11 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "reg_tw.h"

#ifdef ENABLE_BANK_SWITCH
/* System Global Variable for Current Bank */
unsigned char data CURRENTBANK  _at_ 0x10;
#endif

#ifndef ENABLE_EX_MCU
unsigned long code LAYOUT_MAGIC  	_at_ LAYOUT_MAGIC_ADDR;
unsigned long code LAYOUT_TEXTSTART _at_ LAYOUT_TEXTSTART_ADDR;
unsigned long code LAYOUT_TEXTSIZE  _at_ LAYOUT_TEXTSIZE_ADDR;
unsigned long code LAYOUT_USERSTART _at_ LAYOUT_USERSTART_ADDR;
unsigned long code LAYOUT_USERSIZE  _at_ LAYOUT_USERSIZE_ADDR;
unsigned long code LAYOUT_RESSTART  _at_ LAYOUT_RESSTART_ADDR;
unsigned long code LAYOUT_RESSIZE  	_at_ LAYOUT_RESSIZE_ADDR;
unsigned short code LAYOUT_TP_LEFT  _at_ LAYOUT_TP_LEFT_ADDR;
unsigned short code LAYOUT_TP_RIGHT  _at_ LAYOUT_TP_RIGHT_ADDR;
unsigned short code LAYOUT_TP_UP  	_at_ LAYOUT_TP_UP_ADDR;
unsigned short code LAYOUT_TP_DOWN  _at_ LAYOUT_TP_DOWN_ADDR;
unsigned char code LAYOUT_TP_FINISH _at_ LAYOUT_TP_FINISH_ADDR;
unsigned short code LAYOUT_PATCH     _at_ LAYOUT_TP_FINISH_ADDR+1; 
#endif

void ACE_init(void)
{
	/* ACE_DDB_GAIN */
	IC_WritByte(TWIC_P3, 0x7E, 0x84);
 
	/* ACE_CTRL */ //enable ACE
	IC_WritByte(TWIC_P3, 0x7F, 0xE0);

	/* ACE_GAIN */
	IC_WritByte(TWIC_P3, 0x7C, 0x80);
}

void tw_init (void)
{
	/* power on BIAS  */
	IC_WritByte(TWIC_P0, 0xE3, 0x80);
    
	/* power on DPLL */
	IC_WritByte(TWIC_P0, 0xCA, IC_ReadByte(TWIC_P0, 0x0CA)| 0x20);
	
	/* power on Total Pad */
	IC_WritByte(TWIC_P0,0xE0,(IC_ReadByte(TWIC_P0,0xE0) | 0x80));

	/* disable shadow */
	IC_WritByte(TWIC_P0,0xE2, 0x00);
	
	/* turn on free run */
	IC_WritByte(TWIC_P0,0xC2,0x12);
	
	/* ORAM IO WaitState */
#ifndef ENABLE_EX_MCU
	IC_WritByte(TWIC_P1,0xFE,0xAD);
#else
	#ifdef ENABLE_RECIVER_CPUINT
	/* pin function select CPUINT */
	IC_WritByte(TWIC_P1,0xAC, (IC_ReadByte(TWIC_P1,0xAC) & 0xFC));
	#endif
	IC_WritByte(TWIC_P1,0xFE,0x2D);
#endif

	/* ACE reset  */
	IC_WritByte(TWIC_P3, 0x7D, (IC_ReadByte(TWIC_P3,0x7D) | 0x80));
	twdDelay(10);
	IC_WritByte(TWIC_P3, 0x7D, (IC_ReadByte(TWIC_P3,0x7D) & 0x7F));

}

