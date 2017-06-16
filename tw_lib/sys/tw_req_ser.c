/**
 *  @file   tw_req_ser.c
 *  @brief  terawins of require service code
 *  $Id: tw_req_ser.c,v 1.5 2013/04/03 06:09:54 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.5 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/11/06  ken 	New file.
 *
 */

#include "sys.h"
#include "spirw.h"
#include "spiosd.h"
#include "osd2api.h"
#include "reg_tw.h"
#include "tw_irq.h"
#include "tw_ex_sys.h"

#include "tw_req_ser.h"

#ifdef ENABLE_EX_MCU
/* FOR EXMCU WITH NOR FLASH */
#ifdef RESOURCE_ON_NOR_FLASH

/*
 * Synopsis     unsigned char req_ser_dma_go(void);
 * Description  exMCU啟動t123 DMA.
 * Return       1 for in region.
 */
unsigned char req_ser_dma_go(void)
{
	unsigned char count= 0;
	unsigned char rc= 0;

	IC_WritByte(TWIC_P1, EXMCU_CMD_REG, 0x80);
	
	/* servcer to wake up */
	IC_WritByte(TWIC_P1, 0xFF, 0x00);
	
	//twdDelay(100);
	for(count=0; count<255; count++) {		
		twdDelay(100);
		rc= IC_ReadByte(TWIC_P1, EXMCU_CMD_REG);
		if(rc==0x80) {
			dbg(0, (">>>> i51 no ACK!\n\r"));	
		} else if(rc!=0xFF) {
			break;
		}
	}

	if(rc==0xAA) {
		dbg(3, (">>>> DMA Finish!\n\r"));
		return 1;
	} else if(rc==0x55) {
		ERROR(("DMA Fail!\n\r"));
		return 0;
	} else {
		ERROR(("unknow!\n\r"));
		return 0;
	}
}

/*
 * Synopsis     void req_ser_draw_twfont( struct tw_txt* ptxt, 
 *                                        struct tw_tofs* tofs, 
 *                                        unsigned char x, 
 *                                        unsigned char y, 
 *                                        unsigned short *ucs, 
 *                                        unsigned short menu_addr, 
 *                                        unsigned char menu_width, 
 *                                        unsigned char color);
 * Description  exMCU控制T123+nor flash繪製twfont.
 * Parameters   ptxt	    - txt物件指標
 *              tofs        - tof物件指標
 * 				x		    - x軸座標
 *              y		    - y軸座標
 *              ucs		    - 字串指標 
 *              menu_addr	- menu地址
 *              menu_width	- menu寬度
 *              color       - 色盤編號
 * Return       none.
 */
extern struct tw_menu* CUR_MENU_P;
void req_ser_draw_twfont(struct tw_txt* ptxt, struct tw_tofs* tofs, unsigned char x, unsigned char y, unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color)
{
	unsigned short count= 0;
	unsigned char rc= 0;
	unsigned short font_size;

	font_size = CUR_MENU_P->font_w*CUR_MENU_P->font_h/16;
	
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (tofs->base_addr&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((tofs->base_addr>>8)&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((tofs->base_addr>>16)&0x000000FF));
	
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG1, menu_addr&0x00FF);
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG2, (menu_addr>>8)&0x00FF);

	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG1, font_size & 0x00FF);
	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG2, 0x60|((font_size & SPI_DMA_COUNT_BIT)>>8));
	
	/* Setting Cbus Base Address */
	font_size= EX_TWF_START_ADDR;
	IC_WritByte(I2C_CBUS_SLAVE_ADDR, I2C_CBUS_ADDR_REG1, font_size & 0x00FF);
	IC_WritByte(I2C_CBUS_SLAVE_ADDR, I2C_CBUS_ADDR_REG2, (font_size>>8)&0x00FF);

	/* Write TWFONT data to XDATA by Cbus */	
	Burst_A_MCU(I2C_CBUS_SLAVE_ADDR, I2C_CBUS_DATA_REG);
	Burst_D_MCU(x);					// EX_TWF_X_BUF_ADDR
	Burst_D_MCU(y);					// EX_TWF_Y_BUF_ADDR
	Burst_D_MCU(ptxt->index);		// EX_TWF_STAR_INX_BUF_ADDR
	Burst_D_MCU(ptxt->width);		// EX_TWF_TXT_WIDTH_BUF_ADDR
	Burst_D_MCU(ptxt->height);		// EX_TWF_TXT_HEIGHT_BUF_ADDR
	Burst_D_MCU(color);				// EX_TWF_COLOR_INX_BUF_ADDR
	Burst_D_MCU(menu_width);		// EX_TWF_MENU_WIDTH_BUF_ADDR
	for(count=0; (count<(ptxt->width*ptxt->height))&&(ucs[count]!=0); count++) {
		if(count>=((EX_TWF_BUFF_SIZE-EX_TWF_HEADER_SIZE)/2)) {
			break;
		}
		Burst_D_MCU((ucs[count]>>8)&0x00FF);
		Burst_D_MCU(ucs[count]&0x00FF);	
	}
	Burst_D_MCU(0);
	Burst_D_MCU(0);
	Burst_P_MCU();

#if 0
	for(count=0; (count<(ptxt->width*ptxt->height))&&(ucs[count]!=0); count++) {
		if(count>=((EX_TWF_BUFF_SIZE-EX_TWF_HEADER_SIZE)/2)) {
			break;
		}
		INFO(("ucs[%u]= 0x%X\r\n", count, ucs[count]));	
	}
#endif

#ifdef	ENABLE_RECIVER_CPUINT
	mute_tw_irq();
#endif
	
	IC_WritByte(TWIC_P1, EXMCU_CMD_REG, EXMCU_CMD_TWFONT);
	
	/* servcer to wake up */
	IC_WritByte(TWIC_P1, 0xFF, 0x00);
	
	//twdDelay(100);
	for(count=0; count<255; count++) {		
		twdDelay(100);
		rc= IC_ReadByte(TWIC_P1, EXMCU_CMD_REG);
		if(rc==EXMCU_CMD_TWFONT) {
			dbg(0, (">>>> i51 no ACK!\n\r"));	
		} else if(rc!=0xFF) {
			break;
		}
	}

#ifdef	ENABLE_RECIVER_CPUINT
	restore_tw_irq();
#endif
	
	if(rc==0xAA) {
		dbg(3, (">>>> DMA Finish!\n\r"));
	} else if(rc==0x55) {
		ERROR(("DMA Fail!\n\r"));
	} else {
		ERROR(("unknow!\n\r"));
	}
}

/*
 * Synopsis     void req_ser_draw_twfont_scr( struct tw_txt* ptxt, 
 *                                            struct tw_tofs* tofs, 
 *                                            unsigned char x, 
 *                                            unsigned char y, 
 *                                            unsigned short *ucs, 
 *                                            unsigned short menu_addr, 
 *                                            unsigned char menu_width, 
 *                                            unsigned char color); 
 * Description  exMCU控制T123+nor flash繪製twfont. 				
 * Parameters   ptxt	    - txt物件指標
 *              tofs        - tof物件指標
 * 				x		    - x軸座標
 *              y		    - y軸座標
 *              ucs		    - 字串指標 
 *              menu_addr	- menu地址
 *              menu_width	- menu寬度
 *              color       - 色盤編號
 * Return       none.
 */
void req_ser_draw_twfont_scr(struct tw_txt* ptxt, struct tw_tofs* tofs, unsigned char x, unsigned char y, unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color)
{
	unsigned short count= 0;
	unsigned char rc= 0;
	unsigned short font_size;

	font_size = CUR_MENU_P->font_w*CUR_MENU_P->font_h/16;
	
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (tofs->base_addr&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((tofs->base_addr>>8)&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((tofs->base_addr>>16)&0x000000FF));
	
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG1, menu_addr&0x00FF);
	IC_WritByte(TWIC_P6, SPI_DMA_ADDR_REG2, (menu_addr>>8)&0x00FF);

	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG1, font_size & 0x00FF);
	IC_WritByte(TWIC_P6, SPI_DMA_COUNT_REG2, 0x60|((font_size & SPI_DMA_COUNT_BIT)>>8));
	
	/* Setting Cbus Base Address */
	font_size= EX_TWF_START_ADDR;
	IC_WritByte(I2C_CBUS_SLAVE_ADDR, I2C_CBUS_ADDR_REG1, font_size & 0x00FF);
	IC_WritByte(I2C_CBUS_SLAVE_ADDR, I2C_CBUS_ADDR_REG2, (font_size>>8)&0x00FF);

	/* Write TWFONT data to XDATA by Cbus */	
	Burst_A_MCU(I2C_CBUS_SLAVE_ADDR, I2C_CBUS_DATA_REG);
	Burst_D_MCU(x);					// EX_TWF_X_BUF_ADDR
	Burst_D_MCU(y);					// EX_TWF_Y_BUF_ADDR
	Burst_D_MCU(ptxt->index);		// EX_TWF_STAR_INX_BUF_ADDR
	Burst_D_MCU(ptxt->width);		// EX_TWF_TXT_WIDTH_BUF_ADDR
	Burst_D_MCU(ptxt->height);		// EX_TWF_TXT_HEIGHT_BUF_ADDR
	Burst_D_MCU(color);				// EX_TWF_COLOR_INX_BUF_ADDR
	Burst_D_MCU(menu_width);		// EX_TWF_MENU_WIDTH_BUF_ADDR
	for(count=0; (count<(ptxt->width*ptxt->height))&&(ucs[count]!=0); count++) {
		if(count>=((EX_TWF_BUFF_SIZE-EX_TWF_HEADER_SIZE)/2)) {
			break;
		}
		Burst_D_MCU((ucs[count]>>8)&0x00FF);
		Burst_D_MCU(ucs[count]&0x00FF);	
	}
	Burst_D_MCU(0);
	Burst_D_MCU(0);
	Burst_P_MCU();

#if 0
	for(count=0; (count<(ptxt->width*ptxt->height))&&(ucs[count]!=0); count++) {
		if(count>=((EX_TWF_BUFF_SIZE-EX_TWF_HEADER_SIZE)/2)) {
			break;
		}
		INFO(("ucs[%u]= 0x%X\r\n", count, ucs[count]));	
	}
#endif

#ifdef	ENABLE_RECIVER_CPUINT
	mute_tw_irq();
#endif
	
	IC_WritByte(TWIC_P1, EXMCU_CMD_REG, EXMCU_CMD_TWFONT_SCR);
	
	/* servcer to wake up */
	IC_WritByte(TWIC_P1, 0xFF, 0x00);
	
	//twdDelay(100);
	for(count=0; count<255; count++) {		
		twdDelay(100);
		rc= IC_ReadByte(TWIC_P1, EXMCU_CMD_REG);
		if(rc==EXMCU_CMD_TWFONT_SCR) {
			dbg(0, (">>>> i51 no ACK!\n\r"));	
		} else if(rc!=0xFF) {
			break;
		}
	}

#ifdef	ENABLE_RECIVER_CPUINT
	restore_tw_irq();
#endif
	
	if(rc==0xAA) {
		dbg(3, (">>>> DMA Finish!\n\r"));
	} else if(rc==0x55) {
		ERROR(("DMA Fail!\n\r"));
	} else {
		ERROR(("unknow!\n\r"));
	}
}

/*
 * Synopsis     void req_ser_load_luts( unsigned long base_addr, 
 *                                      unsigned short main_ln, 
 *                                      unsigned char r2bp_ln, 
 *                                      unsigned char rbg_ln);
 * Description  exMCU控制T123+nor flash載入lut表.
 * Parameters   base_addr	- lut表地址
 * 				main_ln		- lut資料長度
 * 				r2bp_ln		- 2bp lut映射表資料長度
 * 				rbg_ln  	- 1bp lut映射表資料長度
 * Return       none.
 */
void req_ser_load_luts(unsigned long base_addr, unsigned short main_ln, unsigned char r2bp_ln, unsigned char rbg_ln)
{
	unsigned char count= 0;
	unsigned char rc= 0;
	
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (base_addr&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((base_addr>>8)&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((base_addr>>16)&0x000000FF));
	
	IC_WritByte(TWIC_P1, LUT_MAIN_LEN_REG1, main_ln&0x00FF);
	IC_WritByte(TWIC_P1, LUT_MAIN_LEN_REG2, ((main_ln>>8)&0x00FF));

	IC_WritByte(TWIC_P1, LUT_2BP_LEN_REG, r2bp_ln);
	IC_WritByte(TWIC_P1, LUT_BG_LEN_REG, rbg_ln);

#ifdef	ENABLE_RECIVER_CPUINT
	mute_tw_irq();
#endif

	IC_WritByte(TWIC_P1, EXMCU_CMD_REG, EXMCU_CMD_LUTS);
	
	/* servcer to wake up */
	IC_WritByte(TWIC_P1, 0xFF, 0x00);

	//twdDelay(100);
	for(count=0; count<255; count++) {		
		twdDelay(100);
		rc= IC_ReadByte(TWIC_P1, EXMCU_CMD_REG);
		if(rc==EXMCU_CMD_LUTS) {
			dbg(0, (">>>> i51 no ACK!\n\r"));	
		} else if(rc!=0xFF) {
			break;
		}
	}

#ifdef	ENABLE_RECIVER_CPUINT
	restore_tw_irq();
#endif

	if(rc==0xAA) {
		dbg(3, (">>>> Load Finish!\n\r"));
	} else if(rc==0x55) {
		ERROR(("Load Fail!\n\r"));
	} else {
		ERROR(("unknow!\n\r"));
	}
}

/*
 * Synopsis     void req_ser_load_oregs( unsigned long base_addr, 
 *                                       unsigned char length); 
 * Description  exMCU控制T123+nor flash載入org表.
 * Parameters   base_addr	- org表地址
 * 				length		- 資料長度
 * Return       none.
 */
void req_ser_load_oregs(unsigned long base_addr, unsigned char length)
{
	unsigned char count= 0;
	unsigned char rc= 0;
	
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG1, (base_addr&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG2, ((base_addr>>8)&0x000000FF));
	IC_WritByte(TWIC_P6, SPI_ADDRESS_REG3, ((base_addr>>16)&0x000000FF));
	
	IC_WritByte(TWIC_P1, OREG_TABLES_LEN_REG, length);

#ifdef	ENABLE_RECIVER_CPUINT
	mute_tw_irq();
#endif
	
	IC_WritByte(TWIC_P1, EXMCU_CMD_REG, EXMCU_CMD_OREG);
	
	/* servcer to wake up */
	IC_WritByte(TWIC_P1, 0xFF, 0x00);
	
	//twdDelay(100);
	for(count=0; count<255; count++) {		
		twdDelay(100);
		rc= IC_ReadByte(TWIC_P1, EXMCU_CMD_REG);
		if(rc==EXMCU_CMD_OREG) {
			dbg(0, (">>>> i51 no ACK!\n\r"));	
		} else if(rc!=0xFF) {
			break;
		}
	}

#ifdef	ENABLE_RECIVER_CPUINT
	restore_tw_irq();
#endif
	
	if(rc==0xAA) {
		dbg(3, (">>>> Load Finish!\n\r"));
	} else if(rc==0x55) {
		ERROR(("Load Fail!\n\r"));
	} else {
		ERROR(("unknow!\n\r"));
	}
}

/*
 * Synopsis     void check_t123_status(void); 
 * Description  打印T123當前狀態. 
 * Return       none.
 */
void check_t123_status(void)
{
	unsigned char count= 0;	
	unsigned char rb_dat= 0;

	rb_dat= IC_ReadByte(TWIC_P1, 0xFE);
	if(rb_dat==0xFF) {	// T123 is unknow status
		for(count= 0; count<0xFF; count++) {
			rb_dat= IC_ReadByte(TWIC_P1, 0xFE);
			if(rb_dat!=0xFF)
				break;
		}
	}
	if(count==0xFF)
		ERROR(("T123 is unknow status!\n\r"));
}

/*
 * Synopsis     void req_ser_init(void);
 * Description  t123 i51 service code初始化.
 * Return       none.
 */
void req_ser_init(void)
{	
	check_t123_status();

	/* ENABLE i51 & RESET i51 */
	IC_WritByte(TWIC_P1,0xFE, 0x40);
	IC_WritByte(TWIC_P1,0xFD, 0x80);
	IC_WritByte(TWIC_P1,0xFF, (IC_ReadByte(TWIC_P1, 0xFF)&0x7F));
}

/*
 * Synopsis     void req_ser_start(void);
 * Description  啟動i51 service code.
 * Return       none.
 */
void req_ser_start(void)
{	
	unsigned char count= 0;

	check_t123_status();

	/* ENABLE i51 */
	IC_WritByte(TWIC_P1,0xFE, (IC_ReadByte(TWIC_P1, 0xFE)|0x80));

	if(!(IC_ReadByte(TWIC_P1, 0xFF)&0x80)) {	// T123 of i51 is unknow status
		for(count= 0; count<0xFF; count++) {
			if(IC_ReadByte(TWIC_P1, 0xFF)&0x80)
				break;
		}
	}
	if(count==0xFF)
		ERROR(("T123 of i51 is unknow status!\n\r"));
}

#endif	/* RESOURCE_ON_NOR_FLASH */
#endif	/* ENABLE_EX_MCU */
