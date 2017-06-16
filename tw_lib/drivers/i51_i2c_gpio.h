/**
 *  @file   i51_i2c_gpio.c
 *  @brief  terawins I2C control function
 *  $Id: i51_i2c_gpio.h,v 1.2 2012/12/07 01:31:22 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/11/29  onejoe 	New file.
 *
 */


#ifndef	_I51_I2C_GPIO_H
#define _I51_I2C_GPIO_H

#include "i51_i2c_drivers.h"
extern unsigned char xdata CONFIG_REGS[0x700];

#define _iICReg 0x180 //0xf1

#define _oSCL	0x40 //1000 0000
#define _iSCL	0x40 //0100 0000
#define _oSDA	0x80 //0010 0000
#define _iSDA	0x80 //0001 0000

#define _ACK	0x00
#define _nACK	0x01

#define _iICoe_od  (CONFIG_REGS[_iICReg]|=0x80)
#define _iICoe_id  (CONFIG_REGS[_iICReg]&=0x7F)
#define _iICrd		CONFIG_REGS[_iICReg+1]
#define _iICget		CONFIG_REGS[_iICReg+2]
#define	_iICwr(val)	(CONFIG_REGS[_iICReg+1]=val)

#define	_rdSDA		((_iICget&_iSDA)>>7)
#define _rdSCL		((_iICget&_iSCL)>>6)

#define _wrSDA_H	_iICwr(_oSDA|_iICrd)					
#define _wrSDA_L	_iICwr(~_oSDA&_iICrd)

#define _wrSCL_H	_iICwr(_oSCL|_iICrd)
#define _wrSCL_L	_iICwr(~_oSCL&_iICrd)

#define _SDA_H		_rdSDA!=0
#define _SDA_L		_rdSDA==0
#define _SCL_H		_rdSCL!=0
#define _SCL_L		_rdSCL==0

#define	_iIC_SPEED	0x00		//value:1 (Fastest), 2 (slower), ... N (much slower)

#define	_MAX_WAIT	0x0	

#ifdef _iIC_SPEED
	#define _Delay(c) for(c = 0; c < _iIC_SPEED; c++){}
#else
	#define _Delay(c)
#endif	



void io_iIC_start();
void io_iIC_stop();

unsigned char io_iIC_RdByte(unsigned char ack);
unsigned char io_iIC_wrByte(unsigned char wrData);

unsigned char io_iIC_wrQueue(unsigned char cDevAddr, unsigned char *arrData, unsigned char cNum);
unsigned char io_iIC_rdQueue(unsigned char cDevAddr, unsigned char *arrData, unsigned char cNum);

unsigned char io_iIC_wrReg(unsigned char cDevAddr, unsigned char cReg, 
						   unsigned char *arrData, unsigned char cNum);

unsigned char io_iIC_rdReg(unsigned char cDevAddr, unsigned char cReg, 
						   unsigned char *arrData, unsigned char cNum);


#endif // _I51_I2C_GPIO_H