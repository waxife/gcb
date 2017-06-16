/**
 *  @file   tw_offset.h
 *  @brief  head file for tw_offset
 *  $Id: tw_offset_table.h,v 1.1 2013/06/07 07:16:21 joec Exp $
 *  $Author: joec $
 *  $Revision: 1.1 $
 *
 *  Copyright (c) 2013 Terawins Inc. All rights reserved.
 *
 *  @date   2013/05/08  ken     New file.
 *
 */

#ifndef _TW_OFFSET_H
#define _TW_OFFSET_H

/**
 * TW OSD Position Offset Structure (for diff signal)
 */
typedef struct {
  unsigned char     signal;         /* [7:4] source | [3:0] standard */
  unsigned short    img_hoffset;    /* image/twbc horizontal offset */
  unsigned short    img_voffset;    /* image/twbc vertical offset */
  unsigned short    sp_hoffset;     /* sprite horizontal offset */
  unsigned short    sp_voffset;     /* sprite vertical offset */
  unsigned short    osd2_ratth;     /* horizontal start position of the menu rows */
  unsigned short    osd2_rattv;     /* vertical start position of the menu rows */  
} TW_OSD_OFFSET, *PTW_OSD_OFFSET;

/**
 * TW OSD Position Offset Tables
 */
typedef struct {
  unsigned char    nelements;
  TW_OSD_OFFSET*     desc;        /* sizeof (desc[0]) == 13 */  
} TW_OSD_OFFSET_TBL, *PTW_OSD_OFFSET_TBL;

extern void tw_osd_offset_init(void);
extern void tw_offset_adj(unsigned char src, unsigned char std, unsigned short address, unsigned char width, unsigned char height);

#endif	/* _TW_OFFSET_H */
