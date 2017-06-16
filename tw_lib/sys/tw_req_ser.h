/**
 *  @file   tw_req_ser.h
 *  @brief  head file for tw_req_ser
 *  $Id: tw_req_ser.h,v 1.3 2013/02/05 11:57:00 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.3 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/11/06  ken 	New file.
 *
 */

#ifndef _TW_REQ_SER_H
#define _TW_REQ_SER_H

#define EXMCU_CMD_REG			0xEE

#define EXMCU_CMD_DMA			0x80
#define EXMCU_CMD_TWFONT		0x40
#define EXMCU_CMD_TWFONT_SCR	0x30
#define EXMCU_CMD_LUTS			0x20
#define EXMCU_CMD_OREG			0x10

#define TWF_MENU_INDEX		0xEC
#define TWF_SIZE_ID			0xED

#define LUT_MAIN_LEN_REG1	0xE1
#define LUT_MAIN_LEN_REG2	0xE2
#define LUT_2BP_LEN_REG		0xE3
#define LUT_BG_LEN_REG		0xE4

#define OREG_TABLES_LEN_REG		0xE1

/* TWFONT DEFINE */

#define I2C_CBUS_SLAVE_ADDR		TWIC_P0
#define I2C_CBUS_ADDR_REG1		0xAD
#define I2C_CBUS_ADDR_REG2		0xAE
#define I2C_CBUS_DATA_REG		0xAF

#define EX_TWF_BUFF_SIZE		0x100
#define EX_TWF_START_ADDR		0x100

#define	EX_TWF_X_BUF_ADDR			0	
#define	EX_TWF_Y_BUF_ADDR			1
#define	EX_TWF_STAR_INX_BUF_ADDR	2
//#define	EX_TWF_FONT_SIZE_BUF_ADDR	3
#define	EX_TWF_TXT_WIDTH_BUF_ADDR	3
#define	EX_TWF_TXT_HEIGHT_BUF_ADDR	4
//#define	EX_TWF_LINE_NUM_BUF_ADDR	4
#define	EX_TWF_COLOR_INX_BUF_ADDR	5
#define	EX_TWF_MENU_WIDTH_BUF_ADDR	6
#define EX_TWF_HEADER_SIZE			7

extern unsigned char req_ser_dma_go(void);
extern void req_ser_draw_twfont(struct tw_txt* ptxt, struct tw_tofs* tofs, unsigned char x, unsigned char y, unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color);
extern void req_ser_draw_twfont_scr(struct tw_txt* ptxt, struct tw_tofs* tofs, unsigned char x, unsigned char y, unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color);
extern void req_ser_load_luts(unsigned long base_addr, unsigned short main_ln, unsigned char r2bp_ln, unsigned char rbg_ln);
extern void req_ser_load_oregs(unsigned long base_addr, unsigned char length);
extern void req_ser_start(void);
extern void req_ser_init(void);

#endif
