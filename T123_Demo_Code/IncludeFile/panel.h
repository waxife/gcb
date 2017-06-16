/**
 *  @file   panel.h
 *  @brief  head file for panel
 *  $Id: panel.h,v 1.4 2012/12/26 06:05:43 tony Exp $
 *  $Author: tony $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#define	PANEL_ID "HANSTAR 7' TTL888"

extern unsigned short code IML0_CVBS_TAB[13];
extern unsigned short code IML0_YUV_TAB[17];
extern unsigned short code IML0_VGA_TAB[7];
extern unsigned short code IML0_VIP_TAB[13];
extern unsigned char code ccir656_1080p_patch[];
extern void panel_init(void);
