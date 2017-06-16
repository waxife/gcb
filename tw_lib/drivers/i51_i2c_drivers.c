/**
 *  @file   i51_i2c_drivers.h
 *  @brief  i51 i2c drivers
 *  $Id: i51_i2c_drivers.c,v 1.4 2013/04/02 09:46:59 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "i51_i2c_drivers.h"

#define BASEADDRESS 0x0000
unsigned char xdata CONFIG_REGS[0x700]  _at_ 0xF000;

/*
 * Synopsis    void iIC_WritByte( unsigned short bPage, 
 *                                unsigned char bAdd, 
 *                                unsigned char bData); 
 * Description  Œ‘Èëi2c
 * Parameters   bPage            - i2c page, ex:TWIC_P0
 *              bAdd             - •º´æÆ÷¾ŽÌ–
 *              bData            - ”µÖµ
 * Return       none
 */		  
void iIC_WritByte(unsigned short bPage, unsigned char bAdd, unsigned char bData)
{
	unsigned short Address = BASEADDRESS + bPage + bAdd;
	CONFIG_REGS[Address]=bData;
}

/*
 * Synopsis    unsigned char iIC_ReadByte( unsigned short bPage, 
 *                                         unsigned char bAdd);
 * Description  Œ‘Èëi2c
 * Parameters   bPage            - i2c page, ex:TWIC_P0
 *              bAdd             - •º´æÆ÷¾ŽÌ–
 * Return       •º´æÆ÷”µÖµ
 */	
unsigned char iIC_ReadByte(unsigned short bPage, unsigned char bAdd)
{
	unsigned short Address = BASEADDRESS + bPage + bAdd;	 	
	return CONFIG_REGS[Address];
}

/* Single-Cycle Instruction in 148ns (27MHz) */
/*
 * Synopsis    void itwdDelay(unsigned short wLoops);
 * Description ÑÓßt
 * Parameters  wLoops           - 1 count = 0.1ms 
 * Return      none
 */
void itwdDelay(unsigned short wLoops)  //one loop 0.1ms
{
	
	unsigned char tmp;
	while (wLoops--) {
	    tmp = 68;	// one loop below takes about 0.1ms
        while (tmp--) {};
    }

}
