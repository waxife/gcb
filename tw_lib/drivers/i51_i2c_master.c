/**
 *  @file   i2cm.c
 *  @brief  terawins I2C Master control function
 *  $Id: i51_i2c_master.c,v 1.5 2013/04/03 08:49:22 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.5 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

 /*

protocol:choose i2c master communication mode 

1.QUICK

|S|ADDR|R/W|A|P|

2.SINGLE

|S|ADDR|W|rA|wDATA0|rA|P|

|S|ADDR|R|rA|rDATA0|~wA|P|

3.BYTE

|S|ADDR|W|rA|wDATA0|rA|wDATA1|rA|P|

|S|ADDR|W|rA|wDATA0|rA|S|ADDR|R|rA|rDATA0|~wA|P|

4.WORD

|S|ADDR|W|rA|wDATA0|rA|wDATA|rA|wDATA1|rA|P|

|S|ADDR|W|rA|wDATA0|rA|S|ADDR|R|rA|rDATA0|wA|rDATA1|~wA|P|

5.GENERAL CALL

|S|ADDR|W|rA|wDATA1|rA|wDATA2|rA|S|ADDR|R|rA|rDATA0|wA|rDATA1|~wA|P|

setting: setting reg val

ADDR	_I2CM_SET_ADDR(slave,S_READ/S_WRITE);

wDATA0	_I2CM_SET_CMD(val)
wDATA1	_I2CM_SET_DATA0(val)
wDATA2	_I2CM_SET_DATA1(val)

rDATA0	_I2CM_GET_DATA0()
rDATA1	_I2CM_GET_DATA1()

start comm

                                              I2CM_QUICK
                                              I2CM_SINGLE
_I2CM_SET_INST((_I2CM_GET_INST()| START_INTR |I2CM_BYTE         ));
                                              I2CM_WORD
                                              I2CM_GENERAL_CALL
												

 */

#include "sys.h"
#include "reg_tw.h"

#include "i51_i2c_drivers.h"
#include "i51_i2c_master.h"

#define	_I2CM_SET_STATUS(val)	IC_WritByte(TWIC_P2, I2CM_STATUS, val)
#define	_I2CM_SET_MASK(val)		IC_WritByte(TWIC_P2, I2CM_MASK, val)
#define	_I2CM_SET_INST(val)		IC_WritByte(TWIC_P2, I2CM_INSTRUCT, val)
 
#define	_I2CM_SET_ADDR(slave,rw)	IC_WritByte(TWIC_P2, I2CM_SLAVE_ADDR, slave|rw)
#define	_I2CM_SET_CMD(val)		IC_WritByte(TWIC_P2, I2CM_COMMAND, val)
#define	_I2CM_SET_DATA0(val)	IC_WritByte(TWIC_P2, I2CM_DATA0, val)
#define	_I2CM_SET_DATA1(val)	IC_WritByte(TWIC_P2, I2CM_DATA1, val)

#define	_I2CM_GET_STATUS()		IC_ReadByte(TWIC_P2, I2CM_STATUS)
#define	_I2CM_GET_INST()		IC_ReadByte(TWIC_P2, I2CM_INSTRUCT)	
#define	_I2CM_GET_DATA0()		IC_ReadByte(TWIC_P2, I2CM_DATA0)
#define	_I2CM_GET_DATA1()		IC_ReadByte(TWIC_P2, I2CM_DATA1)

#define _MAX_TRY 1000

/* I2C Master Initialization */

/*
 * Synopsis     unsigned char i2cm_init();
 * Description  iic模塊初始化.
 * Parameters   none
 * Return       iic模塊狀態
 */
unsigned char i2cm_init()
{	
	_I2CM_SET_STATUS(I2CM_400K);/* rates up to 400kbits/s */
	
	_I2CM_SET_MASK(0x00);
	   
	/* enable I2C master/instruct */
	_I2CM_SET_INST(I2CM_EN| I2CM_WAIT| I2C_INTR_EN);
	   
	return _I2CM_GET_STATUS();  /* return status */
}

/*
 * Synopsis     unsigned char i2cm_err();
 * Description  確認iic模塊是否發生錯誤.
 * Parameters   none
 * Return       1/0     - 錯誤/正常
 */
unsigned char i2cm_err(){
	unsigned char i2cm_stutas = 0x00;
	int i = 0;

    i2cm_stutas = _I2CM_GET_STATUS();
    /* check I2C Master status */
	if(i2cm_stutas &(I2CM_FAILED| DEVICE_ERROR| BUS_ERROR))
		return 1;

    /* check I2C Master use/busy */
	while(i2cm_stutas &(I2CM_BUSY| I2C_INUSE)) {
		if(i>_MAX_TRY)
			return 1;
		
		_I2CM_SET_STATUS( _I2CM_GET_STATUS() );
        i2cm_stutas = _I2CM_GET_STATUS();
        i++;
	}
	return 0;
}

/*
 * Synopsis     unsigned char i2cm_finish();
 * Description  iic模塊工作完成.
 * Parameters   none
 * Return       1/0     - 完成/工作失敗
 */
unsigned char i2cm_finish(){
	int i = 0;

	while(!(_I2CM_GET_STATUS()&FINISH_INTR)) {
		if(i>_MAX_TRY)
			return 0;
		i++;
	}
	return 1;
}

/* I2C Master Write Byte */
/*
 * Synopsis     unsigned char i2cm_wr_byte ( unsigned char slave, 
 *                                           unsigned char addr, 
 *                                           unsigned char datas);
 * Description  iic模塊寫入暫存器(byte).
 * Parameters   slave		        - 欲通訊器件位置 
 *              addr		        - 暫存器編號
 *              datas   	        - 資料
 * Return       0/1                 - 完成/工作失敗
 */
unsigned char i2cm_wr_byte (unsigned char slave, unsigned char addr, unsigned char datas)
{		
	if(i2cm_err()) 
		return I2CM_ERROR;
		
	_I2CM_SET_ADDR(slave,S_WRITE);
	_I2CM_SET_CMD(addr);
	_I2CM_SET_DATA0(datas);

	_I2CM_SET_INST((_I2CM_GET_INST()| START_INTR |I2CM_BYTE));

	if(i2cm_finish())
		return I2CM_FINISH;
	else
		return I2CM_ERROR;	
}

/*
 * Synopsis     unsigned char i2cm_wr_word ( unsigned char slave, 
 *                                           unsigned char addr, 
 *                                           unsigned short datas);
 * Description  iic模塊寫入暫存器(word).
 * Parameters   slave		        - 欲通訊器件位置 
 *              addr		        - 暫存器編號
 *              datas   	        - 資料
 * Return       0/1                 - 完成/工作失敗
 */
unsigned char i2cm_wr_word (unsigned char slave, unsigned char addr, unsigned short datas)
{		
	if(i2cm_err())
		return I2CM_ERROR;
		
	_I2CM_SET_ADDR(slave,S_WRITE);
	_I2CM_SET_CMD(addr);

	_I2CM_SET_DATA0(datas&0x00ff);
	_I2CM_SET_DATA1(datas>>8);

	_I2CM_SET_INST((_I2CM_GET_INST()| START_INTR |I2CM_WORD));

	if(i2cm_finish())
		return I2CM_FINISH;
	else
		return I2CM_ERROR;	
}

/* I2C Master Read Byte */
/*
 * Synopsis     unsigned char i2cm_rd_byte ( unsigned char slave, 
 *                                           unsigned char addr, 
 *                                           unsigned char *datas);
 * Description  iic模塊讀取暫存器(byte).
 * Parameters   slave		        - 欲通訊器件位置 
 *              addr		        - 暫存器編號
 *              datas   	        - 資料
 * Return       0/1                 - 完成/工作失敗
 */
unsigned char i2cm_rd_byte (unsigned char slave, unsigned char addr, unsigned char *datas)
{	
    if(i2cm_err())
		return I2CM_ERROR;

	_I2CM_SET_ADDR(slave,S_READ);
	_I2CM_SET_CMD(addr);

	_I2CM_SET_INST((_I2CM_GET_INST()| START_INTR |I2CM_BYTE));

	if(i2cm_finish()){
		*datas = _I2CM_GET_DATA0();
		return I2CM_FINISH;
	}	
	else{
		*datas = 0xFF;
		return I2CM_ERROR;
	}	
}

/*
 * Synopsis     unsigned char i2cm_rd_word ( unsigned char slave, 
 *                                           unsigned char addr, 
 *                                           unsigned short *datas);
 * Description  iic模塊讀取暫存器(word).
 * Parameters   slave		        - 欲通訊器件位置 
 *              addr		        - 暫存器編號
 *              datas   	        - 資料
 * Return       0/1                 - 完成/工作失敗
 */
unsigned char i2cm_rd_word (unsigned char slave, unsigned char addr, unsigned short *datas)
{	
    if(i2cm_err())
		return I2CM_ERROR;

	_I2CM_SET_ADDR(slave,S_READ);
	_I2CM_SET_CMD(addr);

	_I2CM_SET_INST((_I2CM_GET_INST()| START_INTR |I2CM_WORD));

	if(i2cm_finish()){
		*datas = (_I2CM_GET_DATA1()<<8)
				 |_I2CM_GET_DATA0();

		return I2CM_FINISH;
	}	
	else{
		*datas = 0xFFFF;
		return I2CM_ERROR;
	}	
}

