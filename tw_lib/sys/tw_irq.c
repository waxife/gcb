/**
 *  @file   tw_isr.c
 *  @brief  system irq entry
 *  $Id: tw_irq.c,v 1.6 2013/04/02 05:42:18 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.6 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "reg_tw.h"
#include "tw_irq.h"

/*
 * P6_12			Scaler Interrupt Status 1
 *	[7]		R/W		IR_Packet (Write 1 Clear)
 *	[6]		R/W		SPI_DMA_Done_Int (Write 1 Clear)
 *	[5]		R/W		VSYNC Lead.edge (Write 1 Clear)
 *	[4]		R/W		10ms Time Out (Write 1 Clear)
 *	[3]		R/W		HSYNC timing Chg (Write 1 Clear)
 *	[2]		R/W		VSYNC timing Chg (Write 1 Clear)
 *	[1]		R/W		HSYNC missing (Write 1 Clear)
 *	[0]		R/W		VSYNC missing (Write 1 Clear)		
 *
 * P6_13			Scaler Interrupt Status 2
 *	[7]		R/W		CQ_int (Write 1 Clear)
 *	[6]		R/W		GPIO_INT (Write 1 Clear)
 *	[5]		R/W		uart1_int (Write 1 Clear)
 *	[4]		R/W		uart0_int (Write 1 Clear)
 *	[3]		R/W		ShutDn_Warning (Write 1 Clear)
 *	[2]		R/W		ShortVS (Write 1 Clear)
 *	[1]		R/W		SAR1_Toggle (Write 1 Clear)
 *	[0]		R/W		SAR0_Toggle (Write 1 Clear)
 *
 * P6_14			Scaler Interrupt Mask 1
 *	[7]		R/W		Mask IR_Packet
 *	[6]		R/W		Mask SPI_DMA_Done_Int
 *	[5]		R/W		Mask VSYNCedg.int
 *	[4]		R/W		Mask 10ms Time Out
 *	[3]		R/W		Mask HSYNC Chg
 *	[2]		R/W		Mask VSYNC Chg
 *	[1]		R/W		Mask HSYNC miss
 *	[0]		R/W		Mask VSYNC miss
 *
 * P6_15			Scaler Interrupt Mask 2
 *	[7]		R/W		Mask CQ_int
 *	[6]		R/W		Mask Int_GPIO
 *	[5]		R/W		Mask uart1_int
 *	[4]		R/W		Mask uart0_int
 *	[3]		R/W		Mask SHDN_Warning
 *	[2]		R/W		Mask ShortVS
 *	[1]		R/W		Mask_SAR1_Toggle
 *	[0]		R/W		Mask_SAR0_Toggle
 */
#define INT_STATUS_REG1				IC_ReadByte(TWIC_P6, 0x12)
#define INT_STATUS_REG2				IC_ReadByte(TWIC_P6, 0x13)
#define INT_MASK_REG1				IC_ReadByte(TWIC_P6, 0x14)
#define INT_MASK_REG2				IC_ReadByte(TWIC_P6, 0x15)

#define INT_STATUS_CLEAR1(VAL)		do { IC_WritByte(TWIC_P6, 0x12, VAL); } while (0)
#define INT_STATUS_CLEAR2(VAL)		do { IC_WritByte(TWIC_P6, 0x13, VAL); } while (0)
#define INT_MASK_SET1(VAL)			do { IC_WritByte(TWIC_P6, 0x14, VAL); } while (0)
#define INT_MASK_SET2(VAL)			do { IC_WritByte(TWIC_P6, 0x15, VAL); } while (0)

#define INT_STATUS_REG1_INT			IC_ReadByte_INT(TWIC_P6, 0x12)
#define INT_STATUS_REG2_INT			IC_ReadByte_INT(TWIC_P6, 0x13)
#define INT_MASK_REG1_INT			IC_ReadByte_INT(TWIC_P6, 0x14)
#define INT_MASK_REG2_INT			IC_ReadByte_INT(TWIC_P6, 0x15)

#define INT_STATUS_CLEAR1_INT(VAL)	do { IC_WritByte_INT(TWIC_P6, 0x12, VAL); } while (0)
#define INT_STATUS_CLEAR2_INT(VAL)	do { IC_WritByte_INT(TWIC_P6, 0x13, VAL); } while (0)
#define INT_MASK_SET1_INT(VAL)		do { IC_WritByte_INT(TWIC_P6, 0x14, VAL); } while (0)
#define INT_MASK_SET2_INT(VAL)		do { IC_WritByte_INT(TWIC_P6, 0x15, VAL); } while (0)

/* STATUS POLARITY */
#define POL1_VSYNC_MISS		(1 << 0)
#define POL1_HSYNC_MISS		(1 << 1)
#define POL1_VSYNC_CHG		(1 << 2)
#define POL1_HSYNC_CHG		(1 << 3)
#define POL1_10MS_TIME		(1 << 4)
#define POL1_VSYNC_LEAD		(1 << 5)
#define POL1_SPI_DMA		(1 << 6)
#define POL1_IR_PACKET		(1 << 7)

#define POL2_SAR0_TOGGLE	(1 << 0)
#define POL2_SAR1_TOGGLE	(1 << 1)
#define POL2_SHORT_VS		(1 << 2)
#define POL2_HSYNC_CHG		(1 << 3)
#define POL2_UART0_INT		(1 << 4)
#define POL2_UART1_INT		(1 << 5)
#define POL2_GPIO_INT		(1 << 6)
#define POL2_CQ_INT			(1 << 7)

/* MASK POLARITY */
#define MASK1_VSYNC_MISS	(1 << 0)
#define MASK1_HSYNC_MISS	(1 << 1)
#define MASK1_VSYNC_CHG		(1 << 2)
#define MASK1_HSYNC_CHG		(1 << 3)
#define MASK1_10MS_TIME		(1 << 4)
#define MASK1_VSYNC_LEAD	(1 << 5)
#define MASK1_SPI_DMA		(1 << 6)
#define MASK1_IR_PACKET		(1 << 7)

#define MASK2_SAR0_TOGGLE	(1 << 0)
#define MASK2_SAR1_TOGGLE	(1 << 1)
#define MASK2_SHORT_VS		(1 << 2)
#define MASK2_HSYNC_CHG		(1 << 3)
#define MASK2_UART0_INT		(1 << 4)
#define MASK2_UART1_INT		(1 << 5)
#define MASK2_GPIO_INT		(1 << 6)
#define MASK2_CQ_INT		(1 << 7)

static unsigned char data INT_MASK_REG1_BK = 0, INT_MASK_REG2_BK = 0;
/*
 * 摘要 :void backup_tw_irq (void)
 *
 * 描述 :备份所有IRQ中断状态
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void backup_tw_irq (void)
{
	INT_MASK_REG1_BK = INT_MASK_REG1;
	INT_MASK_REG2_BK = INT_MASK_REG2;
}
/*
 * 摘要 :void mute_tw_irq (void)
 *
 * 描述 :关闭所有IRQ中断
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void mute_tw_irq (void)
{
	INT_MASK_REG1_BK = INT_MASK_REG1;
	INT_MASK_REG2_BK = INT_MASK_REG2;

	//DBG_PRINT("MUTE MASK1= 0x%bX!\r\n", INT_MASK_REG1_BK);
	//DBG_PRINT("MUTE MASK2= 0x%bX!\r\n", INT_MASK_REG2_BK);

	INT_MASK_SET1(0xFF);
	INT_MASK_SET2(0xFF);
}
/*
 * 摘要 :void restore_tw_irq (void)
 *
 * 描述 :回复IRQ之前的状态
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void restore_tw_irq (void)
{
	//DBG_PRINT("RE MASK1= 0x%bX!\r\n", INT_MASK_REG1_BK);
	//DBG_PRINT("RE MASK2= 0x%bX!\r\n", INT_MASK_REG2_BK);

	INT_MASK_SET1(INT_MASK_REG1_BK);
	INT_MASK_SET2(INT_MASK_REG2_BK);
}

/*
 * Level triggered interrupts need to write 1 clear
 */
/*
 * 摘要 :void clear_tw_irq(unsigned char irq_nr)
 *
 * 描述 :清除某个IRQ中断状态　写一清零
 *
 * 参数 :unsigned char irq_nr ＝＞要清除的IRQ号码
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void clear_tw_irq(unsigned char irq_nr)
{
	unsigned char data clear;
	if(irq_nr<8) {
		clear = (1 << irq_nr);
		INT_STATUS_CLEAR1_INT(clear);
	} else {
		clear = (1 << (irq_nr-8));
		INT_STATUS_CLEAR2_INT(clear);
	}
}
/*
 * 摘要 :void disable_tw_irq(unsigned char irq_nr)
 *
 * 描述 :关闭某个IRQ
 *
 * 参数 :unsigned char irq_nr ＝＞要关闭的IRQ号码
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void disable_tw_irq(unsigned char irq_nr)
{
	unsigned char data mask;
	if(irq_nr<8) {
		mask = INT_MASK_REG1;
		mask |= (1 << irq_nr);
		INT_MASK_SET1(mask);
		//DBG_PRINT("DISABLE MASK1= 0x%bX!\r\n", mask);
	} else {
		mask = INT_MASK_REG2;
		mask |= (1 << (irq_nr-8));
		INT_MASK_SET2(mask);
		//DBG_PRINT("DISABLE MASK2= 0x%bX!\r\n", mask);
	}
}
/*
 * 摘要 :void enable_tw_irq(unsigned char irq_nr)
 *
 * 描述 :开启某个IRQ
 *
 * 参数 :unsigned char irq_nr ＝＞要开启IRQ号码
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void enable_tw_irq(unsigned char irq_nr)
{
	unsigned char data mask;
	if(irq_nr<8) {
		mask = INT_MASK_REG1;
		mask &= ~(1 << irq_nr);
		INT_MASK_SET1(mask);
	} else {
		mask = INT_MASK_REG2;
		mask &= ~(1 << (irq_nr-8));
		INT_MASK_SET2(mask);
	}
}
/*
 * 摘要 :unsigned short tw_irq_dispatch(void)
 *
 * 描述 :得到IRQ开关状态
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
unsigned short tw_irq_dispatch(void)
{
	unsigned char data status1=0;
	unsigned char data status2=0;
	unsigned char data mask1=0xFF;
	unsigned char data mask2=0xFF;
	
	status1 = INT_STATUS_REG1_INT;
	status2 = INT_STATUS_REG2_INT;
	mask1 = INT_MASK_REG1_INT;
	mask2 = INT_MASK_REG2_INT;

	status1 &=(~mask1);
	status2 &=(~mask2);
	
	return ((status2<<8)|status1);
}
/*
 * 摘要 :void init_IRQ(void)
 *
 * 描述 :IRQ 初始化
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 预设全部关掉
*/
void init_IRQ(void)
{
	INT_MASK_SET1(0xFF);		/* all interrupt mask on */
	INT_MASK_SET2(0xFF);
	INT_STATUS_CLEAR1(0xFF);	/* all status clear */
	INT_STATUS_CLEAR2(0xFF);
	backup_tw_irq();
}
