/**
 *  @file   vivi.h
 *  @brief  vivi color setup header
 *  $Id: vivi.h,v 1.2 2012/12/04 03:06:04 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _VIVI_H_
#define _VIVI_H_ 

extern unsigned char code vivi_color_tbl[];
extern void vivi_init_fast(void);
extern void vivi_init_slow(void);
extern void vivi_enable(void);
extern void vivi_disable(void);
extern void ace_enable(void);
extern void ace_disable(void);
void vivi_init_dma(void);
//void vivi_init(void);

#endif	/* _VIHI_H_ */