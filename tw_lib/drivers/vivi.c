/**
 *  @file   vivi.c
 *  @brief  vivi color setup
 *  $Id: vivi.c,v 1.3 2013/04/02 05:42:18 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.3 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  dos1236 	New file.
 *
 */

#include "sys.h"
#include "vivi.h"
#include "reg_tw.h"
#include "iccontrol.h"
#include "spirw.h"
#include "vivitable.h"
/*
 * 摘要 :vivi_init_fast(void)
 *
 * 描述 :载入快速模式的vivicolor format data
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 格式必须是 fast format
*/
void vivi_init_fast(void)
{
	int i=0;

	IC_WritByte(TWIC_P3, 0x60, 0x04);

	for(i=0;i<1536;i++)
	{
		IC_WritByte(TWIC_P3, 0x61, vivi_color_tbl[i]);
	}

	IC_WritByte(TWIC_P3, 0x60, 0x07);

	DBG_PRINT("enable VIVI color\n");
}
/*
 * 摘要 :vivi_init_slow(void)
 *
 * 描述 :载入快速模式的vivicolor format data
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 格式必须是 slow format
*/
void vivi_init_slow(void)
{
	int i=0;
	unsigned char j=0;
	IC_WritByte(TWIC_P3, 0x60, 0x00);//slow mode

	for(i=0;i<512;i++)
	{
		for(j=0;j<5;j++)
		{
			IC_WritByte(TWIC_P3, 0x65+j, vivi_color_tbl[i]);
		}
	}

	DBG_PRINT("init VIVI color\n");
}
/*
 * 摘要 :vivi_enable(void)
 *
 * 描述 :vivi color 启动
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void vivi_enable(void)
{
	IC_WritByte(TWIC_P3, 0x60, 0x03);
}
/*
 * 摘要 :vivi_disable(void)
 *
 * 描述 :vivi color 关闭
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void vivi_disable(void)
{
	IC_WritByte(TWIC_P3, 0x60, 0x00);
}
/*
 * 摘要 :ace_enable(void)
 *
 * 描述 :ace 启动
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void ace_enable(void)
{
	IC_WritByte(TWIC_P3, 0x7f, 0xe0);
}
/*
 * 摘要 :ace_disable(void)
 *
 * 描述 :ace 关闭
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void ace_disable(void)
{
	IC_WritByte(TWIC_P3, 0x7f, 0x00);
}
/*
 * 摘要 :vivi_init_dma(void)
 *
 * 描述 :使用dma方式处理载入vivicolor
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : format 限定 fast mode
*/
void vivi_init_dma(void)
{
	unsigned short size = 0x80;// change this to test
	unsigned char xdata *dma_vivi_tbl = 0x380;//0x400-size
	int i,j;

	IC_WritByte(TWIC_P3, 0x60, 0x04);//fast mode
												   
	for(i=0;i<1536;i+=size){
		
	//	if(1536-1-i < size) //last move	if 1536%size!=0, use it!
	//		size = 1536 - i;
				
		spi_dma_read((unsigned short)vivi_color_tbl+i, (unsigned short)dma_vivi_tbl, size);	 
		 	
		for(j=0;j<size;j++)
			IC_WritByte(TWIC_P3, 0x61, dma_vivi_tbl[j]);	
	}

	IC_WritByte(TWIC_P3, 0x60, 0x07); //enable vivi color

	DBG_PRINT("enable VIVI color(dma)\n");
}
