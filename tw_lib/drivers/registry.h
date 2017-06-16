/**
 *  @file   registry.h
 *  @brief  head file for registry
 *  $Id: registry.h,v 1.2 2012/12/04 03:06:04 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _REGISTRY_H
#define _REGISTRY_H
typedef struct UserRegistry
{
/* video*/
	unsigned char vASigStd;
	unsigned char vDSigStd;
	unsigned char Pb;
	unsigned char Pr;
	unsigned char YPbPrBalance;	
/*touch panel*/
	unsigned short left;
	unsigned short right;
	unsigned short up;
	unsigned short down;
	unsigned char calibration_finish;	
}RUserData;

extern RUserData UserData;

extern int registry_read(unsigned long registry, int size);
extern int registry_write(unsigned long registry, int size);
extern void registry_test(void);

#endif	/* _REGISTRY_H */
