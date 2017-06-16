/**
 *  @file   spirw.c
 *  @brief  terawins SPI control function
 *  $Id: spirw.c,v 1.25 2013/09/13 01:31:01 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.25 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include <reg51.h>
#include "sys.h"
#include "reg_tw.h"
#include "iccontrol.h"
#include "spiosd.h"
#include "spirw.h"
#include "cq.h"

#ifdef	RESOURCE_ON_NOR_FLASH
#include "tw_req_ser.h"
#endif

//#define SPIRW_DEBUG_MODE

unsigned char SPIRW_FLAG= 0;
/* Variable Flag */
#define SPI_QUAD_MODE		((SPIRW_FLAG>>0)&0x01)
#define SET_SPI_QUAD_MODE	do { SPIRW_FLAG |= 0x01; } while (0)
#define CLR_SPI_QUAD_MODE	do { SPIRW_FLAG &= 0xFE; } while (0)

#define SPI_HP_MODE_USE		((SPIRW_FLAG>>1)&0x01)
#define SET_SPI_HP_MODE_USE	do { SPIRW_FLAG |= 0x02; } while (0)
#define CLR_SPI_HP_MODE_USE	do { SPIRW_FLAG &= 0xFD; } while (0)

//static unsigned char spi_quad_mode = 0;
//static unsigned char spi_hp_mode_use = 0;     // high performance mode
/*
 * 摘要 :void enable_quad_io(void)
 *
 * 描述 :开启Flash quad mode
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
bit enable_quad_io(void)
{
	bit rc = 1;

#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
    unsigned char reg=0;
	unsigned char reg1=0;
	unsigned char count=0;
	INFO(("WIN_QUAD_NOR \r\n"));
	#if (defined(WIN_PROTECT_STATUS))
		spi_wrsr(BLOCK_PROTECT,QUAD_PROTECT_GD128);             // flash quad enable
	#else
		spi_wrsr(BLOCK_PROTECT,QUAD_ENABLE_WIN);
	#endif
	for(count=0; count<255; count++) {
		spi_rdsr_WQ(&reg,&reg1);
		if(!(reg&0x01))
			break;
	}

	INFO(("WQ_QUAD_NOR r1=%bx r2=%bx\r\n",reg,reg1));

	if(!(reg1&0x02)) {
		ERROR(("quad enable error! \r\n"));
		while(1);
	}
	if(reg!=0x3c) {
		//ERROR(("Protect enable error! \r\n"));
		rc = 0;
	}
	#if (defined(WIN_PROTECT_STATUS))
	if(!(reg1&0x01)) {
		//ERROR(("GD 128M SPR1 error! \r\n"));
		rc = 0;
	}
	#endif

#elif (defined(GD_128M_QUAD_NOR)||defined(WQ_25Q_128_QUAD_NOR))
	unsigned char reg=0;
	unsigned char reg1=0;
	unsigned char reg2=0;
	unsigned char count=0;
	INFO(("GD_128M_QUAD_NOR \r\n"));
	spi_wrsr(BLOCK_PROTECT,WS_REG0_GD128M);			   // flash quad enable
	spi_wrsr(QUAD_ENABLE_GD128,WS_REG1_GD128M);			   // flash quad enable
	spi_wrsr(QUAD_STATUS_3_GD128,WS_REG2_GD128M);			   // flash quad enable
#if (defined(GD_128M_STATUS_PROTECT)||defined(WQ_128M_STATUS_PROTECT))
	spi_wrsr(QUAD_PROTECT_GD128,WS_REG1_GD128M);
#endif
	for(count=0; count<255; count++) {
	spi_rdsr_GD(&reg,&reg1,&reg2);
		
	if(!(reg&0x01))
		break;
	}

	INFO(("GD_128M_QUAD_NOR r1=%bx r2=%bx r3=%bx\r\n",reg,reg1,reg2));

	if(!(reg1&0x02)) {
		ERROR(("GD 128M quad enable error! \r\n"));
		while(1);
	}

	if((reg&0x3c)!=0x3c) {
		//ERROR(("GD 128M Block protect error! \r\n"));
		rc = 0;
	}
	if(reg1&0x40) {
		//ERROR(("GD 128M CMP disable error! \r\n"));
		rc = 0;
	}
	if(reg2&0x04) {
		//ERROR(("GD 128M WPS disable error! \r\n"));
		rc = 0;
	}
	#if (defined(GD_128M_STATUS_PROTECT)||defined(WQ_128M_STATUS_PROTECT))
	if(!(reg1&0x01)) {
		//ERROR(("GD 128M SPR1 error! \r\n"));
		rc = 0;
	}
	#endif

#else
    unsigned char reg=0;
	unsigned char count=0;
	INFO(("MXIC_QUAD_NOR \r\n"));
	spi_wrsr(0x7c,0X00);             // flash quad enable
	for(count=0; count<255; count++) {
		spi_rdsr(&reg);
		if(!(reg&0x01))
			break;
	}
	if(!(reg&0x40)) {
		ERROR(("quad enable error! \r\n"));
		while(1);
	}
	if(reg!=0x3c) {
		//ERROR(("Protect enable error! \r\n"));
		rc = 0;
	}
#endif
	spi_quad_mode_enable();             // 4x_8051, 4x_dma
	return rc;
//	spiosd_quad_mode_enable();          // 4x_sosd
}
/*
 * 摘要 :void spi_toggle_byte_set(void)
 *
 * 描述 :设定Flash toggle byte
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void spi_toggle_byte_set(void)
{   
    IC_WritByte(TWIC_P6, SPI_WR_DATA_REG, NONE_TOGGLE_VALUE);
    IC_WritByte(TWIC_P6, SPI_TOGGLE_WR_DATA, HPM_TOGGLE_VALUE);
}
/*
 * 摘要 :void ospi_toggle_byte_set(void)
 *
 * 描述 :设定OFlash toggle byte
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void ospi_toggle_byte_set(void)
{
    IC_WritByte(TWIC_P6, OSPI_WR_DATA_REG, NONE_TOGGLE_VALUE);
    IC_WritByte(TWIC_P6, OSPI_TOGGLE_WR_DATA, HPM_TOGGLE_VALUE);
}
/*
 * 摘要 :void spi_quad_mode_enable(void)
 *
 * 描述 :设定SPI QUAD MODE 使能
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void spi_quad_mode_enable(void)
{
    SET_SPI_QUAD_MODE;
   	IC_WritByte(TWIC_P6, SPI_4X_ENABLE_REG, SPI_4X_I8051 | SPI_4X_DMA);
}
/*
 * 摘要 :void spi_quad_mode_disable(void)
 *
 * 描述 :设定SPI QUAD MODE 关闭
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void spi_quad_mode_disable(void)
{
    CLR_SPI_QUAD_MODE;
   	IC_WritByte(TWIC_P6, SPI_4X_ENABLE_REG, 0);    
}
/*
 * 摘要 :void dma_int_enable()
 *
 * 描述 :设定DMA中断开启
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void dma_int_enable()
{
    IC_WritByte(TWIC_P6, 0x14, (IC_ReadByte(TWIC_P6, 0x14) & 0xbf));
}
/*
 * 摘要 :void dma_int_disable()
 *
 * 描述 :设定DMA中断关闭
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void dma_int_disable()
{
    IC_WritByte(TWIC_P6, 0x14, (IC_ReadByte(TWIC_P6, 0x14) | 0x40));
}
/*
 * 摘要 :bit wait_dma_int(void)
 *
 * 描述 :等待DMA中断完成
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
// wait dma done, check interrupt
bit wait_dma_int(void)
{
    unsigned short i;
    for(i = 0; i < 65535; i++){
        if((IC_ReadByte(TWIC_P6, 0x12) & 0x40)){
          	IC_WritByte(TWIC_P6, 0x12, (IC_ReadByte(TWIC_P6, 0x12) | 0x40) );
            return 1;
        }
        else
            twdDelay(3);
    }
    DBG_PRINT("Wait DMA Interrupt Timeout\r\n");
    return 0;
}
/*
 * 摘要 :bit wait_dma(void)
 *
 * 描述 :等待DMA完成
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
/* SPI: Wait DMA done => 1 */
bit wait_dma(void)
{
	unsigned short count=0;
	for(count=0; count<65535; count++) {
		if((IC_ReadByte(TWIC_P6, SPI_DMA_CTRL_REG)&SPI_DMA_GO))
			return 1;
		else
			twdDelay(3);
	}
    DBG_PRINT("Wait DMA Timeout\r\n");
	return 0;
}
/*
 * 摘要 :bit spi_dma2xram(unsigned long address, unsigned short xaddr, unsigned short length)
 *
 * 描述 :从FLASH透过DMA到XRAM
 *
 * 参数 :unsigned long address  =>FLASH位置
 *       unsigned short xaddr   =>XRAM位置
 *       unsigned short length  =>搬运资料大小
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
bit spi_dma2xram(unsigned long address, unsigned short xaddr, unsigned short length)
{
    bit rc = 0;
    
#ifdef SPIRW_DEBUG_MODE
	dbg(2, ("spi_dma2xram address= 0x%lX, xaddr= 0x%X, length= %u\r\n", address, xaddr, length));
#endif
	
	if(SPI_QUAD_MODE)
    {
        IC_WritByte(TWIC_P6, SPI_4X_ENABLE_REG, SPI_4X_I8051 | SPI_4X_DMA);
      	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);
     	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_DMA_4X);

    }
    else
    {
        IC_WritByte(TWIC_P6, SPI_4X_ENABLE_REG, 0);
      	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ);
     	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_DMA);
    }

	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (address&0x000000FF));		// AL
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((address>>8)&0x000000FF));	// AM
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((address>>16)&0x000000FF));	// AH

    spi_toggle_byte_set();
    
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG1, xaddr&0x00FF);				// External RAM address
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG2, (xaddr>>8)&0x00FF);			// External RAM address
	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG1, length&0x00FF);			// DMA count
	IC_WritByte(TWIC_P6, SPI_DMA_CTRL_REG, DMA_READ|((length&SPI_DMA_COUNT_BIT)>>8));	// DMA go
	
    if((IC_ReadByte(TWIC_P6, 0x14) & 0x40))// dma interrupt mask
    {
       	if(!wait_dma())
	    	goto EXIT;
    }
    else// dma interrupt unmask
    {
        if(!wait_dma_int())
		    goto EXIT;
    }
	rc = 1;
EXIT:    
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);
	return rc;
}
/*
 * 摘要 :bit spi_dma2oram(unsigned long base_address, unsigned short oram_address, unsigned int length)
 *
 * 描述 :从FLASH透过DMA到ORAM
 *
 * 参数 :unsigned long address  =>FLASH位置
 *       unsigned short xaddr   =>XRAM位置
 *       unsigned short length  =>搬运资料大小
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
#define EX_MCU_DMA_DELAY_VALUE	100
bit spi_dma2oram(unsigned long base_address, unsigned short oram_address, unsigned int length)
{
    bit rc = 0;
#if (defined(EX_I2C_SAFE_MODE)&&(!defined(ENABLE_EX_MCU)))
	unsigned char curr_addr= IC_ReadByte(TWIC_P0, 0xF0);
	IC_WritByte(TWIC_P0, 0xF0, SLAVE_SAFE_ADDR);
#endif
    // Notice: OSDRam address is Word Unit
    //            Flash Address is Byte Unit, blit length is also Byte Unit	
	/* word access, LSB first, then MSB byte */	
	IC_WritByte(TWIC_P0, 0xAA, (unsigned char)(oram_address & 0xff));
	IC_WritByte(TWIC_P0, 0xAA, (unsigned char)(oram_address>>8));

    if(SPI_QUAD_MODE)
    {
        IC_WritByte(TWIC_P6, SPI_4X_ENABLE_REG, SPI_4X_I8051 | SPI_4X_DMA);
      	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);
        IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_DMA_4X);
    }
    else
    {
        IC_WritByte(TWIC_P6, SPI_4X_ENABLE_REG, 0);
      	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ);
        IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_DMA);
    }
    
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (base_address&0x000000FF));		    // AL
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((base_address>>8)&0x000000FF));	// AM
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((base_address>>16)&0x000000FF));	// AH

    spi_toggle_byte_set();
    
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG1, OSDRAM_DATA_PORT&0x00FF);			// OSDRam Dataport
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG2, (OSDRAM_DATA_PORT>>8)&0x00FF);	// OSDRam Dataport
	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG1, length & 0x00FF);				// DMA count;

#ifndef ENABLE_EX_MCU
	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG2, DMA_READ|((length & SPI_DMA_COUNT_BIT)>>8));	// DMA go;
    
	// Notice: FPGA Sample Code check P6_12[6], and need write 1 clear
    if((IC_ReadByte(TWIC_P6, 0x14) & 0x40))// dma interrupt mask
    {
       	if(!wait_dma())
	    	goto EXIT;
    }
    else// dma interrupt unmask
    {
        if(!wait_dma_int())
		    goto EXIT;
    }
#ifdef EX_I2C_SAFE_MODE
	if(curr_addr==SLAVE_WORK_ADDR)
		IC_WritByte(TWIC_P0, 0xF0, SLAVE_WORK_ADDR);
#endif
    rc = 1;	
EXIT:    
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);	
	return rc;
#else
	#ifdef RESOURCE_ON_NOR_FLASH
	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG2, /*DMA_READ|*/((length & SPI_DMA_COUNT_BIT)>>8));	// DMA go;	
	rc = req_ser_dma_go();
	if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);
	#endif
	return rc;

#endif
}
/*
 * 摘要 :static bit wait_cmd(void)
 *
 * 描述 :SPI等待COMMAND ISSUE完成
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
/* SPI: Wait command issue => 0 */
static bit wait_cmd(void)
{
	unsigned char count=0;
	for(count=0; count<255; count++) {
		if(!(IC_ReadByte(TWIC_P6, SPI_COMMAND_REG)&SPI_ISSUE_CMD))
			return 1;
		else
			twdDelay(3);
	}
	return 0;
}
/*
 * 摘要 :static bit wait_wip(void)
 *
 * 描述 :SPI等待WIP(Write in progress)状态是否结束
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
/* SPI: Wait WIP => 1 */
static bit wait_wip(void)
{
	unsigned char count=0;
	for(count=0; count<255; count++) {
		if((IC_ReadByte(TWIC_P6, SPI_COMMAND_REG)&SPI_CHECK_WIP))
			return 1;
		else
			twdDelay(3);
	}
	return 0;
}
/*
 * 摘要 :bit spi_wren(void)
 *
 * 描述 :对FLASH下达WREN命令,允许FLASH改变状态
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
/* write enable command */
bit spi_wren(void)
{
    bit rc = 0;
	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, WRITE_ENABLE);	// WREN command
	if(!wait_wip())
		goto EXIT;
	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, WRITE_INSTRUCTION);	// Control 
	rc = 1;
    
EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
    return rc;
}
/*
 * 摘要 :bit spi_wrsr(unsigned char reg1, unsigned char reg2)
 *
 * 描述 :改变Flash状态,可以读取rdsr确认状态值
 *
 * 参数 :unsigned char reg1  =>暂存器一
 *       unsigned char reg2  =>暂存器二
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : mxic是一个byte winbond是两个byte 
*/
/* write status register command */
bit spi_wrsr(unsigned char reg1, unsigned char reg2)
{
    bit rc = 0;
	if(!spi_wren())
        goto EXIT; 
    
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, WS_REG);      // WRSR command
    IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, reg2);            // register value
    IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, reg1);              // register value
    IC_WritByte(TWIC_P6, SPI_COMMAND_REG, WRITE_RSR_WIN);   // Control
#elif (defined(GD_128M_QUAD_NOR)||defined(WQ_25Q_128_QUAD_NOR))    //gd128M
	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, reg2);		// WRSR command
	IC_WritByte(TWIC_P6, SPI_WR_DATA_REG, reg1); 			// register value
	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, WRITE_RSR);		// Control
#else
	reg2=0x00;
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, WS_REG);	    // WRSR command
	IC_WritByte(TWIC_P6, SPI_WR_DATA_REG, reg1);			    // register value
	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, WRITE_RSR);	    // Control
#endif

	if(!wait_wip())
        goto EXIT; 

    rc = 1;
 EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
        
    spi_toggle_byte_set();
    return rc;
}
/*
 * 摘要 :bit spi_rdsr(unsigned char* states)
 *
 * 描述 :读取flash内状态值
 *
 * 参数 :unsigned char* states =>得到的值会写入这个指标
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
/* read status register command */
bit spi_rdsr(unsigned char* states)
{
    bit rc = 0;
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, RS_REG_WIN); // RDSR command
#else
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, RS_REG); // RDSR command
#endif

	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_RSR);	// Control
	if(!wait_cmd())
		goto EXIT;
	*states = IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);	// register value
    rc = 1;	
  EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
    return rc;    
}

#if (defined(GD_128M_QUAD_NOR)||defined(WQ_25Q_128_QUAD_NOR))
bit spi_rdsr_GD(unsigned char* states,unsigned char* states1,unsigned char* states2)
{
    bit rc = 0;
	//step 1
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, RS_REG0_GD); // RDSR command

	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_RSR);	// Control
	if(!wait_cmd())
		goto EXIT;
	*states = IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);	// register value0
	//step 2
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, RS_REG1_GD); // RDSR command

	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_RSR);	// Control
	if(!wait_cmd())
		goto EXIT;
	*states1 = IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);	// register value1
	//step 3
	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, RS_REG2_GD); // RDSR command

	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_RSR);	// Control
	if(!wait_cmd())
		goto EXIT;
	*states2 = IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);	// register value1

	
    rc = 1;	
  EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
    return rc;    
}
#endif
#if (defined(WIN_QUAD_NOR)||defined(AMIC_QUAD_NOR))
bit spi_rdsr_WQ(unsigned char* states,unsigned char* states1)
{
    bit rc = 0;
	//step 1
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, RS_REG0_GD); // RDSR command

	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_RSR);	// Control
	if(!wait_cmd())
		goto EXIT;
	*states = IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);	// register value0
	//step 2
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, RS_REG1_GD); // RDSR command

	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_RSR);	// Control
	if(!wait_cmd())
		goto EXIT;
	*states1 = IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);	// register value1
    rc = 1;	
  EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
    return rc;    
}
#endif
/*
 * 摘要 :void spi_check_shadow(void)
 *
 * 描述 :检查shadow状态,并且等到vsync blank跳出
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void spi_check_shadow(void)
{

	IC_WritByte(TWIC_P0,SHADOW_CTRL_CONF_REG,SHADOW_CTRL_CONF);
	while(IC_ReadByte(TWIC_P0,SHADOW_CTRL_CONF_REG)&SHADOW_BUSY)
		continue;
	/* v blanking star */
}
/*
 * 摘要 :bit spi_dma_write(unsigned long address, unsigned short xaddr, unsigned short length)
 *
 * 描述 :从xram将资要写到FLASH内
 *
 * 参数 :unsigned long address  =>FLASH位置
 *       unsigned short xaddr   =>XRAM位置
 *       unsigned short length  =>搬运资料大小
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
/* --- Program SPI Flash ROM via DMA moving from XDATA (0x0000~0x0400) --- */
bit spi_dma_write(unsigned long address, unsigned short xaddr, unsigned short length)
{
    bit rc = 0;
    char    i;
    unsigned short   remain;
    unsigned short   len[2] = {0};
    unsigned long    addr;
    unsigned short   src;
    
    if(length > 256)
    {
    	ERROR(("spi_dma_write length > 256 bytes\r\n"));
        goto EXIT;
    }

    remain = (unsigned short)(256 - (address % 256));
    addr = address;
    src  = xaddr;
    len[0] = length;
    len[1] = 0;
    
    if(remain < length)
    {
        len[0] = remain;
        len[1] = length - remain;
    }
    

    for(i = 0; i < 2; i++)
    {
        if(i == 1)
        {
            if(len[1] == 0)
                break;
            addr = address + remain;
            src += len[0];
        }
        
    	if(!spi_wren())
	    	goto EXIT;
#if 1
		spi_check_shadow();	
		IC_WritByte(TWIC_P0,0xE2,0x00);			
    	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, PAGE_PROGRAM);			// PP command
	    IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (addr&0x000000FF));		// AL
    	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((addr>>8)&0x000000FF));	// AM
	    IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((addr>>16)&0x000000FF));	// AH
    	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, WRITE_DMA);					// Control w/ Check WIP
	    IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG1, src&0x00FF);				// External RAM address
    	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG2, (src>>8)&0x00FF);			// External RAM address
	    IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG1, len[i]&0x00FF);				// DMA count
#endif
#if 0		
    	cq_write_byte(TWIC_P6, SPI_INSTRUCTION_REG, PAGE_PROGRAM);			// PP command
	    cq_write_byte(TWIC_P6, SPI_ADDRESS_REG1, (addr&0x000000FF));		// AL
    	cq_write_byte(TWIC_P6, SPI_ADDRESS_REG2, ((addr>>8)&0x000000FF));	// AM
	    cq_write_byte(TWIC_P6, SPI_ADDRESS_REG3, ((addr>>16)&0x000000FF));	// AH
    	cq_write_byte(TWIC_P6, SPI_COMMAND_REG, WRITE_DMA);					// Control w/ Check WIP
	    cq_write_byte(TWIC_P6, SPI_DMA_ADDR_REG1, src&0x00FF);				// External RAM address
    	cq_write_byte(TWIC_P6, SPI_DMA_ADDR_REG2, (src>>8)&0x00FF);			// External RAM address
	    cq_write_byte(TWIC_P6, SPI_DMA_COUNT_REG1, len[i]&0x00FF);				// DMA count
		cq_flush_vde();
#endif
		IC_WritByte(TWIC_P6, SPI_DMA_CTRL_REG, DMA_WRITE|((len[i]&SPI_DMA_COUNT_BIT)>>8));	// DMA go

  	    if(!wait_wip())
        { 
            DBG_PRINT("spi_dma_write wip fail\r\n");
            goto EXIT;
        }
    }

    rc = 1;
EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);	
	return rc;
}
/*
 * 摘要 :unsigned char spi_read_byte(unsigned long address)
 *
 * 描述 :从FLASH读取一个BYTE
 *
 * 参数 :unsigned long address  =>FLASH位置
 *
 * 回传值 : 回传读回的资料
 *
 * 注意 : 无
*/
#define SPI_RD_BACK_DATA_REG_MAP	0xF600|SPI_RD_BACK_DATA_REG1
unsigned char spi_read_byte(unsigned long address)
{
#if 0
	/* NOR FLASH READ BYTE COMMAND */
	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, READ_DATA);
	
	/* NOR FLASH ADDRESS */
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, address&0xFF);
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, (address>>8)&0xFF);
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, (address>>16)&0xFF);
	
	/* SPI READ BYTE COMMAND */
	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_BYTE);
	
	//if(!wait_cmd())
	//	return 0;

 
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
 	return IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);
#else
    #ifndef ENABLE_EX_MCU
        unsigned char read;
        spi_dma2xram(address, (unsigned short) &read, 1);
        return read;
    #else
        spi_dma2xram(address, SPI_RD_BACK_DATA_REG_MAP, 1);
        return IC_ReadByte(TWIC_P6, SPI_RD_BACK_DATA_REG1);
    #endif
#endif
}
/*
 * 摘要 :bit spi_write_byte(unsigned long address, unsigned char value)
 *
 * 描述 :从FLASH写入一个BYTE
 *
 * 参数 :unsigned long address  =>FLASH位置
 *       unsigned char value    =>欲写入的值
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
bit spi_write_byte(unsigned long address, unsigned char value)
{	
#if 0
    bit rc = 0;
	/* WRSR */
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
    spi_wrsr(NO_PROTECT | QUAD_ENABLE_WIN);
#else
    spi_wrsr(NO_PROTECT | QUAD_ENABLE);
#endif

	/* SPI PRE WREN */
	//IC_WritByte(TWIC_P1, SPI_4X_ENABLE_REG, IC_ReadByte(TWIC_P1,SPI_4X_ENABLE_REG)|SPI_PRE_WREN);
    
	if(!spi_wren())
        goto EXIT;

	/* PP */
	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, PAGE_PROGRAM);

	/* NOR FLASH ADDRESS */
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, address&0xFF);
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, (address>>8)&0xFF);
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, (address>>16)&0xFF);

	/* WRITE DATA */
	IC_WritByte(TWIC_P6, SPI_WR_DATA_REG, value);

	/* SPI SECTOR ERASE COMMAND */
	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, WRITE_BYTE);

	if(!wait_cmd())
         goto EXIT;
    
    rc = 1;
EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
    spi_toggle_byte_set();
 
	return rc;
#else
    return spi_dma_write(address, (unsigned short)&value, 1);
#endif
}
/*
 * 摘要 :void spi_hp_mode_using(void)
 *
 * 描述 :SPI HIGH PERFORMANCE MODE使用
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void spi_hp_mode_using(void)
{
    SET_SPI_HP_MODE_USE;
}
/*
 * 摘要 :void spi_hp_mode_stop_using(void)
 *
 * 描述 :SPI HIGH PERFORMANCE MODE暂停使用
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void spi_hp_mode_stop_using(void)
{
    CLR_SPI_HP_MODE_USE;
}
/*
 * 摘要 :bit spi_hp_mode_disable(void)
 *
 * 描述 :SPI HIGH PERFORMANCE MODE关闭
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败  1= 成功 
 *
 * 注意 : 无
*/
bit spi_hp_mode_disable(void)
{
    int i = 0;
    IC_WritByte(TWIC_P6, SPI_ARB_EN_I8051, 0);

    // invalid i cache
	IC_WritByte(TWIC_P1, 0xFD, IC_ReadByte(TWIC_P1, 0xFD) | 0x80);
	IC_WritByte(TWIC_P1, 0xFD, IC_ReadByte(TWIC_P1, 0xFD) & 0x7F);

    IC_WritByte(TWIC_P6, SPI_HP_REG, IC_ReadByte(TWIC_P6, SPI_HP_REG) & ~SPI_HP_EN);
    for(i = 0; i < 255; i++)
    {
        if(!(IC_ReadByte(TWIC_P6, SPI_HP_REG) & SPI_HP_EXTRA8T))
        {
                DBG_PRINT("SPI High Performance Mode Disable Success\n");
                return 1;
        }
        twdDelay(3);
    }

    DBG_PRINT("SPI High Performance Mode Disable Fail\n");    
    IC_WritByte(TWIC_P6, SPI_ARB_EN_I8051, 0xC7);
    return 0;
}
/*
 * 摘要 :bit spi_hp_mode_enable(void)
 *
 * 描述 :SPI HIGH PERFORMANCE MODE开启
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败  1= 成功 
 *
 * 注意 : 无
*/
bit spi_hp_mode_enable(void)
{
    int i = 0;
       
    IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);
    spi_toggle_byte_set();
    ospi_toggle_byte_set();

    // workaround recommand by shawn
    IC_WritByte(TWIC_P6, SPI_ARB_EN_I8051, 0);
    twdDelay(200);
         
    // invalid i cache
	IC_WritByte(TWIC_P1, 0xFD, IC_ReadByte(TWIC_P1, 0xFD) | 0x80);
	IC_WritByte(TWIC_P1, 0xFD, IC_ReadByte(TWIC_P1, 0xFD) & 0x7F);
    
    IC_WritByte(TWIC_P6, SPI_HP_REG, (IC_ReadByte(TWIC_P6, SPI_HP_REG) | SPI_HP_EN));
     
    for(i = 0; i < 255; i++)
    {
        if(IC_ReadByte(TWIC_P6, SPI_HP_REG) & SPI_HP_EXTRA8T)
        {
                DBG_PRINT("SPI High Performance Mode Enable Success\n");
                return 1;
        }
        twdDelay(3);
    }

    DBG_PRINT("SPI High Performance Mode Enable Fail\n");
    IC_WritByte(TWIC_P6, SPI_ARB_EN_I8051, 0xC7);
    return 0;
}
/*
 * 摘要 :static bit wait_ospi_cmd(void)
 *
 * 描述 :OSPI等待COMMAND ISSUE完成
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
static bit wait_ospi_cmd(void)
{
	unsigned char count=0;
	for(count=0; count<255; count++) {
		if(!(IC_ReadByte(TWIC_P6, OSPI_COMMAND_REG)&OSPI_ISSUE_CMD))
			return 1;
		else
			twdDelay(3);
	}
	return 0;
}
/*
 * 摘要 :static bit wait_ospi_wip(void)
 *
 * 描述 :OSPI等待WIP(Write in progress)状态是否结束
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
static bit wait_ospi_wip(void)
{
	unsigned char count=0;
	for(count=0; count<255; count++) {
		if((IC_ReadByte(TWIC_P6, OSPI_COMMAND_REG)&OSPI_CHECK_WIP))
			return 1;
		else
			twdDelay(3);
	}
	return 0;
}
/*
 * 摘要 :bit ospi_wren(void)
 *
 * 描述 :对FLASH下达WREN命令,允许FLASH改变状态
 *
 * 参数 :无
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
/* write enable command */
bit ospi_wren(void)
{
    bit rc = 0;
	IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, WRITE_ENABLE);	// WREN command
	if(!wait_ospi_wip())
        goto EXIT;
	IC_WritByte(TWIC_P6, OSPI_COMMAND_REG, WRITE_INSTRUCTION);	// Control
	rc = 1;
EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, FAST_READ_4X);
	return rc;
}
/*
 * 摘要 :bit ospi_wrsr(unsigned char reg1, unsigned char reg2)
 *
 * 描述 :改变Flash状态,可以读取rdsr确认状态值
 *
 * 参数 :unsigned char reg  =>暂存器
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : mxic是一个byte winbond是两个byte 
*/
/* write status register command */
bit ospi_wrsr(unsigned char reg)
{
    bit rc = 0;
    if(!ospi_wren())
        goto EXIT;

#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
    IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, WS_REG);    // WRSR command
    IC_WritByte(TWIC_P6, OSPI_ADDRESS_REG2, reg);          // register value
    IC_WritByte(TWIC_P6, OSPI_ADDRESS_REG3, 0);              // register value
    IC_WritByte(TWIC_P6, OSPI_COMMAND_REG, WRITE_RSR_WIN);       // Control
#else
    IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, WS_REG);    // WRSR command
    IC_WritByte(TWIC_P6, OSPI_WR_DATA_REG, reg);           // register value
    IC_WritByte(TWIC_P6, OSPI_COMMAND_REG, WRITE_RSR);     // Control
#endif

    if(!wait_ospi_wip())
        goto EXIT;
    rc = 1;
EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, FAST_READ_4X);
    
    ospi_toggle_byte_set();
    return rc;
}
/*
 * 摘要 :bit ospi_rdsr(unsigned char* states)
 *
 * 描述 :读取flash内状态值
 *
 * 参数 :unsigned char* states =>得到的值会写入这个指标
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
bit ospi_rdsr(unsigned char* states)
{
    bit rc = 0;
    
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
	IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, RS_REG_WIN);	// RDSR command
#else
	IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, RS_REG);	// RDSR command
#endif
    
	IC_WritByte(TWIC_P6, OSPI_COMMAND_REG, READ_RSR);	// Control
	// [5:4] = oSPI_RdMUX[1:0] = 0~3;
    IC_WritByte(TWIC_P6, OSPI_DMA_CTRL_REG2, (IC_ReadByte(TWIC_P6, OSPI_DMA_CTRL_REG2) & 0xCF) | 0x10);
	if(!wait_ospi_cmd())
		goto EXIT;
    // [5:4] = oSPI_RdMUX[1:0] = 0~3: oSpi_SingleWr_Data, Status/RdData/DID[7:0], DID[15:8], MID
	*states = IC_ReadByte(TWIC_P6, OSPI_WR_DATA_REG);	// register value
    rc = 1;
 EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, OSPI_INSTRUCTION_REG, FAST_READ_4X);
    ospi_toggle_byte_set();
    return rc;
}
/*
 * 摘要 :bit spi_erase_sector(unsigned long address)
 *
 * 描述 :擦除FLASH(以4K为单位)
 *
 * 参数 :unsigned long address =>FLASH位置
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
bit spi_erase_sector(unsigned long address)
{
	bit rc = 0;
	/* WRSR */
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
	if(!spi_wrsr(NO_PROTECT,NO_PROTECT | QUAD_ENABLE_WIN))
		goto EXIT;
#else
	if(!spi_wrsr(NO_PROTECT | QUAD_ENABLE,0x00))
		goto EXIT;
#endif

	/* SPI PRE WREN */
	//IC_WritByte(TWIC_P6, SPI_4X_ENABLE_REG], (IC_ReadByte(TWIC_P6,SPI_4X_ENABLE_REG)|SPI_PRE_WREN));
	
    if(!spi_wren())
        goto EXIT;
    
	/* SE */
	spi_check_shadow();
	//CONFIG_REGS[TWIC_P0|SHADOW_CTRL_CONF_REG]=0x00;
	IC_WritByte(TWIC_P0,0xE2,0x00);		
	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, SECTOR_ERASE);

	/* NOR FLASH SECTOR ADDRESS */
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, address&0xFF);
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, (address>>8)&0xFF);
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, (address>>16)&0xFF);
	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, SE_ERASE);
	if(!wait_cmd())
        goto EXIT;
    
    rc = 1;
EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X); 
    return rc;
}
/*
 * 摘要 :bit spi_dma_read(unsigned long address, unsigned short xaddr, unsigned short length)
 *
 * 描述 :读取FLASH内的值透过DMA到XRAM
 *
 * 参数 :unsigned long address  =>FLASH位置
 *       unsigned short xaddr   =>XRAM位置
 *       unsigned short length  =>搬运资料大小
 *
 * 回传值 : 0 = 失败 1= 成功
 *
 * 注意 : 无
*/
#define DMA_MAX_LENGTH	0x2000
/* --- Read SPI Flash ROM via DMA moving to XDATA (0x0000~0x0400) --- */
bit spi_dma_read(unsigned long address, unsigned short xaddr, unsigned short length)
{
    bit rc = 0;
	
	if(length > DMA_MAX_LENGTH)
    {
    	ERROR(("spi_dma_read length > 8192 bytes\r\n"));
        goto EXIT;
    }
	IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ);				// FR command
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (address&0x000000FF));		// AL
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((address>>8)&0x000000FF));	// AM
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((address>>16)&0x000000FF));	// AH
	IC_WritByte(TWIC_P6, SPI_COMMAND_REG, READ_DMA);					// Control w/ Check WIP
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG1, xaddr&0x00FF);				// External RAM address
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG2, (xaddr>>8)&0x00FF);			// External RAM address
	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG1, length&0x00FF);			// DMA count
	IC_WritByte(TWIC_P6, SPI_DMA_CTRL_REG, DMA_READ|((length&SPI_DMA_COUNT_BIT)>>8));	// DMA go
    if((IC_ReadByte(TWIC_P6, 0x14) & 0x40))// dma interrupt mask
    {
       	if(!wait_dma())
	    	goto EXIT;
    }
    else// dma interrupt unmask
    {
        if(!wait_dma_int())
		    goto EXIT;
    }

    rc = 1;	
EXIT:
    if(SPI_HP_MODE_USE)
        IC_WritByte(TWIC_P6, SPI_INSTRUCTION_REG, FAST_READ_4X);
	return rc;
}
