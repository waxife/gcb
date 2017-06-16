/**
 *  @file   i2cm.h
 *  @brief  head file for i2cm.c
 *  $Id: i51_i2c_master.h,v 1.3 2012/12/10 06:26:19 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.3 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

 extern unsigned char i2cm_init(void);
 extern unsigned char i2cm_wr_byte (unsigned char slave, unsigned char addr, unsigned char datas);
 extern unsigned char i2cm_rd_byte (unsigned char slave, unsigned char addr, unsigned char *datas);
 extern unsigned char i2cm_wr_word (unsigned char slave, unsigned char addr, unsigned short datas);
 extern unsigned char i2cm_rd_word (unsigned char slave, unsigned char addr, unsigned short *datas);
// extern void MI2CWriteRegsTable(unsigned char code* RegsTable);