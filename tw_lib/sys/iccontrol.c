/**
 *  @file   iccontrol.c
 *  @brief  ic control function
 *  $Id: iccontrol.c,v 1.7 2013/04/02 05:42:18 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.7 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "reg_tw.h"
#include "iccontrol.h"
#include "cq.h"

#define	USE_CQ_WRITE
#define FIX_FOR_GLOBAL_SHADOW
/*
 * 摘要 :void ClosePanel(unsigned char uR, unsigned char uG, unsigned char uB)
 *
 * 描述 :关闭INPUT影像显示讯号,并且设置内建PATTERN
 *
 * 参数 :unsigned char uR=> 红色值
 *		 unsigned char uG=> 绿色值
 *       unsigned char uB=> 蓝色值
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void ClosePanel(unsigned char uR, unsigned char uG, unsigned char uB)
{
#ifdef USE_CQ_WRITE
	cq_config(TWIC_P0, FRAME_CRED_CFG_REG, 1, 3);
	cq_data(uR);
	cq_data(uG);
	cq_data(uB);
	cq_write_byte (TWIC_P0, BTIN_PATTERN_REG, 0x87);
	cq_write_byte (TWIC_P0, POUT_VSYNC_CTRL_REG, (IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)|POUT_VSYNC_CTRL));
	cq_trigger_source (CQ_TRIGGER_SW);
	cq_flush();
#ifdef FIX_FOR_GLOBAL_SHADOW	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
#else
	IC_WritByte(TWIC_P0, FRAME_CRED_CFG_REG, uR);
	IC_WritByte(TWIC_P0, FRAME_CGRN_CFG_REG, uG);
	IC_WritByte(TWIC_P0, FRAME_CBLU_CFG_REG, uB);
	IC_WritByte(TWIC_P0, BTIN_PATTERN_REG, 0x87);
	IC_WritByte(TWIC_P0, POUT_VSYNC_CTRL_REG, (IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)|POUT_VSYNC_CTRL));
	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);

#endif
}
/*
 * 摘要 :void OpenPanel(void)
 *
 * 描述 :开启INPUT影像显示讯号
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void OpenPanel(void)
{	
	DBG_PRINT("\r1.  open panel BLUE  !!!\n\r");
#ifdef USE_CQ_WRITE
	cq_write_byte (TWIC_P0, POUT_VSYNC_CTRL_REG, (IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&(~POUT_VSYNC_CTRL)));
	cq_write_byte (TWIC_P0, BTIN_PATTERN_REG, 0x07); 
	cq_trigger_source (CQ_TRIGGER_VSYNC);
	cq_flush();
#ifdef FIX_FOR_GLOBAL_SHADOW	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif		
#else
	IC_WritByte(TWIC_P0, POUT_VSYNC_CTRL_REG, (IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&(~POUT_VSYNC_CTRL)));
	IC_WritByte(TWIC_P0, BTIN_PATTERN_REG, 0x07);
	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);

#endif
}
/*
 * 摘要 :void WriteRegsTable(unsigned char code* RegsTable,unsigned char trig)
 *
 * 描述 :针对各种不同INPUT讯号,设置暂存器　
 *
 * 参数 :unsigned char code* RegsTable=> TABLE指针
 *　　　unsigned char trig=>COMMAND QUEUE 触发方式 0:SOFTWARE TRIG 1:VDE TRIG 2:VSYNC TRIG
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void WriteRegsTable(unsigned char code* RegsTable,unsigned char trig)
{
	unsigned short Index;
	unsigned short curID;
	unsigned short address=0;
	unsigned char tmp=0;
	curID = TWIC_P0;
	Index = 0;
#ifdef USE_CQ_WRITE
	cq_WriteRegsTable_issue(RegsTable, 4, trig);
	if (cq_flush () < 0)
		ERROR (("cq_flush()\n"));
#ifdef FIX_FOR_GLOBAL_SHADOW	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif		
#else
	IC_WritByte(TWIC_P2,0x3F,0x01);
	while(1)
	{
		if(RegsTable[Index] == 0xFF)
		{
			curID += PAGE_OFFSET;
			if(curID > TWIC_P3)
				break;
		}
		IC_WritByte(curID,RegsTable[Index],RegsTable[Index+1]);
		Index += 2;
	}	

	IC_WritByte(TWIC_P2,0x3f,0x00);
#endif	
}

#define SHADOW_CTRL_CONF_REG		0xE2
#define SHADOW_CTRL_CONF			0x11
#define SHADOW_BUSY					0x01
/*
 * 摘要 :void check_shadow(void)
 *
 * 描述 :确认SHADOW状态
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void check_shadow(void)
{
	unsigned short count= 0;
	unsigned char temp= 0;

	temp= IC_ReadByte(TWIC_P0,SHADOW_CTRL_CONF_REG);
	
	IC_WritByte(TWIC_P0,SHADOW_CTRL_CONF_REG,SHADOW_CTRL_CONF);
	for(count= 0; count<0xFFFF; count++) {
		if(!(IC_ReadByte(TWIC_P0,SHADOW_CTRL_CONF_REG)&SHADOW_BUSY))
			goto EXIT;
		/* v blanking star */
	}
	ERROR(("NO VSYNC, NEED ENTER FREE RUN STATUS!\r\n"));
EXIT:
	IC_WritByte(TWIC_P0,SHADOW_CTRL_CONF_REG, temp);	
}
