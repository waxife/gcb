/**
 *  @file   iccontrol.h
 *  @brief  head file for ic control function
 *  $Id: iccontrol.h,v 1.4 2012/12/12 07:57:58 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef	_ICCONTROL_H
#define _ICCONTROL_H

extern void WriteRegsTable(unsigned char code* RegsTable,unsigned char trig);
extern void ClosePanel(unsigned char uR, unsigned char uG, unsigned char uB);
extern void OpenPanel(void);
extern void check_shadow(void);

#endif	/* _ICCONTROL_H */
