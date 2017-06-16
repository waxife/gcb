/**
 *  @file   sarkey.h
 *  @brief  head file for SARKEY driver
 *  $Id: sarkey.h,v 1.2 2012/12/04 03:06:04 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _SARKEY_H
#define _SARKEY_H

#define SARKEY_BUFF_SIZE	10

extern void sarkey_init(void);
extern unsigned char SARscan(unsigned char *value);
extern void CheckSAR(void);

#endif	/* _SARKEY_H */
