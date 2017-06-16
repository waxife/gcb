/**
 *  @file   osd2api.c
 *  @brief  terawins OSD2 control function
 *  $Id: osd2api.c,v 1.25 2013/08/01 05:22:09 lym Exp $
 *  $Author: lym $
 *  $Revision: 1.25 $
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
#include "spirw.h"
#include "osd2api.h"  
#include "tw_widget_sys.h"
#include "tw_irq.h"
#include "cq.h"

#define USE_CQ_WRITE
#define FIX_FOR_GLOBAL_SHADOW
//#define DEBUG_OSD2
//#define OSD_BENCHMARK


static unsigned char OSD2_IS_ENABLE= DISABLE;

/*
 * Synopsis     void osd_set_location_addr( unsigned char x,    
 *											unsigned char y,
 *											unsigned short address,
 *											unsigned char width);
 * Description  設定ORAM之寫入位置於某OSD2 Menu之座標位置.
 * Parameters   x		    - x 位置 
 *              y		    - y 位置
 *              address		- OSD2 Menu 位址
 *              width		- OSD2 Menu 寬度(tile數量)
 * Return       none
 */
void osd_set_location_addr(unsigned char x, unsigned char y, unsigned short address, unsigned char width)
{	
	address = address + ((width + 4) * y + x + 4);
												
	IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(address & 0xff));
	IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(address>>8));							  
#ifdef	DEBUG_OSD2	
    dbg(0, ("osd_set_location_addr %bu, %bu (0x%04hX) \n", x, y, address));
#endif
}

/*
 * Synopsis     unsigned char osd_cfg_read( unsigned char index );
 * Description  讀取OSD2設定暫存器.
 * Parameters   index		    - OSD2設定暫存器編號
 * Return       暫存器數值
 */
unsigned char osd_cfg_read(unsigned char index)
{
	IC_WRITBYTE(TWIC_P0, 0xA8, index);
	return IC_READBYTE(TWIC_P0, 0xA9);
}

/* config OSD register */

/*
 * Synopsis     osd_cfg_wr( unsigned char index, 
 *                          unsigned char dat);    
 * Description  寫入OSD2設定暫存器. 
 * Parameters   index		    - OSD2設定暫存器位置 
 *              dat		        - 數值
 * Return       none
 */
void osd_cfg_wr(unsigned char index , unsigned char dat)
{
	IC_WRITBYTE(TWIC_P0, 0xA8, index);
	IC_WRITBYTE(TWIC_P0, 0xA9, dat);
#ifdef	DEBUG_OSD2
	dbg(0, ("osd_cfg_wr 0x%bX, 0x%bX \n", index, dat));
#endif
}

/* set OSD RAM address port of starting Access */

/*
 * Synopsis     void osd_set_ram_addr( unsigned short address );    
 * Description  設定ORAM之寫入位置
 * Parameters   address		- ORAM位址
 * Return       none
 */
void osd_set_ram_addr(unsigned short address)
{
  	/* word access, LSB first, then MSB byte */
	IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(address & 0xff));
	IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(address>>8));

#ifdef	DEBUG_OSD2
	dbg(0, ("osd_set_ram_addr 0x%X \n", address) );
#endif
}

/* access(write) data 2 OSD RAM */

/*
 * Synopsis     void osd_set_ram_data(unsigned short wdata);   
 * Description  寫入ORAM
 * Parameters   wdata		-寫入數值(1 WORD)
 * Return       none
 */
void osd_set_ram_data(unsigned short wdata)
{
	/* word access, LSB first, then MSB byte */
	OSD_SET_RAM_DATA(wdata);
	//IC_WRITBYTE(TWIC_P0, 0xAB, (unsigned char)(wdata & 0xff));
  	//IC_WRITBYTE(TWIC_P0, 0xAB, (unsigned char)(wdata>>8));

#ifdef	DEBUG_OSD2
	dbg(0, ("osd_set_ram_data 0x%X \n", wdata));
#endif
}	   

/*
 * Synopsis     osd_set_ram_data_byte(unsigned char dat);   
 * Description  寫入ORAM
 * Parameters   dat		-寫入數值(1 BYTE)
 * Return       none
 */
void osd_set_ram_data_byte(unsigned char dat)
{			   
	IC_WRITBYTE(TWIC_P0, 0xAB, dat);

#ifdef	DEBUG_OSD2
	dbg(0, ("osd_set_ram_data_byte 0x%bX \n", dat));
#endif
}

/*
 * Synopsis     osd_oram_fill(unsigned short start_address, 
 *                            unsigned short length, 
 *                            unsigned short dat);   
 * Description  將某區段ORAM填入特定數值
 * Parameters   start_address	-起始位置
 *              length          -數量
 *              dat             -寫入數值(1 WORD)
 * Return       none
 */
void osd_oram_fill(unsigned short start_address, unsigned short length, unsigned short dat)
{								   
	unsigned short count = 0;	

	if(length > 0x4000)
	{
		ERROR (("osd_oram_fill: length too long.\n"));	
		return;
	}
	 
	OSD_CFG_WR(0x40, (dat & 0x00ff));	  	             //WriteData
	OSD_CFG_WR(0x41, ((dat & 0xff00)>>8));
	OSD_CFG_WR(0x42, (start_address & 0x00ff));		  	 //Start addr
	OSD_CFG_WR(0x43, ((start_address & 0xff00)>>8));  

	OSD_CFG_WR(0x44, (length & 0x00ff));
	OSD_CFG_WR(0x45, (((length & 0xff00)>>8) | 0x80));	 

	while(!(IC_READBYTE(TWIC_P0, 0xa9) & 0x80)) 
	{
		count++;
		if(count > 10240)
		{
			ERROR (("osd_oram_fill: time out.\n"));	
			return;
		}
	}
}

/*
 * Synopsis     osd_benchmark (void)   
 * Description  將ORAM填入測試資料
 * Return       none
 */

#ifdef OSD_BENCHMARK
#include <radio_page.h>
#define MENU_ORAM_START_ADDR	0x3750
#define ROW1_FONTS_BASE_ADDR	MENU_ORAM_START_ADDR+4
#define ROW_CHAR_NUM			0x36
#define ADDR(Y)					(ROW1_FONTS_BASE_ADDR + (Y*ROW_CHAR_NUM))
void osd_benchmark (void)
{
	unsigned char data i, j, x, y, dat;
    extern void radio_menu_draw (void);
	radio_menu_draw ();

	osd_enable();
	mute_tw_irq();
	DBG_PRINT ("OSD Benchmark ");

	/* clear OSD RAM */
	osd_oram_fill(MENU_ORAM_START_ADDR, 0x0438, 0x0000);
	for(i=0; i<20; i++) {
		OSD_SET_RAM_ADDR(MENU_ORAM_START_ADDR+(i*ROW_CHAR_NUM));
		if(i==0)
			OSD_SET_RAM_DATA(0x4002);
		else
			OSD_SET_RAM_DATA(0xE000);
		OSD_SET_RAM_DATA(0x6062);
		OSD_SET_RAM_DATA(0xD000);
		OSD_SET_RAM_DATA(0x0032);
	}

	for (i = 0; i < 100; i++) {
		dat = i % 2 == 0 ? 54 : 55;
		for (j = 0; j < 100; j++) {
			for (y = 0; y < 20; y++) {
				OSD_SET_RAM_ADDR (ADDR(y));
				for (x = 0; x < 49; x++) {
					OSD_SET_RAM_DATA(dat);
				}
			}
		}
        DBG_PRINT (".");
	}

	DBG_PRINT ("\n");
	restore_tw_irq();
    osd_disable();
}
#endif

/*
 * Synopsis     void osd_init()   
 * Description  OSD功能初始化
 * Return       none
 */
void osd_init()
{
#ifdef OSD_BENCHMARK
    osd_benchmark();
#endif
	/* OSD Disable */
    osd_disable();
	/* clear OSD RAM */
	osd_ram_clear();
}

/*
 * Synopsis     void osd_ram_clear()   
 * Description  清除ORAM資料
 * Return       none
 */
void osd_ram_clear()
{
	/* clear OSD RAM */
	osd_oram_fill(0x0000, 0x3FFF, 0x0000);
}

/*
 * Synopsis     void osd_menu1_enable(void) 
 * Description  顯示osd2 menu1
 * Return       none
 */
void osd_menu1_enable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;

	rc = cq_write_byte(TWIC_P0, 0xA8, 0x10);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x80);

	rc = cq_write_byte(TWIC_P0, 0xA8, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x80);
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));
#else
	OSD_CFG_WR(0x10,0x80);		// menu1 enable
	OSD_CFG_WR(0x00,0x80);		// OSD1 enable
	IC_WRITBYTE(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
	OSD2_IS_ENABLE= ENABLE;
}

/*
 * Synopsis     void osd_menu1_disable(void)  
 * Description  關閉osd2 menu1
 * Return       none
 */
void osd_menu1_disable(void)
{
#ifdef USE_CQ_WRITE	
	char rc=0;	
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x10);
		rc = cq_write_byte(TWIC_P0, 0xA9, 0x00);

	rc = cq_write_byte(TWIC_P0, 0xA8, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x00);
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));
#else
	OSD_CFG_WR(0x10,0x00);		// menu1 disable
	OSD_CFG_WR(0x00,0x00);		// OSD1 disable
	IC_WRITBYTE(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
	//OSD2_IS_ENABLE= DISABLE;
}

/*
 * Synopsis     void osd_menu2_enable(void)  
 * Description  顯示osd2 menu2
 * Return       none
 */
void osd_menu2_enable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;

	rc = cq_write_byte(TWIC_P0, 0xA8, 0x18);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x80);

	rc = cq_write_byte(TWIC_P0, 0xA8, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x80);
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));
#else
	OSD_CFG_WR(0x10,0x80);		// menu1 enable
	OSD_CFG_WR(0x00,0x80);		// OSD1 enable
	IC_WRITBYTE(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
	OSD2_IS_ENABLE= ENABLE;
}

/*
 * Synopsis     void osd_menu2_enable(void)  
 * Description  關閉osd2 menu2
 * Return       none
 */
void osd_menu2_disable(void)
{
#ifdef USE_CQ_WRITE	
	char rc=0;	

	rc = cq_write_byte(TWIC_P0, 0xA8, 0x18);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x00);

	rc = cq_write_byte(TWIC_P0, 0xA8, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x00);
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));
#else
	OSD_CFG_WR(0x10,0x00);		// menu1 disable
	OSD_CFG_WR(0x00,0x00);		// OSD1 disable
	IC_WRITBYTE(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
	//OSD2_IS_ENABLE= DISABLE;
}

/*
 * Synopsis     void osd_enable(void)  
 * Description  啟用osd2
 * Return       none
 */
void osd_enable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;	
	rc = cq_write_byte(TWIC_P0, 0xA8, 0x10);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x80);
	rc = cq_write_byte(TWIC_P0, 0xA8, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x80);
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));
#ifdef FIX_FOR_GLOBAL_SHADOW	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
#else
	OSD_CFG_WR(0x10,0x80);		// menu1 enable
	OSD_CFG_WR(0x00,0x80);		// OSD1 enable
	IC_WRITBYTE(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
	OSD2_IS_ENABLE= ENABLE;
}

/*
 * Synopsis     void osd_disable()  
 * Description  停用osd2
 * Return       none
 */
void osd_disable()
{
#ifdef USE_CQ_WRITE	
	char rc=0;	
	rc = cq_write_byte(TWIC_P0, 0xA8, 0x10);	// menu1
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA8, 0x18);	// menu2
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA8, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA9, 0x00);
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));
#else
	OSD_CFG_WR(0x10,0x00);		// menu1 disable
	OSD_CFG_WR(0x18,0x00);		// menu2 disable
	OSD_CFG_WR(0x00,0x00);		// OSD1 disable
	IC_WRITBYTE(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif
	OSD2_IS_ENABLE= DISABLE;
}

/*
 * Synopsis     void osd_update(void) 
 * Description  刷新osd2
 * Return       none
 */
void osd_update(void)
{
#ifdef USE_CQ_WRITE
	if(OSD2_IS_ENABLE) {
		if (cq_flush_vsync () < 0)
			ERROR (("cq_flush()\n"));
	} else {
		if (cq_flush_now () < 0)
			ERROR (("cq_flush()\n"));
	}
#ifdef FIX_FOR_GLOBAL_SHADOW	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif	
#else
	if(OSD2_IS_ENABLE)
		IC_WRITBYTE(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);		// Shadow Update	
#endif
}

/*
 * Synopsis     void osd_blink(unsigned char freq, 
 *                             unsigned char duty);
 * Description  設定osd2閃爍狀態
 * Parameters   freq	-頻率
 *              duty    -占空比 
 * Return       none
 */
void osd_blink(unsigned char freq, unsigned char duty)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	freq = ((freq<<2)&0x0C);
	duty = (duty&0x03);
	rc = cq_write_byte(TWIC_P0, 0xA8, 0x00);
	rc = cq_write_byte(TWIC_P0, 0xA9, (0x80|freq|duty));
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
#else
	freq = ((freq<<2)&0x0C);
	duty = (duty&0x03);
	osd_cfg_wr(0x00,(0x80|freq|duty));		// OSD1 enable
#endif
	osd_update();
}

/*
 * Synopsis     void osd_wr_lut_tbl(unsigned long base_address, 
 *                                  unsigned short length);
 * Description  寫入lutRAM
 * Parameters   base_address	-lut資料地址
 *              length          -lut資料長度 
 * Return       none
 */
#define LUT_XRAM_LENGTH		64
void osd_wr_lut_tbl(unsigned long base_address, unsigned short length)
{
#if 1
	unsigned char i= 0, j= 0;
	unsigned short count= 0;
	unsigned char xdata buf[LUT_XRAM_LENGTH];
	
	count= length/LUT_XRAM_LENGTH;
	
	OSD_CFG_WR(0x08, 0x00);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);
	
	for(i=0; i< count; i++) {
		spi_dma2xram((base_address+(i*LUT_XRAM_LENGTH)), 
				(unsigned short)buf, 
				LUT_XRAM_LENGTH);
		/* I2C burst write */
		for (j = 0; j<LUT_XRAM_LENGTH; j++) {		// OSD LUT table is 512 bytes
			IC_WRITBYTE(TWIC_P0, 0xA9, buf[j]);
		}
#ifdef DEBUG_OSD2		
		dbg(0, ("spi_dma2xram(%08lX, %08X, %08X)\n", 
				(base_address+(i*LUT_XRAM_LENGTH)), 
				(unsigned short)buf, 
				LUT_XRAM_LENGTH));
#endif
	}
	
	if((length%LUT_XRAM_LENGTH)) {
		spi_dma2xram((base_address+(i*LUT_XRAM_LENGTH)), 
				(unsigned short)buf, 
				(length%LUT_XRAM_LENGTH));
		
		/* I2C burst write */
		for (j = 0; j<(length%LUT_XRAM_LENGTH); j++) {		// OSD LUT table is 512 bytes
			IC_WRITBYTE(TWIC_P0, 0xA9, buf[j]);
		}
			
#ifdef DEBUG_OSD2		
		dbg(0, ("spi_dma2xram(%08lX, %08X, %08X)\n", 
				(base_address+(i*LUT_XRAM_LENGTH)), 
				(unsigned short)buff, 
				(length%LUT_XRAM_LENGTH)));
#endif
	}

#else
	unsigned short i=0;

	if(length==0)
		return;
	
	OSD_CFG_WR(0x08, 0x00);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);

#ifdef	DEBUG_OSD2	
	dbg(0, ("osd_wr_lut_tbl(%lX, %u) \n", base_address, length));
#endif
	/* I2C burst write */
	for (i = 0; i<length; i++) {		// OSD LUT table is 512 bytes
		IC_WRITBYTE(TWIC_P0, 0xA9, spi_read_byte(base_address+i));
	}
#endif
}

/*
 * Synopsis     void osd_wr_2bp_lut_tbl(unsigned long base_address, 
 *                                      unsigned short length);
 * Description  寫入2bp lut映射表
 * Parameters   base_address	-映射資料地址
 *              length          -映射資料長度 
 * Return       none
 */
void osd_wr_2bp_lut_tbl(unsigned long base_address, unsigned char length)
{
#if 1
	unsigned char i= 0;
	unsigned char xdata buf[64];
	
	if(length==0)
		return;
	if(length>64)
		length= 64;
	
	spi_dma2xram((base_address), (unsigned short)buf, length);
	
	OSD_CFG_WR(0x08, 0x90);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);

#ifdef	DEBUG_OSD2	
   	dbg(0, ("osd_wr_2bp_lut_tbl(%lX, %bu) \n", base_address, length));
#endif
	
	/* I2C burst write */
	for (i = 0; i<length; i++) {		// OSD C2BP remap table is 64 bytes
		IC_WRITBYTE(TWIC_P0, 0xA9, buf[i]);
#ifdef	DEBUG_OSD2
		dbg(0, ("2BP REMAP[%bu] (%bu) \n", i, buf[i]));
#endif
	}

#else
	unsigned char i=0;

	if(length==0)
		return;
	
	OSD_CFG_WR(0x08, 0x90);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);

#ifdef	DEBUG_OSD2	
   	dbg(0, ("osd_wr_2bp_lut_tbl(%lX, %bu) \n", base_address, length));
#endif
	/* I2C burst write */
	for (i = 0; i<length; i++) {		// OSD C2BP remap table is 64 bytes
		IC_WRITBYTE(TWIC_P0, 0xA9, spi_read_byte(base_address+i));
#ifdef	DEBUG_OSD2
		dbg(0, ("2BP REMAP[%bu] (%bu) \n", i, spi_read_byte(base_address+i)));
#endif
	}
#endif
}

/*
 * Synopsis     void osd_wr_bg_lut_tbl(unsigned long base_address, 
 *                                unsigned short length);
 * Description  寫入1bp lut映射表
 * Parameters   base_address	-映射資料地址
 *              length          -映射資料長度 
 * Return       none
 */
void osd_wr_bg_lut_tbl(unsigned long base_address, unsigned char length)
{
#if 1
	unsigned char i= 0;
	unsigned char xdata buf[6];
	
	if(length==0)
		return;
	if(length>6)
		length= 6;
	
	spi_dma2xram((base_address), (unsigned short)buf, length);
	
	OSD_CFG_WR(0x08, 0x98);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);

#ifdef	DEBUG_OSD2	
   	dbg(0, ("osd_wr_bg_lut_tbl(%lX, %bu) \n", base_address, length));
#endif
	
	/* I2C burst write */
	for (i = 0; i<length; i++) {	// OSD background table is 6 bytes
		IC_WRITBYTE(TWIC_P0, 0xA9, buf[i]);
#ifdef	DEBUG_OSD2
		dbg(0, ("BG REMAP[%bu] (%bu) \n", i, buf[i]));
#endif
	}

#else
	unsigned char i=0;

	if(length==0)
		return;
	
	OSD_CFG_WR(0x08, 0x98);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);		


#ifdef	DEBUG_OSD2			 
	dbg(0, ("osd_wr_bg_lut_tbl(%lX, %bu) \n", base_address, length));
#endif	
	/* I2C burst write */
	for (i = 0; i<length; i++)		// OSD background table is 6 bytes
		IC_WRITBYTE(TWIC_P0, 0xA9, spi_read_byte(base_address+i));
#endif
}

/*
 * Synopsis     void osd_wr_regs_tbl(unsigned long base_address, 
 *                                   unsigned short length);
 * Description  寫入OSD2設定暫存器之資料表
 * Parameters   base_address	-映射資料地址
 *              length          -映射資料長度 
 * Return       none
 */
void osd_wr_regs_tbl(unsigned long base_address, unsigned char length)
{
	unsigned char i=0;
	unsigned char p_tbl[66];
	
	spi_dma2xram(base_address, (unsigned short)p_tbl, length);

#ifdef	DEBUG_OSD2		
    dbg(0, ("osd_wr_regs_tbl(%lX, %bu) \n", base_address, length));
#endif	
	
	for(i=0; i<length; i+=2)   {
		OSD_CFG_WR( p_tbl[i], p_tbl[i+1]);	
#ifdef	DEBUG_OSD2		
		dbg(0, ("p_tbl[i] = %02bX, p_tbl[i+1] = %02bX \n", p_tbl[i], p_tbl[i+1]));
#endif
	}	  
}

/*
 * Synopsis     osd_wr_menu_start_end( unsigned char menu_idx,
 *                                     unsigned short menu_start_address, 
 *                                     unsigned short menu_end_address);
 * Description  設定OSD2 menu於ORAM之位置
 * Parameters   menu_idx	        - 1: menu1, 2: menu2
 *              menu_start_address  - menu起始位置
 *              menu_end_address    - menu結束位置
 * Return       none
 */
void osd_wr_menu_start_end(unsigned char menu_idx,unsigned short menu_start_address, unsigned short menu_end_address)
{
#ifdef USE_CQ_WRITE
	char rc=0;

	if (menu_idx == 1)
	{
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x11);
		rc = cq_write_byte(TWIC_P0, 0xA9, (menu_start_address & 0xff));
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x12);
		rc = cq_write_byte(TWIC_P0, 0xA9, ((menu_start_address>>8) & 0xff));
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x13);
		rc = cq_write_byte(TWIC_P0, 0xA9, (menu_end_address & 0xff));
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x14);
		rc = cq_write_byte(TWIC_P0, 0xA9, ((menu_end_address>>8) & 0xff));
	}
	else if (menu_idx == 2)
	{
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x19);
		rc = cq_write_byte(TWIC_P0, 0xA9, (menu_start_address & 0xff));
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x1A);
		rc = cq_write_byte(TWIC_P0, 0xA9, ((menu_start_address>>8) & 0xff));
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x1B);
		rc = cq_write_byte(TWIC_P0, 0xA9, (menu_end_address & 0xff));
		rc = cq_write_byte(TWIC_P0, 0xA8, 0x1C);
		rc = cq_write_byte(TWIC_P0, 0xA9, ((menu_end_address>>8) & 0xff));
	}
	if(rc<0)	
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));

#ifdef FIX_FOR_GLOBAL_SHADOW	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);
#endif

#else
	if (menu_idx == 1)
	{
		OSD_CFG_WR( 0x11, (menu_start_address & 0xff));
		OSD_CFG_WR( 0x12, ((menu_start_address>>8) & 0xff));
		OSD_CFG_WR( 0x13, (menu_end_address & 0xff));
		OSD_CFG_WR( 0x14, ((menu_end_address>>8) & 0xff));
	}
	else if (menu_idx == 2)
	{
		OSD_CFG_WR( 0x19, (menu_start_address & 0xff));
		OSD_CFG_WR( 0x1a, ((menu_start_address>>8) & 0xff));
		OSD_CFG_WR( 0x1b, (menu_end_address & 0xff));
		OSD_CFG_WR( 0x1c, ((menu_end_address>>8) & 0xff));
	}
	
	if((IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_CTRL_CONF_DEFAULT))
		IC_WritByte(TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_CONF);

#endif
}

/*
 * Synopsis    void _osd_bmp_pos( unsigned short hstar, 
 *                                unsigned short vstar);
 * Description  設定BMP模式下OSD2顯示位置
 * Parameters   hstar	        - X軸起始位置
 *              vstar           - Y軸起始位置
 * Return       none
 */
void _osd_bmp_pos(unsigned short hstar, unsigned short vstar)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	/* Set BMP Horizintal Start Position */
	rc = cq_write_byte (TWIC_P0, 0xA8, 0x28);
	rc = cq_write_byte (TWIC_P0, 0xA9, (unsigned char)(hstar & 0xff));
	rc = cq_write_byte (TWIC_P0, 0xA8, 0x29);
	rc = cq_write_byte (TWIC_P0, 0xA9, (unsigned char)(hstar>>8));
	/* Set BMP Vertical Start Position */
	rc = cq_write_byte (TWIC_P0, 0xA8, 0x2A);
	rc = cq_write_byte (TWIC_P0, 0xA9, (unsigned char)(vstar & 0xff));
	rc = cq_write_byte (TWIC_P0, 0xA8, 0x2B);
	rc = cq_write_byte (TWIC_P0, 0xA9, (unsigned char)(vstar>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	/* Set BMP Horizintal Start Position */
	OSD_CFG_WR(0x28, (unsigned char)(hstar & 0xff));
	OSD_CFG_WR(0x29, (unsigned char)(hstar>>8));
	/* Set BMP Vertical Start Position */
	OSD_CFG_WR(0x2A, (unsigned char)(vstar & 0xff));
	OSD_CFG_WR(0x2B, (unsigned char)(vstar>>8));
#endif
}

/*
 * Synopsis    void osd_bmp_star_position( unsigned short hstar, 
 *                                         unsigned short vstar);
 * Description  設定BMP模式下OSD2顯示位置(並刷新)
 * Parameters   hstar	        - X軸起始位置
 *              vstar           - Y軸起始位置
 * Return       none
 */
void osd_bmp_star_position(unsigned short hstart, unsigned short vstart)
{
	_osd_bmp_pos(hstart, vstart);
	osd_update();
}

/*
 * Synopsis    void osd_bmp_disable(void) 
 * Description  關閉BMP模式
 * Return       none
 */
void osd_bmp_disable(void)
{
	OSD_CFG_WR(0x20,0x00); // disable BMP
}

/*
 * Synopsis    unsigned long _twf_get_bmp_offset ( unsigned long address, 
 *                                                 unsigned short uc); 
 * Description  回傳tof物件中某字元之位址
 * Parameters   address	        - tof物件位置
 *              uc              - utf-16字元
 * Return       字元位址
 */
unsigned long _twf_get_bmp_offset (unsigned long address, unsigned short uc)
{
    unsigned long gnum, i, j, fac;
	unsigned long bmpo;
    unsigned char c, gap; 
	unsigned short fcode, imin, imax, imid;
	unsigned char count = 50;
	
	/* 0x4f = charset address */				   
    c  = spi_read_byte(address + 0x4fL);
    gap = 6L;
    if (c == 0L) 
        gap = 5L;
 
    gnum = 0L;
    for(i=0L, fac=1L; i<4L; i++, fac*=256L) {	
		/* 0x58 = the total number of characters address */
        c = spi_read_byte(address + 0x58L + i);
        gnum+=c*fac;
    }	
			 
	imin = 1;
	imax = gnum;
		 
	while (imax >= imin && count--)
	{						  
		imid = (imin+imax)/2L; 
										   
		/* 0x5dL = unicode mapping table start address */
		fcode = spi_read_byte(address + 0x5dL + (imid-1L)*gap);	
		
		if(gap == 6)
			fcode += (spi_read_byte(address + 0x5eL + (imid-1L)*gap)<<8L);
		
		if(fcode <  uc) {	 
			imin = imid + 1L;
		}
		else if(fcode > uc ) {	 
			imax = imid - 1L;
		}
		else {
			break;
		}
	}	

	if(fcode != uc) {	
		return 0;
	}

    bmpo = 0;
    for(j=0L, fac=1L; j<4L; j++, fac*=256L) {    
        c = spi_read_byte(address + 0x5dL + (imid-1L)*gap + (gap - 4L) + j);  
        bmpo+=(c*fac);	 
    }	 
	   
    bmpo = 0x5dL + gnum*gap + bmpo; 
    bmpo += address;   
	     	
    return bmpo;   
}

/*
 * Synopsis    void osd_put_twf_char_to_oram ( unsigned long file_address, 
 *                                             unsigned short oram_address, 
 *                                             unsigned short uc,  
 *                                             unsigned short byte_size); 
 * Description  回傳tof物件中某字元之字型
 * Parameters   file_address	- tof物件位置
 *              oram_address    - oram位置
 *              uc              - utf-16字元
 *              byte_size       - 字型大小
 * Return       none
 */
unsigned char osd_put_twf_char_to_oram (unsigned long file_address, unsigned short oram_address, unsigned short uc,  unsigned short h_tile_byte_size, unsigned char isHSA)
{
	unsigned long bmpo;
	unsigned char w_count;
					   					  
	bmpo = _twf_get_bmp_offset(file_address, uc); 
	w_count = isHSA ? spi_read_byte(bmpo) : 1;

	if(bmpo == 0)
	{
		ERROR (("twf_put_char_to_oram: Unexpected character.\n"));
	}

	spi_dma2oram(bmpo+5, oram_address, h_tile_byte_size*w_count); 

	return w_count;
}

/*
 * Synopsis    void osd_set_access_mode_LSB(void);
 * Description  設定ORAM寫入模式為low byte only mode
 * Return       none
 */
void osd_set_access_mode_LSB(void) 
{ 
	OSD_CFG_WR(0x00, (osd_cfg_read(0x00) & 0xCF) | 0x20);
}

/*
 * Synopsis    void osd_set_access_mode_MSB(void);
 * Description  設定ORAM寫入模式為high byte only mode
 * Return       none
 */
void osd_set_access_mode_MSB(void) 
{			
	OSD_CFG_WR(0x00, osd_cfg_read(0x00) | 0x30);
}

/*
 * Synopsis    void osd_set_access_mode_word(void); 
 * Description  設定ORAM寫入模式為all byte
 * Return       none
 */
void osd_set_access_mode_word(void) 
{								 
	OSD_CFG_WR(0x00, osd_cfg_read(0x00) & 0xCF);
}

/*
 * Synopsis    void osd_menu_write_byte( unsigned char x, 
 *                                       unsigned char y, 
 *                                       unsigned char dat, 
 *                                       unsigned short menu_address, 
 *                                       unsigned char menu_width);
 * Description  寫入menu特定位置
 * Parameters   x	                - X軸位置(tile)
 *              y                   - Y軸位置(tile)
 *              dat                 - 數值
 *              menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 * Return       none
 */
void osd_menu_write_byte(unsigned char x, unsigned char y, unsigned char dat, unsigned short menu_address, unsigned char menu_width)
{							
	osd_set_location_addr(x, y, menu_address, menu_width);	
	osd_set_access_mode_LSB();
	OSD_SET_RAM_DATA_BYTE(dat);
	osd_set_access_mode_word();
}

/*
 * Synopsis    void osd_menu_write( unsigned char x, 
 *                                  unsigned char y, 
 *                                  unsigned short index, 
 *                                  unsigned char color, 
 *                                  unsigned short menu_address, 
 *                                  unsigned char menu_width);
 * Description  寫入menu特定位置
 * Parameters   x	                - X軸位置(tile)
 *              y                   - Y軸位置(tile)
 *              index               - 字型索引
 *              color               - 色盤索引
 *              menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 * Return       none
 */
void osd_menu_write(unsigned char x, unsigned char y, unsigned short index, unsigned char color, unsigned short menu_address, unsigned char menu_width)
{										   
	unsigned short dat = (color<<9) + index;
	osd_set_location_addr(x, y, menu_address, menu_width);	
    OSD_SET_RAM_DATA(dat);
}

/*
 * Synopsis    void osd_menu_write_line(unsigned char y, 
 *                                      unsigned short index, 
 *                                      unsigned char color, 
 *                                      unsigned short menu_address, 
 *                                      unsigned char menu_width);
 * Description  將字型寫入menu特定行
 *              y                   - Y軸位置(tile)
 *              index               - 字型索引
 *              color               - 色盤索引
 *              menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 * Return       none
 */
void osd_menu_write_line(unsigned char y, unsigned short index, unsigned char color, unsigned short menu_address, unsigned char menu_width)
{										   
	unsigned short dat = (color<<9) + index;
	
	osd_oram_fill((menu_address+((menu_width+4)*y)), menu_width, dat);
}

/*
 * Synopsis    void osd_menu_write_block(unsigned char x, 
 *                                       unsigned char y, 
 *                                       unsigned char width, 
 *                                       unsigned char height,
 *                                       unsigned char index, 
 *                                       unsigned char color, 
 *                                       unsigned short menu_address, 
 *                                       unsigned char menu_width)
 * Description  將字型寫入menu特定區塊
 * Parameters   x	                - X軸位置(tile)
 *              y                   - Y軸位置(tile)
 *              width               - 區塊寬度
 *              height              - 區塊高度
 *              index               - 字型索引
 *              color               - 色盤索引
 *              menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 * Return       none
 */
void osd_menu_write_block(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char index, unsigned char color, unsigned short menu_address, unsigned char menu_width)
{										   
	unsigned char w, h = 0;
	unsigned short dat = (color<<9) + index;
	
	for(h=0; h<height; h++) {
		osd_set_location_addr(x, y+h, menu_address, menu_width);
		for(w=0; w< width; w++) {
    		OSD_SET_RAM_DATA(dat);
		}
	}
}

/*
 * Synopsis    void osd_menu_start_h ( unsigned short x, 
 *                                     unsigned short address, 
 *                                     unsigned char width, 
 *                                     unsigned char row);
 * Description  設定menu x軸位置
 * Parameters   x	                - X軸位置
 *              address             - menu oram地址
 *              width               - menu寬度(tile)
 *              row                 - menu高度(tile)
 * Return       none
 */
#define OSD2_RATTV	0x4000
#define OSD2_RATTH	0x6000
void osd_menu_start_h (unsigned short x, unsigned short address, unsigned char width, unsigned char row)
{
	x &= 0x07FF;
	
	address += ((width+4)*row)+1;
	OSD_SET_RAM_ADDR(address);
	OSD_SET_RAM_DATA((OSD2_RATTH|x));
#ifdef	DEBUG_OSD2	
	dbg(0, ("osd_menu_location(%u, %u, 0x%X, %bu) 0x%X, 0x%X \n", x, y, address, width, row, (OSD2_RATTH|x)));
#endif
}

/*
 * Synopsis    void osd_menu_location ( unsigned short x, 
 *                                      unsigned short y, 
 *                                      unsigned short address, 
 *                                      unsigned char width, 
 *                                      unsigned char height);
 * Description  設定menu位置
 * Parameters   x	                - X軸位置
 *              y                   - Y軸位置
 *              address             - menu oram地址
 *              width               - menu寬度(tile)
 *              row                 - menu高度(tile)
 * Return       none
 */
void osd_menu_location (unsigned short x, unsigned short y, unsigned short address, unsigned char width, unsigned char height)
{
#if 1   // use CQ for image skew
	unsigned char i;

	x &= 0x07FF;
	y &= 0x07FF;
	
	for(i=0; i<height; i++) {
		cq_write_word (TWIC_P0, 0xAA, address, 0);
		if(i==0)
			cq_write_word (TWIC_P0, 0xAB, (OSD2_RATTV|y), 0);
		else
			cq_write_word (TWIC_P0, 0xAB, 0xE000, 0);
		cq_write_word (TWIC_P0, 0xAB, (OSD2_RATTH|x), 0);
#ifdef	DEBUG_OSD2	
		dbg(0, ("osd_menu_location(%u, %u, 0x%X, %bu, %bu) 0x%X, 0x%X \n", x, y, address, width, height, (OSD2_RATTV|y), (OSD2_RATTH|x)));
#endif
		address += (width + 4);
	}

	if (cq_flush_vde () < 0)
		ERROR (("cq_flush()\n"));
#else   // no use CQ for speed
	unsigned char i;

	x &= 0x07FF;
	y &= 0x07FF;
	
	for(i=0; i<height; i++) {
		OSD_SET_RAM_ADDR(address);
		if(i==0)
			OSD_SET_RAM_DATA((OSD2_RATTV|y));
		else
			OSD_SET_RAM_DATA(0xE000);
		OSD_SET_RAM_DATA((OSD2_RATTH|x));
#ifdef	DEBUG_OSD2	
		dbg(0, ("osd_menu_location(%u, %u, 0x%X, %bu, %bu) 0x%X, 0x%X \n", x, y, address, width, height, (OSD2_RATTV|y), (OSD2_RATTH|x)));
#endif
		address += (width + 4);
	}
#endif
}

/*
 * Synopsis    void osd_clear_char( unsigned char x, 
 *                                  unsigned char y, 
 *                                  unsigned short menu_address, 
 *                                  unsigned char menu_width);
 * Description  清除menu特定位置
 * Parameters   x	                - X軸位置(tile)
 *              y                   - Y軸位置(tile)
 *              menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 * Return       none
 */
void osd_clear_char(unsigned char x, unsigned char y, unsigned short menu_address, unsigned char menu_width)
{
	/* DEFALUT BLANK FONT INDEX IS 0 */
	osd_menu_write_byte(x, y, 0, menu_address, menu_width);
}

/*
 * Synopsis    void osd_clear_line(unsigned char y, 
 *                                 unsigned short menu_address, 
 *                                 unsigned char menu_width);
 * Description  清除menu特定行
 *              y                   - Y軸位置(tile)
 *              menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 * Return       none
 */
void osd_clear_line(unsigned char y, unsigned short menu_address, unsigned char menu_width)
{
	/* DEFALUT BLANK FONT INDEX IS 0 */
	//osd_menu_write_line(y, 0, 0, menu_address, menu_width);
	unsigned char i= 0;

	osd_set_location_addr(0, y, menu_address, menu_width);
	osd_set_access_mode_LSB();
	for(i=0; i<menu_width; i++)			
    	OSD_SET_RAM_DATA_BYTE(0);
	osd_set_access_mode_word();
}

/*
 * Synopsis    void osd_clear_block(unsigned char x, 
 *                                  unsigned char y, 
 *                                  unsigned char width, 
 *                                  unsigned char height,
 *                                  unsigned short menu_address, 
 *                                  unsigned char menu_width)
 * Description  清除menu特定區塊
 * Parameters   x	                - X軸位置(tile)
 *              y                   - Y軸位置(tile)
 *              width               - 區塊寬度
 *              height              - 區塊高度
 *              menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 * Return       none
 */
void osd_clear_block(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned short menu_address, unsigned char menu_width)
{
	unsigned char w, h = 0;

#ifdef VERSION_102	
	osd_set_access_mode_LSB();
	/* DEFALUT BLANK FONT INDEX IS 0 */
	for(h=0; h<height; h++) {
		osd_set_location_addr(x, y+h, menu_address, menu_width);
		for(w=0; w< width; w++) {
    		OSD_SET_RAM_DATA_BYTE(0);
		}
	}
	osd_set_access_mode_word();
#else
    /* add on version v1.04 for color&font index to clear */
	/* DEFALUT BLANK FONT INDEX IS 0x0000 */
	for(h=0; h<height; h++) {
		osd_set_location_addr(x, y+h, menu_address, menu_width);
		for(w=0; w< width; w++) {
			osd_set_ram_data(0);
		}
	}
#endif	
}

/*
 * Synopsis    void osd_clear_menu(unsigned short menu_address, 
 *                                 unsigned char menu_width
 *                                 unsigned char menu_height);
 * Description  清除menu
 * Parameters   menu_address        - menu地址
 *              menu_width          - menu寬度(tile)
 *              menu_height         - menu高度(tile)
 * Return       none
 */
void osd_clear_menu(unsigned short menu_address, unsigned char menu_width, unsigned char menu_height)
{
#ifdef VERSION_102
	unsigned char i= 0;

	for(i=0; i<menu_height; i++)	
		osd_clear_line(i, menu_address, menu_width);
#else
    /* add on version v1.04 */
	DBG_PRINT("not VERSION_102\n");

	DBG_PRINT("menu_address=%bd, menu_width=%bd, menu_height=%bd\r\n",menu_address, menu_width, menu_height);
	
    osd_clear_block(0, 0, menu_width, menu_height, menu_address, menu_width);
#endif		
}

/*
 * Synopsis    void osd_oram_dump( unsigned short start_address, 
 *                                 unsigned short length);
 * Description  讀取menu並打印
 * Parameters   start_address        - menu地址
 *              length               - 資料長度
 * Return       none
 */
void osd_oram_dump(unsigned short start_address, unsigned short length)
{		
	unsigned short count, idx, j, i;
	unsigned short read[8];
	unsigned short low, high;

	//IC_WritByte(TWIC_P0, 0xf0, 0x40);
	start_address = (start_address>>4) << 4;
								  
	DBG_PRINT("\n                 ", idx); 
	for(j = 0; j < 16; j++)
	{
		DBG_PRINT("  %1hXh ", j);	
	}							
	DBG_PRINT("\n", j);

	for( idx = start_address, i = 0; idx < start_address + length; idx+=8, i++)
	{
		IC_WritByte(TWIC_P0, 0xaa, (idx & 0xff));
		IC_WritByte(TWIC_P0, 0xaa, (idx & 0xff00)>>8);
		IC_WritByte(TWIC_P0, 0xa8, 0x48);
		IC_WritByte(TWIC_P0, 0xa9, 0x80);

		count = 0;
		while(!(IC_ReadByte(TWIC_P0, 0xaa) & 0x04)) 
		{
			count++;
			if(count == 10240)
			{
				ERROR (("osd_dump_memory: time out.\n"));	
				break;
			}
		}	

		for(j = 0; j < 8; j++)
		{
			low = IC_ReadByte(TWIC_P0, 0xab);
			high = IC_ReadByte(TWIC_P0, 0xab);	
			read[j] = low + (high<<8);	 	
		}
													
		if(i%2 == 0) 
		{				 
			DBG_PRINT("ORAM Dump %04hXh:  ", idx);
		}
		for(j = 0; j < 8; j++)
		{
			DBG_PRINT("%04hX ", read[j]);		
		} 
		if(i%2 == 1)
		{ 
			DBG_PRINT("\n");
		}
			
	}	
	//IC_WritByte(TWIC_P0, 0xf0, 0x50);
}

/*
 * Synopsis    void osd_wr_regs_tbl_nf( unsigned char code *p_tbl, 
 *                                      unsigned char length);
 * Description  exmcu寫入osd2設定表
 * Parameters   p_tbl                - 資料指標
 *              length               - 資料長度
 * Return       none
 */
#ifdef ENABLE_EX_MCU
void osd_wr_regs_tbl_nf(unsigned char code *p_tbl, unsigned char length)
{
	unsigned char i=0;
	//unsigned char p_tbl[66];
	
	for(i=0; i<length; i+=2)   {
		OSD_CFG_WR( p_tbl[i], p_tbl[i+1]);	
#ifdef	DEBUG_OSD2		
		dbg (0,("p_tbl[i] = %02bX, p_tbl[i+1] = %02bX \n", p_tbl[i], p_tbl[i+1]));
#endif
	}	  
}

/*
 * Synopsis    void osd_wr_lut_tbl_nf(unsigned char code *lut, 
 *                                    unsigned short length);
 * Description  exmcu寫入osd2 lut表
 * Parameters   lut                  - 資料指標
 *              length               - 資料長度
 * Return       none
 */
void osd_wr_lut_tbl_nf(unsigned char code *lut, unsigned short length)
{
	int i=0;

	if(length==0)
		return;
	
	OSD_CFG_WR(0x08, 0x00);	// OSD LUT address start point
	IC_WritByte(TWIC_P0, 0xA8, 0x09);

#ifdef	DEBUG_OSD2	
	dbg(0, ("osd_wr_lut_tbl(%X, %u) \n", (unsigned short)lut, length));
#endif
	/* I2C burst write */
	Burst_A_MCU(TWIC_P0, 0xA9);
	for (i=0; i<length; i++)
	{
		Burst_D_MCU(lut[i]);
#ifdef	DEBUG_OSD2		
		dbg(0,("lut[%d] = %02bX\n",i, lut[i]));
#endif
	}
	Burst_P_MCU();
}

/*
 * Synopsis    void osd_wr_2bp_lut_tbl_nf(unsigned char code *lut, 
 *                                        unsigned char length) 
 * Description  exmcu寫入osd2 2bp lut映射表
 * Parameters   lut                  - 資料指標
 *              length               - 資料長度
 * Return       none
 */
void osd_wr_2bp_lut_tbl_nf(unsigned char code *lut, unsigned char length)
{
	int i=0;

	if(length==0)
		return;
	
	OSD_CFG_WR(0x08, 0x90);	// OSD LUT address start point
	IC_WritByte(TWIC_P0, 0xA8, 0x09);

#ifdef	DEBUG_OSD2	
   	dbg(0, ("osd_wr_2bp_lut_tbl(%X, %bu) \n", (unsigned short)lut, length));
#endif
	/* I2C burst write */	
	Burst_A_MCU(TWIC_P0, 0xA9);
	for (i=0; i<length; i++)
	{
		Burst_D_MCU(lut[i]);
#ifdef	DEBUG_OSD2		
		dbg(0,("lut[%d] = %02bX\n",i, lut[i]));
#endif
	}
	Burst_P_MCU();
}

/*
 * Synopsis    void osd_wr_bg_lut_tbl_nf(unsigned char code *lut, 
 *                                        unsigned char length) 
 * Description  exmcu寫入osd2 1bp lut映射表
 * Parameters   lut                  - 資料指標
 *              length               - 資料長度
 * Return       none
 */
void osd_wr_bg_lut_tbl_nf(unsigned char code *lut, unsigned char length)
{
	unsigned char i=0;

	if(length==0)
		return;
	
	OSD_CFG_WR(0x08, 0x98);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);		


#ifdef	DEBUG_OSD2			 
	dbg(0, ("osd_wr_bg_lut_tbl(%X, %bu) \n", (unsigned short)lut, length));
#endif
	/* I2C burst write */	
	Burst_A_MCU(TWIC_P0, 0xA9);
	for (i=0; i<length; i++)
	{
		Burst_D_MCU(lut[i]);
#ifdef	DEBUG_OSD2		
		dbg(0,("lut[%d] = %02bX\n",i, lut[i]));
#endif
	}
	Burst_P_MCU();
}

#define SPARE_REG_LENGTH	8
#define SPARE_REG_ADDR		0xE0
#define PAGE1_REG_OFFSET	0xF100

/*
 * Synopsis    void osd_wr_regs_tbl_rb( unsigned long base_address, 
 *                                      unsigned char length);
 * Description  exmcu+nor flash寫入osd2設定表
 * Parameters   base_address         - flash資料位址
 *              length               - 資料長度
 * Return       none
 */
void osd_wr_regs_tbl_rb(unsigned long base_address, unsigned char length)
{
	unsigned char count= 0;
	unsigned char i= 0, j=0;

	if(length==0)
		return;

#ifdef	DEBUG_OSD2		
    dbg(0, ("osd_wr_regs_tbl_rb(%lX, %bu) \n", base_address, length));
#endif	
	
	count = (length/SPARE_REG_LENGTH);

	for(i=0; i< count; i++) {
		spi_dma2xram(base_address+(i*SPARE_REG_LENGTH), (PAGE1_REG_OFFSET|SPARE_REG_ADDR), SPARE_REG_LENGTH);
		for(j=0; j<SPARE_REG_LENGTH; j+=2) {
			OSD_CFG_WR(IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+j), IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+(j+1)));
#ifdef	DEBUG_OSD2		
			dbg(0, ("p_tbl[%bu] = %02bX, p_tbl[%bu] = %02bX \n", (j+(i*SPARE_REG_LENGTH)), IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+j), (j+1+(i*SPARE_REG_LENGTH)), IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+(j+1))));
#endif
		}
	}

	if((length%SPARE_REG_LENGTH)) {
		spi_dma2xram(base_address+(i*SPARE_REG_LENGTH), (PAGE1_REG_OFFSET|SPARE_REG_ADDR), (length%SPARE_REG_LENGTH));
		for(j=0; j<(length%SPARE_REG_LENGTH); j+=2) {
			OSD_CFG_WR(IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+j), IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+(j+1)));
#ifdef	DEBUG_OSD2		
			dbg(0, ("p_tbl[%bu] = %02bX, p_tbl[%bu] = %02bX \n", (j+(i*SPARE_REG_LENGTH)), IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+j), (j+1+(i*SPARE_REG_LENGTH)), IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+(j+1))));
#endif
		}
	}	  
}

/*
 * Synopsis    void osd_wr_lut_tbl_rb( unsigned long base_address, 
 *                                     unsigned char lut_addr, 
 *                                     unsigned short length); 
 * Description  exmcu+nor flash寫入lut表
 * Parameters   base_address         - flash資料位址
 *              lut_addr             - lut ram位址
 *              length               - 資料長度
 * Return       none
 */
void osd_wr_lut_tbl_rb(unsigned long base_address, unsigned char lut_addr, unsigned short length)
{
	unsigned char count= 0;
	unsigned char i= 0, j=0;

	if(length==0)
		return;

#ifdef	DEBUG_OSD2		
    dbg(0, ("osd_wr_lut_tbl_rb(0x%lX, 0x%bX, %bu) \n", base_address, lut_addr, length));
#endif	
	
	count = (length/SPARE_REG_LENGTH);

	OSD_CFG_WR(0x08, lut_addr);	// OSD LUT address start point
	IC_WRITBYTE(TWIC_P0, 0xA8, 0x09);
	
	for(i=0; i< count; i++) {
		spi_dma2xram(base_address+(i*SPARE_REG_LENGTH), (PAGE1_REG_OFFSET|SPARE_REG_ADDR), SPARE_REG_LENGTH);		
		for(j=0; j<SPARE_REG_LENGTH; j++) {
			IC_WRITBYTE(TWIC_P0, 0xA9, IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+j));
		}
	}

	if((length%SPARE_REG_LENGTH)) {
		spi_dma2xram(base_address+(i*SPARE_REG_LENGTH), (PAGE1_REG_OFFSET|SPARE_REG_ADDR), (length%SPARE_REG_LENGTH));
		for(j=0; j<(length%SPARE_REG_LENGTH); j++) {
			IC_WRITBYTE(TWIC_P0, 0xA9, IC_ReadByte(TWIC_P1, SPARE_REG_ADDR+j));
		}
	}
}

#endif
