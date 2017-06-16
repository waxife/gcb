/**
 *  @file   osd2api.h
 *  @brief  head file for osd2 api
 *  $Id: osd2api.h,v 1.18 2013/08/01 05:22:09 lym Exp $
 *  $Author: lym $
 *  $Revision: 1.18 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _OSD2API_H
#define _OSD2API_H

#include "config.h"

#ifdef ENABLE_EX_MCU

#define IC_WRITBYTE		IC_WritByte
#define IC_READBYTE 	IC_ReadByte

/* Speed-up Macro */
#define OSD_CFG_WR(INDEX, DATA) do { IC_WRITBYTE(TWIC_P0, 0xA8, INDEX); IC_WRITBYTE(TWIC_P0, 0xA9, DATA); } while (0)
#define OSD_SET_RAM_ADDR(ADDR) do {	IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(ADDR)); IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(ADDR >> 8)); } while (0)
#define OSD_SET_RAM_DATA(DATA) do { IC_WRITBYTE(TWIC_P0, 0xAB, (unsigned char)(DATA)); IC_WRITBYTE(TWIC_P0, 0xAB, (unsigned char)(DATA >> 8)); } while (0)
#define OSD_SET_RAM_DATA_BYTE(DATA) do { IC_WRITBYTE(TWIC_P0, 0xAB, DATA); } while (0)

#else
extern unsigned char xdata CONFIG_REGS[];

#define IC_WRITBYTE(PAGE, ADDR, VALUE)  CONFIG_REGS[PAGE + ADDR] = VALUE
#define IC_READBYTE(PAGE, ADDR)         CONFIG_REGS[PAGE + ADDR]

/* Speed-up Macro */
#define OSD_CFG_WR(INDEX, DATA) do { IC_WRITBYTE(TWIC_P0, 0xA8, INDEX); IC_WRITBYTE(TWIC_P0, 0xA9, DATA); } while (0)
#define OSD_SET_RAM_ADDR(ADDR) do {	IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(ADDR)); IC_WRITBYTE(TWIC_P0, 0xAA, (unsigned char)(ADDR >> 8)); } while (0)
#define OSD_SET_RAM_DATA(DATA) do { OSD_SET_RAM_DATA_BYTE(DATA); OSD_SET_RAM_DATA_BYTE((unsigned char)(DATA>>8)); } while (0)
#define OSD_SET_RAM_DATA_BYTE(DATA) do { unsigned char data i_tii=I51_W_ORAM_DELAY; IC_WRITBYTE(TWIC_P0, 0xAB, DATA); while(i_tii--);} while (0)
#endif

/* ADVANCE FUNCTION */
extern void osd_cfg_wr(unsigned char index, unsigned char dat);
extern unsigned char osd_cfg_read(unsigned char index);
extern void _osd_bmp_pos(unsigned short hstar, unsigned short vstar);	
extern void osd_oram_dump(unsigned short start_address, unsigned short length); 
extern void osd_oram_fill(unsigned short start_address, unsigned short length, unsigned short dat);
extern void osd_bmp_star_position(unsigned short hstart, unsigned short vstart);
extern void osd_bmp_disable(void);
extern void osd_set_location_addr(unsigned char x, unsigned char y, unsigned short address, unsigned char width);
extern void osd_cfg_wr(unsigned char index, unsigned char dat);
extern void osd_set_ram_addr(unsigned short address);
extern void osd_set_ram_data(unsigned short wdata);
extern void osd_set_ram_data_byte(unsigned char dat);
extern void osd_wr_lut_tbl_nf(unsigned char code *lut, unsigned short length);
extern void osd_wr_lut_tbl(unsigned long base_address, unsigned short length);
extern void osd_wr_lut_tbl_rb(unsigned long base_address, unsigned char lut_addr, unsigned short length);
extern void osd_wr_2bp_lut_tbl_nf(unsigned char code *lut, unsigned char length);
extern void osd_wr_2bp_lut_tbl(unsigned long base_address, unsigned char length);
extern void osd_wr_bg_lut_tbl(unsigned long base_address, unsigned char length);
extern void osd_wr_bg_lut_tbl_nf(unsigned char code *lut, unsigned char length);
extern void osd_wr_regs_tbl_nf(unsigned char code *p_tbl, unsigned char length);
extern void osd_wr_regs_tbl(unsigned long base_address, unsigned char length);
extern void osd_wr_regs_tbl_rb(unsigned long base_address, unsigned char length);
extern void osd_init();
extern void osd_ram_clear();
extern void osd_enable();
extern void osd_disable();
extern void osd_menu1_enable(void);
extern void osd_menu1_disable(void);
extern void osd_menu2_enable(void);
extern void osd_menu2_disable(void);
extern void osd_blink(unsigned char freq, unsigned char duty);
extern void osd_menu_write(unsigned char x, unsigned char y, unsigned short index, unsigned char color, unsigned short menu_address, unsigned char menu_width);
extern void osd_menu_write_line(unsigned char y, unsigned short index, unsigned char color, unsigned short menu_address, unsigned char menu_width);
extern void osd_menu_write_block(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char index, unsigned char color, unsigned short menu_address, unsigned char menu_width);
extern void osd_clear_char(unsigned char x, unsigned char y, unsigned short menu_address, unsigned char menu_width);
extern void osd_clear_line(unsigned char y, unsigned short menu_address, unsigned char menu_width);
extern void osd_clear_block(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned short menu_address, unsigned char menu_width);
extern void osd_clear_menu(unsigned short menu_address, unsigned char menu_width, unsigned char menu_height);
extern unsigned char osd_put_twf_char_to_oram (unsigned long file_address, unsigned short oram_address, unsigned short uc, unsigned short h_tile_byte_size, unsigned char isHSA);	
extern void osd_set_access_mode_LSB();
extern void osd_set_access_mode_MSB();
extern void osd_set_access_mode_word();
extern void osd_wr_menu_start_end(unsigned char menu_idx,unsigned short menu_start_address, unsigned short menu_end_address);

extern void osd_menu_location (unsigned short x, unsigned short y, unsigned short address, unsigned char width, unsigned char height);
extern void osd_menu_start_h (unsigned short x, unsigned short address, unsigned char width, unsigned char row);
	
#endif	/* _OSD2API_H */
