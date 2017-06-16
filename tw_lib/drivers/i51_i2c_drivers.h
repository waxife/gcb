/**
 *  @file   i51_i2c_drivers.h
 *  @brief  head file for i51 i2c drivers
 *  $Id: i51_i2c_drivers.h,v 1.2 2012/12/04 03:06:04 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef	_I51_I2C_DRIVERS_H
#define _I51_I2C_DRIVERS_H

extern void iIC_WritByte(unsigned short bPage, unsigned char bAdd, unsigned char bData);
extern unsigned char iIC_ReadByte(unsigned short bPage, unsigned char bAdd);
extern void itwdDelay(unsigned short wLoops);

#endif	/* _I51_I2C_DRIVERS_H */
