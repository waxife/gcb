/**
 *  @file   tw_ex_sys.c
 *  @brief  T123 OSD system for external MCU
 *  $Id: tw_ex_sys.c,v 1.30 2014/03/07 08:38:53 joec Exp $
 *  $Author: joec $
 *  $Revision: 1.30 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/11/06  ken 	New file.
 *
 */

#include "sys.h"
#include "tw_ex_sys.h"
#include "spirw.h"
#include "spiosd.h"
#include "osd2api.h"
#include "reg_tw.h"

#include "tw_req_ser.h"

//#define TW_EX_DEBUG_MODE

#ifdef ENABLE_EX_MCU

/* Status variable 
 * 		CUR_PAGE_ID		current of page ID
 *		CUR_IMG_P		current of image/twbc ID
 *		CUR_MENU_RES	current of OSD2 menu of resource
 *		CUR_TOF_RES		current of TOF	of resource
 *		CUR_BK_ID		current of Background (IMG/TWBC) of ID
 *		CUR_SPR_ID	current of Sprite of ID
 *		CUR_ICONSET_NUM		current of ICON SET of number
 */
static unsigned char CUR_ICONSET_NUM= 0;
#ifndef NO_SUPPORT_1BP_ROM_FONT
static unsigned char SUPPORT_1BP_ROM= 0;
#endif


/* FOR EXMCU WITH NOR FLASH */
#ifdef RESOURCE_ON_NOR_FLASH

struct tw_menu* CUR_MENU_P;
static struct tw_td* CUR_IMG_P;

#define CUR_MENU_ADDRESS	CUR_MENU_P->addr
#define CUR_MENU_WIDTH		CUR_MENU_P->width
#define CUR_MENU_FONT_W		CUR_MENU_P->font_w
#define CUR_MENU_FONT_H		CUR_MENU_P->font_h		

#define DMA_MAX_LENGTH	0x2000

/*
 * Synopsis     static void tw_fixed_menu2(void);
 * Description  avoided incorrect of display when OSD2 MENU-2 no assigned a space
 * Parameters   none
 * Return       none
 * NOTE         assigned 2 words space in last RAM for OSD2 MENU-2
 *              for exMCU+NOR of project 
 */
static void tw_fixed_menu2(void)
{
	osd_set_ram_addr(0x3FFE);
	osd_set_ram_data(0x0801);
	osd_set_ram_data(0x0000);	// 0 char
}

/*
 * Synopsis     void tw_load_menu_dma(unsigned long base_addr,
 *                                    unsigned short oram_addr,
 *                                    unsigned short length);
 * Description  loading for OSD2 of MEMU data by DMA                妏蚚DMA徭osd2 menu揃蹋
 * Parameters   base_addr	- source of base address                menu揃蹋華硊
 *              oram_addr	- assingned a OSD2 RAM of base address  menu oram 華硊
 *              length      - data of length                        menu 揃蹋墿僅
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_menu_dma(unsigned long base_addr, unsigned short oram_addr, unsigned short length)
{
	unsigned char i=0, count=0;
	
	count = (length/DMA_MAX_LENGTH);

#ifdef TW_EX_DEBUG_MODE
	dbg(0, (">>>> MENU LENGTH %u !\n", length));
#endif

    for(i=0; i< count; i++) {
		spi_dma2oram((base_addr+(i*DMA_MAX_LENGTH)), 
				(oram_addr+(i*DMA_MAX_LENGTH/2)), 
				DMA_MAX_LENGTH);
#ifdef TW_EX_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(base_addr+(i*DMA_MAX_LENGTH)), 
				(oram_addr+(i*DMA_MAX_LENGTH/2)), 
				DMA_MAX_LENGTH));

		//osd_oram_dump((oram_addr+(i*DMA_MAX_LENGTH/2)), DMA_MAX_LENGTH/2);
#endif
	}
	if((length%DMA_MAX_LENGTH)) {
		spi_dma2oram((base_addr+(i*DMA_MAX_LENGTH)), 
				(oram_addr+(i*DMA_MAX_LENGTH/2)), 
				(length%DMA_MAX_LENGTH));
#ifdef TW_EX_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(base_addr+(i*DMA_MAX_LENGTH)), 
				(oram_addr+(i*DMA_MAX_LENGTH/2)), 
				(length%DMA_MAX_LENGTH)));
		//osd_oram_dump((oram_addr+(i*DMA_MAX_LENGTH/2)), (length%DMA_MAX_LENGTH)/2);
#endif	
	}
	
	tw_fixed_menu2();				    
}

/*
 * Synopsis     void tw_load_icon_dma(unsigned long base_addr,
 *                                    unsigned short *oaddr,
 *                                    unsigned short length);
 * Description  loading for OSD2 of ICON(Fonts) by DMA                                      妏蚚DMA徭tile
 * Parameters   base_addr	- source of base address                                        tile flash華硊
 *              *oaddr	    - point of OSD2 base address (load after add a size of ICON)    oram tile華硊(硌綒崝)
 *              length      - data of length                                                tile 揃蹋墿僅
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_icon_dma(unsigned long base_addr, unsigned short *oaddr, unsigned short length)
{
	unsigned char i= 0;
	unsigned short count= 0;

	count = (length/DMA_MAX_LENGTH);

    for(i=0; i< count; i++) {
		(*oaddr)+= (i==0?0:(DMA_MAX_LENGTH/2));
		spi_dma2oram((base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				DMA_MAX_LENGTH);
#ifdef TW_EX_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				DMA_MAX_LENGTH));
		//osd_oram_dump((*oaddr), DMA_MAX_LENGTH/2);
#endif
	}
	
	if((length%DMA_MAX_LENGTH)) {
		(*oaddr)+= (i==0?0:(DMA_MAX_LENGTH/2));
		spi_dma2oram((base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				(length%DMA_MAX_LENGTH));
		
#ifdef TW_EX_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				(length%DMA_MAX_LENGTH)));
		//osd_oram_dump((*oaddr), (length%DMA_MAX_LENGTH)/2);
#endif
		(*oaddr)+= ((length%DMA_MAX_LENGTH)/2);
	}
}

/*
 * Synopsis     void tw_load_luts_dma(unsigned long base_addr,
 *                                    unsigned short main_ln,
 *                                    unsigned char r2bp_ln,
 *                                    unsigned char rbg_ln);
 * Description  loading for OSD2 of LUT table by DMA    妏蚚DMA徭lut桶
 * Parameters   base_addr	- source of base address    lut flash弇硊
 *              main_ln	    - main LUT of length        lut揃蹋墿僅
 *              r2bp_ln     - 2BP remap of length       2bp lut茬扞桶揃蹋墿僅
 *              rbg_ln      - BG remap of length        1bp ltu茬扞桶揃蹋墿僅
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_luts_dma(unsigned long base_addr, unsigned short main_ln, unsigned char r2bp_ln, unsigned char rbg_ln)
{
	osd_wr_lut_tbl_rb(base_addr, 0x00, (main_ln*4));	
	osd_wr_lut_tbl_rb((base_addr+(main_ln*4)), 0x90, (r2bp_ln*1));	
	osd_wr_lut_tbl_rb((base_addr+(main_ln*4)+r2bp_ln), 0x98, (rbg_ln*1));       
}

/*
 * Synopsis     void tw_load_oregs_dma(unsigned long base_addr,
 *                                     unsigned short length);
 * Description  loading for OSD2 of registers table by DMA  妏蚚DMA徭org桶
 * Parameters   base_addr	- source of base address        org桶 flash弇硊
 *              length      - table of length               org桶 揃蹋墿僅
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_oregs_dma(unsigned long base_addr, unsigned char length)
{
	osd_wr_regs_tbl_rb(base_addr, length);

	SUPPORT_1BP_ROM= 0;

#ifndef NO_SUPPORT_1BP_ROM_FONT
	if(osd_cfg_read(OSD_PT_LUT_BASEADD)==0x01) {
		SUPPORT_1BP_ROM= 1;
		INFO(("Support 1BP/ROM Font version!\n"));
	}
#endif        
}

/*
 * Synopsis     void tw_load_oregs(struct tw_regs *pregs);
 * Description  loading for OSD2 of registers table by service  妏蚚service code徭org桶
 * Parameters   *pregs	- point of tw_regs of struct            org昜璃硌
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_oregs(struct tw_regs *pregs)
{
	//tw_load_oregs_dma(pregs->base_addr, pregs->length);
	req_ser_load_oregs(pregs->base_addr, pregs->length);

	SUPPORT_1BP_ROM= 0;

#ifndef NO_SUPPORT_1BP_ROM_FONT
	if(osd_cfg_read(OSD_PT_LUT_BASEADD)==0x01) {
		SUPPORT_1BP_ROM= 1;
		INFO(("Support 1BP/ROM Font version!\n"));
	}
#endif
}

/*
 * Synopsis     void tw_load_luts(struct tw_regs *pregs);
 * Description  loading for OSD2 of LUT table by service    妏蚚service code徭lut桶
 * Parameters   *pluts	- point of tw_luts of struct        lut昜璃硌
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_luts(struct tw_luts *pluts)
{
	//tw_load_luts_dma(pluts->base_addr, pluts->main_ln, pluts->re_2bp_ln, pluts->re_bg_ln);
	req_ser_load_luts(pluts->base_addr, pluts->main_ln, pluts->re_2bp_ln, pluts->re_bg_ln);
}

/*
 * Synopsis     void tw_load_tiles(struct tw_tiles *ptiles);
 * Description  loading for OSD2 of tiles(ICONs) table  妏蚚dma徭tile桶
 * Parameters   *ptiles	- point of tw_tiles of struct   tile昜璃硌
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_tiles(struct tw_tiles *ptiles)
{
	unsigned short oaddr= 0;
	unsigned short i= 0;
	struct tw_tiles *t_ptiles = ptiles;

	/* CHECK CHANGEABLE ICONS OF TABLE */
	if((ptiles->tables[0]==0)&&(ptiles->tables[1]==0)) {
		
		if((CUR_ICONSET_NUM>=ptiles->cnt)||(CUR_ICONSET_NUM<=0))
			CUR_ICONSET_NUM= 1;
		t_ptiles = (struct tw_tiles *) ptiles->tables[(CUR_ICONSET_NUM*2)];
	}

	/* Load icon from tile table */
	for(i=0; i<(t_ptiles->cnt*2); i+=2) {
		tw_load_icon_dma(t_ptiles->tables[i*2], &oaddr, t_ptiles->tables[(i*2)+1]);
	}
}

static unsigned char menu_string_start_index = 0;
/*
 * Synopsis     void tw_menu_draw(struct tw_menu *pmenu);
 * Description  load OSD2 data for show OSD2            徭osd2 menu鷂眈燊揃蹋
 * Parameters   *pmenu	- point of tw_menu of struct    osd2 menu昜璃硌
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              this function just load OSD2 data to OSD RAM,
 *				need disable OSD2 before, 
 *				and change to enable (sosd_enable) when load finish
 */
void tw_menu_draw(struct tw_menu *pmenu)
{
	CUR_MENU_ADDRESS= pmenu->addr;
	CUR_MENU_WIDTH= pmenu->width;
	
	CUR_MENU_P= pmenu;	

	tw_load_oregs(pmenu->regs);
	tw_load_luts(pmenu->luts);
	tw_load_tiles(pmenu->tiles);	
	tw_load_menu_dma(pmenu->base_addr, pmenu->addr, (((pmenu->width+4)*pmenu->height)*2));

	if(osd_cfg_read(OSD_PT_LUT_BASEADD)==0x01)
		tw_fixed_menu2();

	menu_string_start_index = 0;
}

/* TWFONT DRAW FUNCTION */
unsigned char _tw_is_half_alphabet()
{
	static unsigned char is_half_alphabet = 0xff; 

	if(is_half_alphabet == 0xff) {		
    	is_half_alphabet  = spi_read_byte(CUR_MENU_P->tofs->base_addr + 0x54L);
	}
	return is_half_alphabet;
}

unsigned char _tw_get_draw_len(unsigned short *ucs, unsigned char width)
{
	unsigned char offset, i;

	offset = 0;
	for(i=0; ucs[i]!=0; i++) {
		if(_tw_is_half_alphabet() && ucs[i] < 256)
			offset+=1;
		else {	
			offset+=2;
		}

		if(offset > width)
			break;
	}

	return i;	
}

void tw_menu_draw_string(unsigned char x, unsigned char y, const unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color)
{
	unsigned char i, x_offset;
	unsigned short font_size;

	font_size = CUR_MENU_P->tofs->font_w*CUR_MENU_P->tofs->font_w/16;
				
	x_offset = 0;						
   	for(i=0; ucs[i]!=0; i++) {	 
	    if(_tw_is_half_alphabet() && ucs[i] < 256) {
			osd_put_twf_char_to_oram(CUR_MENU_P->tofs->base_addr, font_size*(menu_string_start_index)/2, ucs[i], font_size, 0);  
			osd_menu_write(x+x_offset, y, menu_string_start_index , color, menu_addr, menu_width); 
			menu_string_start_index += 1;
			x_offset+=1;
		}
		else {																				   
			osd_put_twf_char_to_oram(CUR_MENU_P->tofs->base_addr, font_size*(menu_string_start_index)/2, ucs[i], font_size*2, 0); 
			osd_menu_write(x+x_offset, y, menu_string_start_index , color, menu_addr, menu_width); 
			osd_menu_write(x+x_offset+1, y, menu_string_start_index + 1, color, menu_addr, menu_width);
			menu_string_start_index += 2;
			x_offset+=2;
		}
	}	 
}

unsigned char tw_menu_draw_nstring(unsigned char x, unsigned char y, const unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color, unsigned char num)
{
	unsigned char i, x_offset;
	unsigned short font_size;

	font_size = CUR_MENU_P->tofs->font_w*CUR_MENU_P->tofs->font_w/16;
							
	x_offset = 0;	
   	for(i=0; (i<num)&&(ucs[i]!=0); i++) {    
		if(_tw_is_half_alphabet() && ucs[i] < 256) { 
			osd_put_twf_char_to_oram(CUR_MENU_P->tofs->base_addr, font_size*(menu_string_start_index)/2, ucs[i], font_size, 0); 
			osd_menu_write(x+x_offset, y, menu_string_start_index , color, menu_addr, menu_width); 
			menu_string_start_index += 1;
			x_offset+=1;
		}
		else {	   
			osd_put_twf_char_to_oram(CUR_MENU_P->tofs->base_addr, font_size*(menu_string_start_index)/2, ucs[i], font_size*2, 0); 
			osd_menu_write(x+x_offset, y, menu_string_start_index , color, menu_addr, menu_width); 
			osd_menu_write(x+x_offset+1, y, menu_string_start_index + 1, color, menu_addr, menu_width);
			menu_string_start_index += 2;
			x_offset+=2;
		}
	}	
	
	return i;	
}

void tw_menu_draw_string_rev_nums(unsigned char x, unsigned char y, const unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color)
{
	unsigned char i, x_offset;
	unsigned short font_size;

	font_size = CUR_MENU_P->tofs->font_w*CUR_MENU_P->tofs->font_w/16;
				
	x_offset = 0;						
   	for(i=0; ucs[i]!=0; i++) {
		if(ucs[i] < 58 && ucs[i] > 47) {	 	
			osd_menu_write(x+i*2, y, (ucs[i] - 48)*2 , color, menu_addr, menu_width); 
			osd_menu_write(x+i*2+1, y, (ucs[i] - 48)*2+1, color, menu_addr, menu_width);
		}
		else { 
		    if(_tw_is_half_alphabet() && ucs[i] < 256) { 
				osd_put_twf_char_to_oram(CUR_MENU_P->tofs->base_addr, font_size*(menu_string_start_index)/2, ucs[i], font_size, 0);
				osd_menu_write(x+x_offset, y, menu_string_start_index , color, menu_addr, menu_width); 
				menu_string_start_index += 1;
				x_offset+=1;
			}
			else {	
				osd_menu_write(x+x_offset, y, menu_string_start_index , color, menu_addr, menu_width); 	 
				osd_put_twf_char_to_oram(CUR_MENU_P->tofs->base_addr, font_size*(menu_string_start_index)/2, ucs[i], font_size*2, 0);
				osd_menu_write(x+x_offset+1, y, menu_string_start_index + 1, color, menu_addr, menu_width);
				menu_string_start_index += 2;
				x_offset+=2;
			}
		}
	}	 
}

void tw_menu_reset_index(void)
{
	menu_string_start_index = 0;
}

void tw_menu_reset_index_n(unsigned char idx)
{
	menu_string_start_index = idx;	
}

void tw_menu_reset_index_rev_nums(void)
{
	menu_string_start_index = 20;	
}

void tw_menu_nums_init(void)
{
	unsigned char i;
	unsigned short font_size;

	font_size = CUR_MENU_P->tofs->font_w*CUR_MENU_P->tofs->font_h/16;
							
   	for(i=0; i<10; i++) {	 
		osd_put_twf_char_to_oram(CUR_MENU_P->tofs->base_addr, font_size*i, i+48, font_size*2, 0); 
	}
	menu_string_start_index = 20;	 
	
}



#ifdef ENABLE_EXMCU_USE_SPIOSD
/*
 * Synopsis     void tw_img_draw_ex(unsigned long lut_addr,
 *                               unsigned long index_addr,
 *                               unsigned short line_jump,
 *                               unsigned short width,
 *                               unsigned short height,
 *                               unsigned short x,
 *                               unsigned short y);
 * Description  setting registers for show image (SPIOSD)   偞隅img鞞尨垀剒湔
 * Parameters   color_addr	- lut of base address           img伎屜 flash弇硊
 *              index_addr	- index of base address         img flash弇硊
 *              line_jump	- image of line store size      img 藩俴揃蹋墿僅
 *              width	    - image of width                img �僅
 *              height      - image of height               img 詢僅
 *              x	        - x of coordinate of display    x嵞釱
 *              y	        - y of coordinate of display    y嵞釱
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              this function just setting registers,
 *				if SPIOSD of status is disable, 
 *				need change to enable (sosd_enable)
 */
void tw_img_draw_ex(unsigned long lut_addr, unsigned long index_addr, unsigned short line_jump, unsigned short width, unsigned short height, unsigned short x, unsigned short y)
{
	_sosd_pLUT_addr(lut_addr);
    _sosd_img_addr(index_addr);
    _sosd_img_ls(line_jump);
    _sosd_img_window(width, height);
	_sosd_img_loca(x, y);
} 

/*
 * Synopsis     void tw_spr_draw(unsigned long lut_addr,
 *                               unsigned long index_addr,
 *                               unsigned short line_jump,
 *                               unsigned short width,
 *                               unsigned short height,
 *                               unsigned short x,
 *                               unsigned short y);
 * Description  setting registers for show sprite (SPIOSD)  偞隅鞞尨spr垀剒湔
 * Parameters   color_addr	- lut of base address           spr伎屜 flash弇硊
 *              index_addr	- index of base address         spr flash弇硊
 *              line_jump	- sprite of line store size     spr藩俴揃蹋講
 *              width	    - sprite of width               spr�僅
 *              height      - sprite of height              spr詢僅
 *              x	        - x of coordinate of display    x嵞釱
 *              y	        - y of coordinate of display    y嵞釱
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              this function just setting registers,
 *				if SPIOSD-Sprite of status is disable, 
 *				need change to enable (sosd_spr_enable)
 */
void tw_spr_draw_ex(unsigned long lut_addr, unsigned long index_addr, unsigned char line_jump, unsigned char width, unsigned short height, unsigned short x, unsigned short y)
{
    _sosd_sLUT_addr(lut_addr);
	_sosd_spr_addr(index_addr);
	_sosd_spr_ls(line_jump);
    _sosd_spr_window(width, height);
	_sosd_spr_loca(x, y); 
}

/*
 * Synopsis     void tw_td_draw_ex(unsigned long color_addr,
 *                                 unsigned long index_addr,
 *                                 unsigned short line_jump,
 *                                 unsigned short width,
 *                                 unsigned short height,
 *                                 unsigned short x,
 *                                 unsigned short y);
 * Description  setting registers for show twbc (SPIOSD)    偞隅鞞尨twbc垀剒湔
 * Parameters   color_addr	- color of base address         twbc伎屜 flash華硊
 *              index_addr	- index of base address         twbc flash華硊
 *              line_jump	- twbc of line store size       twbc藩俴揃蹋講
 *              width	    - twbc of width                 twbc�僅
 *              height      - twbc of height                twbc詢僅
 *              x	        - x of coordinate of display    x嵞釱
 *              y	        - y of coordinate of display    y嵞釱
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              this function just setting registers,
 *				if SPIOSD and SPIOSD-TD of status is disable, 
 *				need change to enable (sosd_td_enable, sosd_enable)
 */
void tw_td_draw_ex(unsigned long color_addr, unsigned long index_addr, unsigned short line_jump, unsigned short width, unsigned short height, unsigned short x, unsigned short y)
{
	_sosd_tdc_addr(color_addr);
    _sosd_img_addr(index_addr);
    _sosd_td_ls(line_jump);
	_sosd_tdc_h_size(line_jump);
    _sosd_td_window(width, height);
	_sosd_td_loca(x, y);
	_sosd_td_spilt_mode_en();
}

/*
 * Synopsis     void tw_img_draw(struct tw_img* pimg);
 * Description  setting registers for show image (SPIOSD)   偞隅鞞尨img垀剒湔
 * Parameters   pimg	- point of image of struct          img昜璃硌
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              this function just setting registers,
 *				if SPIOSD of status is disable, 
 *				need change to enable (sosd_enable)
 */
void tw_img_draw(struct tw_img *img)
{
	CUR_IMG_P = (struct tw_td *)img;
	
	tw_img_draw_ex(img->lut_addr, img->index_addr, img->line_jump, img->width, img->height, img->x, img->y);
}

/*
 * Synopsis     void tw_spr_draw(struct tw_spr* pspr);  
 * Description  setting registers for show sprite (SPIOSD)  偞隅鞞尨spr垀剒湔
 * Parameters   pspr	- point of sprite of struct         spr昜璃硌
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              this function just setting registers,
 *				if SPIOSD-Sprite of status is disable, 
 *				need change to enable (sosd_spr_enable)
 */
void tw_spr_draw(struct tw_spr *spr)
{
	tw_spr_draw_ex(spr->lut_addr, spr->index_addr, spr->line_jump, spr->width, spr->height, spr->x, spr->y);
}

/*
 * Synopsis     void tw_td_draw(struct tw_td* ptd);
 * Description  setting registers for show twbc (SPIOSD)    偞隅鞞尨td垀剒湔
 * Parameters   ptd	- point of twbc of struct               td昜璃硌
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              this function just setting registers,
 *				if SPIOSD and SPIOSD-TD of status is disable, 
 *				need change to enable (sosd_td_enable, sosd_enable)
 */
void tw_td_draw(struct tw_td *td)
{
	CUR_IMG_P = td;

	tw_td_draw_ex(td->color_addr, td->index_addr, td->line_jump, td->width, td->height, td->x, td->y);
}

/*
 * Synopsis     void tw_emu_trig(void);
 * Description  emulation trig     茦追emulation岈璃
 * Parameters   none
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              need load emulation (tw_load_emu) before
 */
void tw_emu_trig(void)
{
	IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_ENABLE);	// for SPIOSD shadow issue
	sosd_emu_trig();
}

/*
 * Synopsis     void tw_load_emu(unsigned long base_addr);
 * Description  setting registers for emulation                 偞隅emulation垀剒湔
 * Parameters   base_addr	- emulation script of base address  emulation flash弇硊
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_load_emu(unsigned long base_addr)
{
	tw_emu_exit();

	sosd_set_emu_base_addrss(base_addr);
	tw_emu_trig();
}

/*
 * Synopsis     void tw_wait_emu_done(void);
 * Description  waitting emulation script done  脹渾emulation岈璃俇傖
 * Parameters   none
 * Return       none
 * NOTE         for exMCU+NOR of project
 *              some emulation scripts was always run, ex. animation,
 *              these emulation should force exit (tw_emu_exit) by fw.
 */
void tw_wait_emu_done(void)
{
	sosd_wait_emu_done();
	IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_DISABLE);	// for SPIOSD shadow issue
}

/*
 * Synopsis     void tw_emu_exit(void);
 * Description  force exit from emulation script    �秶泐堤emulation岈璃
 * Parameters   none
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_emu_exit(void)
{
	sosd_emu_exit();
	IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_DISABLE);	// for SPIOSD shadow issue
}

/* Status variable 
 * 		SLIDE_LOCK		    slide of direction
 *		SLIDE_COLOR_ADDR	remember color of base address for performance
 *		SLIDE_IMG_ADDR		remember image of base address for performance
 */
static unsigned char SLIDE_LOCK= SLIDE_NONE;
static unsigned long SLIDE_COLOR_ADDR= 0;
static unsigned long SLIDE_IMG_ADDR= 0;

/*
 * Synopsis     unsigned char tw_img_slide(struct tw_layout* playout,
 *                                         unsigned char dir,
 *                                         unsigned long offset,
 *                                         unsigned char i,
 *                                         unsigned char j);
 * Description  image of slide effect                           img賑滲
 * Parameters   *playout	- point of tw_layout of struct      �醱票擁磐�硌
 * 				dir		    - slide of direction                賑源砃
 *              offset		- slide of offset                   賑講
 *              i		    - number of row in layout           �醱俴
 *              j		    - number of column in layout        �醱蹈
 * Return       0xFE: slide successful                          賑傖髡
 *              0xFF: slide error                               賑囮
 *              other value: current stay of number in layout   醴鞞尨票擁昜璃��
 * NOTE         for exMCU+NOR of project
 */
unsigned char tw_img_slide(struct tw_layout* playout, unsigned char dir, unsigned long offset, unsigned char i, unsigned char j)
{
	unsigned char c_w= 0, c_h= 0;
	unsigned long shift= 0;
	struct tw_img			*pimg;
	struct tw_layout_item	*playout_item;

	unsigned long temp= 0;

	//offset = (offset/TD_LINE_ALING)*TD_LINE_ALING;	// ALIGN 4
	if(offset<=0)
		return SLIDE_HV_ERROR;
    
    /* get first image base address */
	pimg = playout->group[i].item[0].tw_object;
    
    /* get current image base address */
	//_get_tdc_addr(&addr);
	if(SLIDE_COLOR_ADDR==0)
		SLIDE_COLOR_ADDR= pimg->lut_addr;
	if(SLIDE_IMG_ADDR==0) {
		SLIDE_IMG_ADDR= pimg->index_addr;
		/* disable sprite & td */
		sosd_td_disable();
		sosd_sp_disable();
    }
    
    /* calc current of cnt at group */
	shift = SLIDE_IMG_ADDR-pimg->index_addr;
	c_w = ((shift%(pimg->line_jump))/pimg->width);
	c_h = ((shift/(pimg->line_jump))/(pimg->height));	
	c_w = c_w+c_h*playout->group[i].width;
	
	/* get current of ID & Object */
	pimg = playout->group[i].item[c_w].tw_object;
	
	shift = SLIDE_IMG_ADDR-pimg->index_addr;
    
	switch(dir) {
		case DIR_LEFT:	// left dir of slide
			/* check next image if current address == n image start address */
			if((SLIDE_IMG_ADDR==pimg->index_addr)) {
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				if(playout->group[i].item[j].right==NULL) {
					if((playout->group[i].item[j].jump==NULL)) {
						dbg(0, ("Can't left slide!\n"));
						return SLIDE_HV_ERROR;
					} else {
 						playout_item = playout->group[i].item[j].jump;
						if(playout_item->right==NULL) {
							dbg(0, ("Can't left slide!\n"));
							return SLIDE_HV_ERROR;
						} else {
							/* Load sLUT */
							playout_item = 
								(struct tw_layout_item*)playout->group[i].item[j].jump;
							pimg = playout_item->tw_object;
							tw_img_draw(pimg);
							
							playout_item = 
								(struct tw_layout_item*)playout_item->right;
							pimg = playout_item->tw_object;							
							_sosd_sLUT_addr(pimg->lut_addr);
							_sosd_load_slut();
							_sosd_swtc_offset(pimg->width);
							_sosd_swtc_h_mode();
							_sosd_swtc_enable();
							sosd_update();
							_sosd_swtc_offset(pimg->width);
						}
					}		
				} else {
					/* Load sLUT */
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].right;
					pimg = playout_item->tw_object;
					_sosd_sLUT_addr(pimg->lut_addr);
					_sosd_load_slut();					
					_sosd_swtc_h_mode();
					_sosd_swtc_enable();
					sosd_update();
					_sosd_swtc_offset(pimg->width);
				}
			} 
			/* next page */
			if((((shift)%(pimg->line_jump))+offset)
				>=pimg->width) {
				_sosd_swtc_disable();
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].right;
				pimg = playout_item->tw_object;
				tw_img_draw(pimg);
				sosd_enable(LOAD_LUT);
				CUR_IMG_P= (struct tw_td *) pimg;
				SLIDE_IMG_ADDR= pimg->index_addr;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* left slide */
			else {
				SLIDE_IMG_ADDR+= offset;
				_sosd_img_addr(SLIDE_IMG_ADDR);
				_get_swtc_offset((unsigned short*)&SLIDE_COLOR_ADDR);
				SLIDE_COLOR_ADDR >>= 16;
				SLIDE_COLOR_ADDR-= offset;
				//offset= SLIDE_COLOR_ADDR;
				//offset= ((IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG)&(~SOSD_SW_TC2_BIT)) | ((offset>>8)&SOSD_SW_TC2_BIT));
				sosd_update();
				//IC_WritByte(TWIC_P3, SOSD_SW_TC_REG1, (SLIDE_COLOR_ADDR&0x00FF));
				//IC_WritByte(TWIC_P3, SOSD_SW_TC_REG2, (offset&0x00FF));
				_sosd_swtc_offset(SLIDE_COLOR_ADDR);
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_RIGHT:	// right dir of slide
			/* check next image if current address == n image start address */
			if((SLIDE_IMG_ADDR==pimg->index_addr)) {
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				if(playout->group[i].item[j].left==NULL) {
					if((playout->group[i].item[j].jump==NULL)) {
						dbg(0, ("Can't left slide!\n"));
						return SLIDE_HV_ERROR;
					} else {
						playout_item = 
							(struct tw_layout_item*)playout->group[i].item[j].jump;
						if(playout_item->left==NULL) {
							dbg(0, ("Can't left slide!\n"));
							return SLIDE_HV_ERROR;
						} else {
							/* Load sLUT */
							playout_item = 
								(struct tw_layout_item*)playout->group[i].item[j].jump;
							pimg = playout_item->tw_object;
							_sosd_sLUT_addr(pimg->lut_addr);
							tw_img_draw(pimg);
							playout_item = 
								(struct tw_layout_item*)playout_item->left;
							pimg = playout_item->tw_object;
							_sosd_pLUT_addr(pimg->lut_addr);
							_sosd_load_plut();							
							_sosd_swtc_h_mode();
							_sosd_swtc_offset(0);
							
							_sosd_swtc_enable();
							sosd_pLUTsLUT_load();
							
							pimg = playout->group[i].item[c_w].tw_object;
						}
					}		
				} else {
					/* Load sLUT */
					_sosd_sLUT_addr(pimg->lut_addr);
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].left;
					pimg = playout_item->tw_object;
					_sosd_pLUT_addr(pimg->lut_addr);					
					_sosd_swtc_h_mode();
					_sosd_swtc_offset(0);
					
					_sosd_swtc_enable();
					sosd_pLUTsLUT_load();

					pimg = playout->group[i].item[c_w].tw_object;
				}
			} 
			/* next page */
			if((((shift)%(pimg->line_jump))<=offset)
				&&(SLIDE_IMG_ADDR!=pimg->index_addr)) {
				_sosd_swtc_disable();
				pimg = playout->group[i].item[c_w].tw_object;
				tw_img_draw(pimg);
				sosd_update();
				CUR_IMG_P= (struct tw_td *) pimg;
				SLIDE_IMG_ADDR= pimg->index_addr;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* right slide */
			else {
				SLIDE_IMG_ADDR-= offset;
				_sosd_img_addr(SLIDE_IMG_ADDR);
				_get_swtc_offset((unsigned short*)&SLIDE_COLOR_ADDR);
				SLIDE_COLOR_ADDR >>= 16;
				SLIDE_COLOR_ADDR+= offset;
				sosd_update();
				_sosd_swtc_offset(SLIDE_COLOR_ADDR);
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_UP:	// up dir of slide
			if(c_w!=j) {
				CUR_IMG_P= (struct tw_td *) pimg;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(SLIDE_IMG_ADDR==pimg->index_addr) {
				if(playout->group[i].item[j].down==NULL) {
					dbg(0, ("Can't UP slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;
					return SLIDE_HV_ERROR;		
				} else {
					/* Load sLUT */
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].down;
					pimg = playout_item->tw_object;
					_sosd_sLUT_addr(pimg->lut_addr);
					_sosd_load_slut();
					_sosd_swtc_v_mode();
					_sosd_swtc_enable();
					sosd_update();
					_sosd_swtc_offset(pimg->height+2);
				}
			}
			/* next page */
			if((((shift)/(pimg->line_jump))+offset)
				>=((pimg->height))) {
				_sosd_swtc_disable();
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].down;
				pimg = playout_item->tw_object;
				tw_img_draw(pimg);
				sosd_enable(LOAD_LUT);
				CUR_IMG_P= (struct tw_td *) pimg;
				SLIDE_IMG_ADDR= pimg->index_addr;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* up slide */
			else {
			    SLIDE_IMG_ADDR+= (offset*(pimg->line_jump));
				_sosd_img_addr(SLIDE_IMG_ADDR);
				_get_swtc_offset((unsigned short*)&SLIDE_COLOR_ADDR);
				SLIDE_COLOR_ADDR >>= 16;
				SLIDE_COLOR_ADDR-= offset;
				sosd_update();
				_sosd_swtc_offset(SLIDE_COLOR_ADDR);
				SLIDE_LOCK = SLIDE_V_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_DOWN:	// down dir of slide
			if(c_w!=j) {
				CUR_IMG_P= (struct tw_td *) pimg;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(SLIDE_IMG_ADDR==pimg->index_addr) {
				if(playout->group[i].item[j].up==NULL) {
					dbg(0, ("Can't down slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;	// slide init
					return SLIDE_HV_ERROR;
				} else {
					/* Load sLUT */
					_sosd_sLUT_addr(pimg->lut_addr);
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].up;
					pimg = playout_item->tw_object;
					_sosd_pLUT_addr(pimg->lut_addr);
					_sosd_swtc_v_mode();
					_sosd_swtc_offset(2);
					
					_sosd_swtc_enable();
					sosd_pLUTsLUT_load();

					pimg = playout->group[i].item[c_w].tw_object;
				}
			}
			/* next page */
			if(((((shift)/(pimg->line_jump))
				%(pimg->height))<=offset)&&(SLIDE_IMG_ADDR!=pimg->index_addr)) {
				_sosd_swtc_disable();
				tw_img_draw(pimg);
				sosd_update();
				CUR_IMG_P= (struct tw_td *) pimg;
				SLIDE_IMG_ADDR= pimg->index_addr;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			} 
			/* down slide */
			else {
				SLIDE_IMG_ADDR-= (offset*(pimg->line_jump));
				_sosd_img_addr(SLIDE_IMG_ADDR);
				_get_swtc_offset((unsigned short*)&SLIDE_COLOR_ADDR);
				SLIDE_COLOR_ADDR >>= 16;
				SLIDE_COLOR_ADDR+= offset;
				sosd_update();
				_sosd_swtc_offset(SLIDE_COLOR_ADDR);
				SLIDE_LOCK = SLIDE_V_DIR;
				return SLIDE_MOTION;
			}
			break;
	}
	return SLIDE_HV_ERROR;
}

/*
 * Synopsis     unsigned char tw_td_slide(struct tw_layout* playout,
 *                                        unsigned char dir,
 *                                        unsigned long offset,
 *                                        unsigned char i,
 *                                        unsigned char j);
 * Description  twbc of slide effect                            twbc賑虴彆滲
 * Parameters   *playout	- point of tw_layout of struct      �醱票擁磐�硌
 * 				dir		    - slide of direction                賑源砃
 *              offset		- slide of offset                   賑講
 *              i		    - number of row in layout           �醱俴
 *              j		    - number of column in layout        �醱蹈
 * Return       0xFE: slide successful                          賑傖髡
 *              0xFF: slide error                               賑囮
 *              other value: current stay of number in layout   醴鞞尨票擁昜璃��
 * NOTE         for exMCU+NOR of project
 */
#define TD_LINE_DUMMY	8
#define TD_LINE_ALING	4
unsigned char tw_td_slide(struct tw_layout* playout, unsigned char dir, unsigned long offset, unsigned char i, unsigned char j)
{	
	unsigned char c_w= 0, c_h= 0;
	unsigned long shift= 0;
	struct tw_td			*pimg;
	struct tw_layout_item	*playout_item;

	offset = (offset/TD_LINE_ALING)*TD_LINE_ALING;	// ALIGN 4
	if(offset<=0)
		return SLIDE_HV_ERROR;

	/* get first image base address */
	pimg = playout->group[i].item[0].tw_object;
	
	/* get current image base address */
	//_get_tdc_addr(&addr);
	if(SLIDE_COLOR_ADDR==0)
		SLIDE_COLOR_ADDR= pimg->color_addr;
	if(SLIDE_IMG_ADDR==0)
		SLIDE_IMG_ADDR= pimg->index_addr;	

	/* calc current of cnt at group */
	shift = SLIDE_COLOR_ADDR-pimg->color_addr;
	c_w = ((shift%(pimg->line_jump+TD_LINE_DUMMY))/pimg->width);
	c_h = ((shift/(pimg->line_jump+TD_LINE_DUMMY))/(pimg->height/TD_LINE_ALING));	
	c_w = c_w+c_h*playout->group[i].width;

	/* get current of ID & Object */
	pimg = playout->group[i].item[c_w].tw_object;

	switch(dir) {
		case DIR_LEFT:	// left dir of slide
			/* check next image if current address == n image start address */
			if((SLIDE_COLOR_ADDR==pimg->color_addr)) {
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				if(playout->group[i].item[j].right==NULL) {
					if((playout->group[i].item[j].jump==NULL)) {
						dbg(0, ("Can't left slide!\n"));
						return SLIDE_HV_ERROR;
					} else {
 						playout_item = playout->group[i].item[j].jump;
						if(playout_item->right==NULL) {
							dbg(0, ("Can't left slide!\n"));
							return SLIDE_HV_ERROR;
						}
					}		
				}
			}
			/* jump 2 next group if shift > linejump */
			if((shift%(pimg->line_jump+TD_LINE_DUMMY))
				>=(pimg->line_jump-pimg->width)) {
				if((playout->group[i].item[j].jump!=NULL)) {
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].jump;
					pimg = playout_item->tw_object;
					//tw_td_draw(pimg);
					SLIDE_COLOR_ADDR= pimg->color_addr+offset;
					_sosd_tdc_addr(SLIDE_COLOR_ADDR);
					SLIDE_IMG_ADDR= pimg->index_addr+(offset/TD_LINE_ALING);
					_sosd_img_addr(SLIDE_IMG_ADDR);
					sosd_update();
					CUR_IMG_P= pimg;
					return SLIDE_MOTION;
				} else {
					dbg(0, ("Can't jump!\n"));
					return SLIDE_HV_ERROR;
				}
			} 
			/* next page */
			else if((((SLIDE_COLOR_ADDR-pimg->color_addr)%(pimg->line_jump+TD_LINE_DUMMY))+offset)
				>=pimg->width) {
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].right;
				pimg = playout_item->tw_object;
				tw_td_draw(pimg);
				sosd_update();
				SLIDE_COLOR_ADDR= pimg->color_addr;
				SLIDE_IMG_ADDR= pimg->index_addr;
				CUR_IMG_P= pimg;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* left slide */
			else {
				SLIDE_COLOR_ADDR+= offset;
				_sosd_tdc_addr(SLIDE_COLOR_ADDR);
				SLIDE_IMG_ADDR+=(offset/TD_LINE_ALING);
				_sosd_img_addr(SLIDE_IMG_ADDR);
				sosd_update();
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_RIGHT:	// right dir of slide
			/* check next image if current address == n image start address */
			if((SLIDE_COLOR_ADDR==pimg->color_addr)) {
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				if(playout->group[i].item[j].left==NULL) {
					if((playout->group[i].item[j].jump==NULL)) {
						dbg(0, ("Can't left slide!\n"));
						return SLIDE_HV_ERROR;
					} else {
						playout_item = 
							(struct tw_layout_item*)playout->group[i].item[j].jump;
						if(playout_item->left==NULL) {
							dbg(0, ("Can't left slide!\n"));
							return SLIDE_HV_ERROR;
						}
					}		
				}
			}
			/* jump 2 next group if shift <= 0 */
			if((shift%(pimg->line_jump+TD_LINE_DUMMY))==0) {
				if((playout->group[i].item[j].jump!=NULL)) {
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].jump;
					pimg = playout_item->tw_object;
					//tw_td_draw(pimg);
					SLIDE_COLOR_ADDR= pimg->color_addr-offset;
					_sosd_tdc_addr(SLIDE_COLOR_ADDR);
					SLIDE_IMG_ADDR= pimg->index_addr-(offset/TD_LINE_ALING);
					_sosd_img_addr(SLIDE_IMG_ADDR);
					sosd_update();
					CUR_IMG_P= pimg;
					return SLIDE_MOTION;
				} else {
					dbg(0, ("Can't jump!\n"));
					return SLIDE_HV_ERROR;
				}
			} 
			/* next page */
			else if((((SLIDE_COLOR_ADDR-pimg->color_addr)%(pimg->line_jump+TD_LINE_DUMMY))<=offset)
				&&(SLIDE_COLOR_ADDR!=pimg->color_addr)) {
				pimg = playout->group[i].item[c_w].tw_object;
				tw_td_draw(pimg);
				sosd_update();
				SLIDE_COLOR_ADDR= pimg->color_addr;
				SLIDE_IMG_ADDR= pimg->index_addr;
				CUR_IMG_P= pimg;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* right slide */
			else {
				SLIDE_COLOR_ADDR-= offset;
				SLIDE_IMG_ADDR-= (offset/TD_LINE_ALING);				
				_sosd_tdc_addr(SLIDE_COLOR_ADDR);
				_sosd_img_addr(SLIDE_IMG_ADDR);
			    sosd_update();
				CUR_IMG_P= pimg;
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_UP:	// up dir of slide
			if(c_w!=j) {
				CUR_IMG_P= pimg;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(SLIDE_COLOR_ADDR==pimg->color_addr) {
				if(playout->group[i].item[j].down==NULL) {
					dbg(0, ("Can't UP slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;
					return SLIDE_HV_ERROR;		
				}
			}
			offset = offset/TD_LINE_ALING;
			/* next page */
			if((((SLIDE_COLOR_ADDR-pimg->color_addr)/(pimg->line_jump+TD_LINE_DUMMY))+offset)
				>=((pimg->height/TD_LINE_ALING))) {
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].down;
				pimg = playout_item->tw_object;
				tw_td_draw(pimg);
				sosd_update();
				SLIDE_COLOR_ADDR= pimg->color_addr;
				SLIDE_IMG_ADDR= pimg->index_addr;
				CUR_IMG_P= pimg;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* up slide */
			else {
				SLIDE_COLOR_ADDR+= (offset*(pimg->line_jump+TD_LINE_DUMMY));
				SLIDE_IMG_ADDR+= (offset*pimg->line_jump);
				_sosd_tdc_addr(SLIDE_COLOR_ADDR);
				_sosd_img_addr(SLIDE_IMG_ADDR);
				sosd_update();
				SLIDE_LOCK = SLIDE_V_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_DOWN:	// down dir of slide
			if(c_w!=j) {
				CUR_IMG_P= pimg;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(SLIDE_COLOR_ADDR==pimg->color_addr) {
				if(playout->group[i].item[j].up==NULL) {
					dbg(0, ("Can't down slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;	// slide init
					return SLIDE_HV_ERROR;
				}
			}
			offset = offset/TD_LINE_ALING;
			/* next page */
			if(((((SLIDE_COLOR_ADDR-pimg->color_addr)/(pimg->line_jump+TD_LINE_DUMMY))
				%(pimg->height/TD_LINE_ALING))<=offset)&&(SLIDE_COLOR_ADDR!=pimg->color_addr)) {
				_sosd_tdc_addr(pimg->color_addr);
				_sosd_img_addr(pimg->index_addr);
			    sosd_update();
				SLIDE_COLOR_ADDR= pimg->color_addr;
				SLIDE_IMG_ADDR= pimg->index_addr;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			} 
			/* down slide */
			else {
				SLIDE_COLOR_ADDR-= (offset*(pimg->line_jump+TD_LINE_DUMMY));
				SLIDE_IMG_ADDR-= (offset*pimg->line_jump);
				_sosd_tdc_addr(SLIDE_COLOR_ADDR);
				_sosd_img_addr(SLIDE_IMG_ADDR);
			    sosd_update();
				SLIDE_LOCK = SLIDE_V_DIR;
				return SLIDE_MOTION;
			}
			break;
	}
	return SLIDE_HV_ERROR;
}

/*
 * Synopsis     void tw_slide_init(void);   
 * Description  slide init      賑虴彆場宎趙
 * Parameters   none
 * Return       none
 * NOTE         for exMCU+NOR of project
 */
void tw_slide_init(void)
{
	SLIDE_LOCK = SLIDE_NONE;
	SLIDE_COLOR_ADDR= 0;
	SLIDE_IMG_ADDR= 0;
	sosd_sw_tc_disable();
}

/*
 * Synopsis     unsigned char tw_search_ele(struct tw_layout* playout,
 *                                        unsigned char *g,
 *                                        unsigned char *w);
 * Description  search current of ele in layout                 刲醴�醱眳票擁昜璃��
 * Parameters   *playout	- point of tw_layout of struct      �醱票擁磐�硌
 *              *g		    - point of row                      �醱俴
 *              *w		    - point of column                   �醱蹈
 * Return       0xFE: slide successful                          賑傖髡
 *              0xFF: slide error                               賑囮
 *              other value: current stay of number in layout   醴鞞尨票擁昜璃��
 * NOTE         for exMCU+NOR of project
 */
unsigned char tw_search_ele(struct tw_layout* playout, unsigned char *g, unsigned char *w)
{
	unsigned char i= 0, j= 0, c_w= 0;
	struct tw_td	*pimg;

	for(i=0; i<playout->cnt; i++) {
		for(j=0; j<(playout->group[i].width*playout->group[i].height); j++) {
			pimg = (struct tw_td*)playout->group[i].item[j].tw_object;
			if(pimg==CUR_IMG_P) {
				c_w = 1;
				break;
			}
		}
		if(c_w)
			break;
	}

	if(i>=playout->cnt) {
		dbg(0, ("i= %bu, j= %bu!\n", i, j));
		ERROR(("Group Out Range!\n"));
		return SLIDE_HV_ERROR;
	}
	if(j>=(playout->group[i].width*playout->group[i].height)) {
		ERROR(("Group[%bu]: Item Out Range!\n", i));
		return SLIDE_HV_ERROR;
	}

	*g = i;
	*w = j;

	return 0;
}

/*
 * Synopsis     unsigned char tw_page_slide(struct tw_layout* playout,
 *                                        unsigned char dir,
 *                                        unsigned long offset);
 * Description  background of slide effect                      �醱掖劓賑虴彆
 * Parameters   *playout	- point of tw_layout of struct      �醱票擁磐�硌
 * 				dir		    - slide of direction                賑源砃
 *              offset		- slide of offset                   賑講
 * Return       0xFE: slide successful                          賑傖髡
 *              0xFF: slide error                               賑囮
 *              other value: current stay of number in layout   醴鞞尨票擁昜璃��
 * NOTE         for exMCU+NOR of project
 */
unsigned char tw_page_slide(struct tw_layout* playout, unsigned char dir, unsigned short offset)
{
	unsigned char i= 0, j= 0, c_w= 0;

	if(offset<=0)
		return SLIDE_HV_ERROR;

	switch(dir) {
		case DIR_LEFT:
		case DIR_RIGHT:
			if(SLIDE_LOCK==SLIDE_V_DIR) {
				INFO(("can't H slide, current is V slide!\n"));
				return SLIDE_HV_ERROR;
			}
			break;
		case DIR_UP:
		case DIR_DOWN:
			if(SLIDE_LOCK==SLIDE_H_DIR) {
				INFO(("can't V slide, current is H slide!\n"));
				return SLIDE_HV_ERROR;
			}
			break;
	}

	c_w = tw_search_ele(playout, &i, &j);
	if(c_w!=0)
		return c_w;

	switch(playout->type) {
		case IMG_SLIDE:
			c_w = tw_img_slide(playout, dir, offset, i, j);
			break;
		case TD_SLIDE:
			c_w = tw_td_slide(playout, dir, offset, i, j);
			break;
	}

	if((c_w!=SLIDE_MOTION)&&(c_w!=SLIDE_HV_ERROR)) {
		c_w = tw_search_ele(playout, &i, &j);
		if(c_w!=0)
			return c_w;
		else {
			c_w = (playout->group[0].width*playout->cnt)-(playout->cnt-1);
			return (j%playout->group[0].width)+(j/playout->group[0].width*c_w)+(i*(playout->group[0].width-1))+1;			
		}	
	} else
		return c_w;
}

#endif	/* ENABLE_EXMCU_USE_SPIOSD */

#else
/* FOR exMCU NF  */

static unsigned short CUR_MENU_ADDRESS= 0;
static unsigned char CUR_MENU_WIDTH= 0;
static unsigned char CUR_MENU_FONT_W= 0;
static unsigned char CUR_MENU_FONT_H= 0;

/* Load OSD2 Registers Table */
void tw_load_oregs(unsigned char *pconfig, unsigned char length)
{
	osd_wr_regs_tbl_nf(pconfig, length); 
	
	SUPPORT_1BP_ROM= 0;

#ifndef NO_SUPPORT_1BP_ROM_FONT
	if(osd_cfg_read(OSD_PT_LUT_BASEADD)==0x01) {
		SUPPORT_1BP_ROM= 1;
		INFO(("Support 1BP/ROM Font version!\n"));
	}
#endif  
}

/* Load OSD2 LUTs */
void tw_load_luts(unsigned char *plut, unsigned short s_addr, unsigned long length)
{
	osd_wr_lut_tbl_nf(&plut[s_addr], length);	     
}

/* Load OSD2 REMAP LUT */
void tw_load_remap(unsigned char *plut, unsigned short s_addr, unsigned long length)
{
	osd_wr_2bp_lut_tbl_nf(&plut[s_addr], length);	     
}

/* Load OSD2 BG LUT */
void tw_load_bg_lut(unsigned char *plut, unsigned short s_addr, unsigned long length)
{
	osd_wr_bg_lut_tbl_nf(&plut[s_addr], length);	     
}

/* Load OSD2 Tiles */
void tw_load_tiles(unsigned short *ptable, unsigned short s_addr, unsigned short d_addr, unsigned short length)
{
	unsigned short oram_address = d_addr;
	unsigned short i= 0;
	unsigned char code*ptlies;
	unsigned short code*pptable;

	/* CHECK CHANGEABLE ICONS OF TABLE */
	if(ptable[0]==0) {
		
		if((CUR_ICONSET_NUM>ptable[1])||(CUR_ICONSET_NUM<=0))
			CUR_ICONSET_NUM= 1;
		pptable = (unsigned short*) ptable[(CUR_ICONSET_NUM*2)];
  		ptlies = (unsigned char*) pptable[0];
	} else
		ptlies = (unsigned char*) ptable[0];


	IC_WritByte(TWIC_P0, 0xAA, (unsigned char)(oram_address & 0xff));
	IC_WritByte(TWIC_P0, 0xAA, (unsigned char)(oram_address>>8));

	Burst_A_MCU(TWIC_P0, 0xAB);
	for (i=0; i<length; i++)
	{
		Burst_D_MCU(ptlies[s_addr+i]);

#ifdef	TW_EX_DEBUG_MODE		
		dbg (0,("ptlies[%d] = %02bX\n",i, ptlies[s_addr+i]));
#endif
	}
	Burst_P_MCU();

	//osd_oram_dump(0, length/2);
}

static void tw_fixed_menu2(void)
{
	osd_set_ram_addr(0x3FFE);

	Burst_A_MCU(TWIC_P0, 0xAB);
	Burst_D_MCU(0x01);
	Burst_D_MCU(0x08);
	Burst_D_MCU(0x00);
	Burst_D_MCU(0x00);
	Burst_P_MCU();
}

/* Load OSD2 Menu */
void tw_load_menu(unsigned char *pmenu, unsigned short d_addr, unsigned char width, unsigned short length, unsigned char font_w, unsigned char font_h)
{
	char rc= 0;
	unsigned short i=0;

	CUR_MENU_ADDRESS= d_addr;
	CUR_MENU_WIDTH= width;
	CUR_MENU_FONT_W= font_w;
	CUR_MENU_FONT_H= font_h;
	
	IC_WritByte(TWIC_P0, 0xAA, (unsigned char)(d_addr & 0xff));
	IC_WritByte(TWIC_P0, 0xAA, (unsigned char)(d_addr>>8));

	Burst_A_MCU(TWIC_P0, 0xAB);
	for (i=0; i<length; i++)
	{
		Burst_D_MCU(pmenu[i]);

#ifdef	TW_EX_DEBUG_MODE		
		dbg (0,("pmenu[%d] = %02bX\n",i, pmenu[i]));
#endif
	}
	Burst_P_MCU();

	tw_fixed_menu2();
	
	//osd_oram_dump(menu_addr, length/2);
}
#endif	/* RESOURCE_ON_NOR_FLASH */


/*
 * Synopsis     unsigned char region_check( unsigned int tp_x,
 *											unsigned int tp_y,
 *											unsigned int x,
 *											unsigned int y,
 *											unsigned int w,
 *											unsigned int h,
 *											unsigned char type);
 * Description  check touch location whether in region.         復庲茦癲岆瘁弇黺郖
 * Parameters   tp_x	- x of coordinate of touch              茦癲x嵞釱
 *              tp_y	- y of coordinate of touch              茦癲y嵞釱
 * 				x		- x of coordinate of region             郖x嵞釱
 *              y		- y of coordinate of region             郖y嵞釱
 *              w		- width of region                       郖�僅
 *              h		- height of region                      郖詢僅
 *              type	- 1: image/twbc, 2: sprite, 3: OSD2     弇
 * Return       1 for in region.                                1郖
 */
unsigned char region_check(unsigned int tp_x, unsigned int tp_y, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char type)
{
    char ret = 0;

	if(type==TW_UI_TYPE_OSD2) {
		x=x*CUR_MENU_FONT_W; y=y*CUR_MENU_FONT_H; w=w*CUR_MENU_FONT_W; h=h*CUR_MENU_FONT_H;
	}		

    dbg(3, ("x=[%u] y=[%u] w=[%u] h=[%u]!\n", x, y, w, h));

        if((tp_x > x) && (tp_x < x+w) && (tp_y > y) && (tp_y < y+h)) {
            dbg(3, ("in region!\n"));
            ret = 1;
        }

    return ret;
}

/*
 * Synopsis     void next_iconset(void);
 * Description  change to next icon set. (multi-language)   狟珨瞎(嗣惤晟)
 * Parameters   none
 * Return       none
 * NOTE         max of icon set is 8
 */
void next_iconset(void)
{
	CUR_ICONSET_NUM++;
	if(CUR_ICONSET_NUM>8)
		CUR_ICONSET_NUM= 1;	
	dbg(3, ("CUR_ICONSET_NUM= %bu!\n\r", CUR_ICONSET_NUM));
}

/*
 * Synopsis     void set_iconset_num(unsigned char num);        
 * Description  setting number of icon set. (multi-language)    偞隅瞎(嗣惤晟)
 * Parameters   num	- number of icon set                        瞎
 * Return       none
 * NOTE         max of icon set is 8
 */
void set_iconset_num(unsigned char num)
{
	CUR_ICONSET_NUM= num;
	if(CUR_ICONSET_NUM>8)
		CUR_ICONSET_NUM= 1;	
	dbg(3, ("CUR_ICONSET_NUM= %bu!\n\r", CUR_ICONSET_NUM));
}

/*
 * Synopsis     unsigned char get_iconset_num(void);
 * Description  getting number of icon set. (multi-language)    莮瞎(嗣惤晟)
 * Parameters   none
 * Return       current of number of icon set                   瞎講
 * NOTE         max of icon set is 8
 */
unsigned char get_iconset_num(void)
{	
	dbg(3, ("CUR_ICONSET_NUM= %bu!\n\r", CUR_ICONSET_NUM));
	return CUR_ICONSET_NUM;
}

/*
 * Synopsis     unsigned char check_color_config(unsigned short index, 
 *												unsigned char offset,
 *												unsigned char color);
 * Description  check type of color index                   復庲伎屜��
 * Parameters   index	- font of start index of ICON       icon宎index
 *				offset	- font of offset                    index痄講
 *				color	- color index of ICON of struct     icon伎屜��
 * Return       color index (type of in RAM)                �蕣伎屜��
 * NOTE         1BP/ROM Font of color type is different,
 *				check font of type by index and offset,
 *				and change type of color index
 */
#ifndef NO_SUPPORT_1BP_ROM_FONT
unsigned char check_color_config(unsigned short index, unsigned char offset, unsigned char color)
{
	unsigned short FONT_START= 0;
	unsigned char ccolor= 0;

	if(SUPPORT_1BP_ROM) {	
		FONT_START= osd_cfg_read(OSD_CHAR2BP_FONT_INDEX);
		if(FONT_START==0xFF)
			FONT_START= osd_cfg_read(OSD_CHAR4BP_FONT_INDEX);
		FONT_START <<= 1;

		if(index<0xFF)
			index+= offset;
		else
			index= offset;
	
		if(index<FONT_START) {
			/* 1BP/ROM Font */
			ccolor = color;//(color&0x0F)|((color&0xF0)<<1);
		} else {
			/* 2BP/4BP/8BP Font */
			ccolor = (color&0x0F);
		}
	} else {
		/* 2BP/4BP/8BP Font */
		ccolor = (color&0x0F);
	}

	return ccolor;
}
#endif

/*
 * Synopsis     void _icon_draw_at(unsigned short index,
 *									unsigned char color,
 *									unsigned char x,
 *									unsigned char y,
 *									unsigned char width,
 *									unsigned char height,
 *									unsigned short menu_addr,
 *									unsigned char menu_width,
 *									unsigned short offset,
 *									unsigned char blinking);
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)  icon惿奻menu(筳蕆滲宒)
 * Parameters   index		- start font index of ICON                          icon宎tile��
 *				color		- color index of ICON                               icon伎屜��
 * 				x			- x of coordinate of OSD2-MENU (unit is font)       menu X嵞弇离(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)       menu Y嵞弇离(tile)
 *              width		- width of ICON (unit is font)                      icon �僅(tile)
 *              height		- height of ICON (unit is font)                     icon 詢僅(tile)
 *              menu_addr	- base address of OSD2-MENU                         menu oram華硊
 *              menu_width	- width of OSD2-MENU (unit is font)                 menu �僅(tile)
 *				offset		- font index of offset (for double buffer, etc.)    tile 痄講
 *				blinking	- 0: none, 1: blinking                              嬐
 * Return       none                                
 * NOTE         OSD2-MENU of every ROW of char count need same
 */
void _icon_draw_at(unsigned short index, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height,
		 unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking)
{
	unsigned char w,h = 0;
	unsigned short i = 0;
	unsigned short temp= 0;

	i=0;

	color <<= 1;	// lut index[8:5]
	if(blinking)
		color |= 0x20;

    for(h=0; h< height; h++) {
        osd_set_location_addr(x, y+h,  menu_addr, menu_width);
#if 0		
		for(w=0; w< width; w++) {
			temp = index+i+offset;
			temp |= (color<<8);
			osd_set_ram_data(temp);
			i++;
        }
#else
		Burst_A_MCU(TWIC_P0, 0xAB);
		for(w=0; w< width; w++) {
			temp = index+i+offset;
			temp |= (color<<8);
			Burst_D_MCU((unsigned char)(temp&0xFF));
			Burst_D_MCU((unsigned char)(temp>>8));
			i++;	
		}
		Burst_P_MCU();
#endif
    }
}

/*
 * Synopsis     void _tw_icon_draw_at(struct tw_icon* picon,
 *									unsigned short menu_addr,
 *									unsigned char menu_width,
 *									unsigned short offset,
 *									unsigned char blinking);
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)  icon惿奻osd2 menu(筳蕆滲宒)
 * Parameters   menu_addr	- base address of OSD2-MENU                         menu oram 華硊
 *              menu_width	- width of OSD2-MENU (unit is font)                 menu �僅(tile)
 *				offset		- font index of offset (for double buffer, etc.)    icon��黺tile桶笢眳痄講
 *				blinking	- 0: none, 1: blinking                              嬐
 * Return       none
 * NOTE         can assigned base address and width of OSD2-MENU
 *				for advance application,
 *				ex. double buffer
 */
void _tw_icon_draw_at(struct tw_icon* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking)
{   
#ifndef NO_SUPPORT_1BP_ROM_FONT
	unsigned char color= 0;

	color = check_color_config(picon->index, 0, picon->color);

	_icon_draw_at(picon->index, color, picon->x, picon->y, picon->width, picon->height, menu_addr, menu_width, offset, blinking);
#else
	_icon_draw_at(picon->index, (picon->color&0x0F), picon->x, picon->y, picon->width, picon->height, menu_addr, menu_width, offset, blinking);	
#endif
}

/*
 * Synopsis     void _icon_draw(unsigned short index,
 *									unsigned char color,
 *									unsigned char x,
 *									unsigned char y,
 *									unsigned char width,
 *									unsigned char height,
 *									unsigned char blinking);
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)  icon惿奻osd2 menu(筳蕆滲宒)
 * Parameters   index		- start font index of ICON                      icon 宎tile��
 *				color		- color index of ICON                           icon 伎屜��
 * 				x			- x of coordinate of OSD2-MENU (unit is font)   x嵞弇离(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)   y嵞弇离(tile)
 *              width		- width of ICON (unit is font)                  icon �僅(tile)
 *              height		- height of ICON (unit is font)                 icon 詢僅(tile)
 *				blinking	- 0: none, 1: blinking                          嬐
 * Return       none
 */
void _icon_draw(unsigned short index, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char blinking)
{   
#ifndef NO_SUPPORT_1BP_ROM_FONT
	unsigned char ccolor= 0;

	ccolor = check_color_config(index, 0, color);

	_icon_draw_at(index, ccolor, x, y, width, height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, 0, blinking);
#else
	_icon_draw_at(index, (color&0x0F), x, y, width, height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, 0, blinking);	
#endif
}

/*
 * Synopsis     void tw_icon_color_change(struct tw_icon* picon,
 *										unsigned char color);
 * Description  change color index of ICON                      icon眳伎屜��
 * Parameters   *tw_icon	- point of icon struct              icon昜璃硌
 *				color		- want to change of color index     郗載眳伎屜��
 * Return       none
 * NOTE         for 1BP/ROM Font
 */
void tw_icon_color_change(struct tw_icon* picon, unsigned char color)
{
	_icon_draw_at(picon->index, color, picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, 0, 0);
}

/*
 * Synopsis     void tw_icon_1st_color(struct tw_icon* picon);
 * Description  change to 1st color of ICON         祫菴珨瞎伎屜
 * Parameters   *tw_icon	- point of icon struct  icon昜璃硌
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icon_1st_color(struct tw_icon* picon)
{
	tw_icon_draw(picon);
}

/*
 * Synopsis     void tw_icon_2nd_color(struct tw_icon* picon);
 * Description  change to 2nd color of ICON         祫菴媼瞎伎屜
 * Parameters   *tw_icon	- point of icon struct  icon昜璃硌
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icon_2nd_color(struct tw_icon* picon)
{
	_icon_draw_at(picon->index, ((picon->color)>>4), picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, 0, 0);
}

/*
 * Synopsis     void tw_icon_draw(struct tw_icon* picon);
 * Description  show ICON(OSD2)                     鞞尨icon黺menu奻
 * Parameters   *tw_icon	- point of icon struct  icon昜璃硌
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICON
 */
void tw_icon_draw(struct tw_icon* picon)
{   
	_icon_draw(picon->index, picon->color, picon->x, picon->y, picon->width, picon->height, 0);
}

unsigned char check_load_gicon (struct tw_tiles *ptiles, unsigned short oram_addr)
{
	unsigned char i = 0;
	unsigned short oaddr = oram_addr;

	if(ptiles->cnt > 1) {
		dbg(2, ("Group ICON + Changeable!\n\r"));
        if((CUR_ICONSET_NUM >= ptiles->cnt) || (CUR_ICONSET_NUM <= 0))
			CUR_ICONSET_NUM= 1;
        i = CUR_ICONSET_NUM - 1;
	} else if(ptiles->cnt == 1) {
		i = 0;
	} else {
        ERROR(("group icon object is error!\n\r"));
        return 0;
	}

	/* Load icon from tile table */
	tw_load_icon_dma(ptiles->tables[i*2], &oaddr, ptiles->tables[(i*2)+1]);

    return 1;
}

/*
 * Synopsis     void tw_gicon_draw(struct tw_gicon* pgicon);
 * Description  show ICON(OSD2)							鞞尨icon黺menu奻
 * Parameters   *tw_icon	- point of icon struct		icon昜璃硌
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICON
 */
void tw_gicon_draw(struct tw_gicon* pgicon)
{
	/* dynamic load ICON to OSDRAM */
	if(!(check_load_gicon(pgicon->tiles, pgicon->oram_addr)))
        return;

	_icon_draw(pgicon->index, pgicon->color, pgicon->x, pgicon->y, pgicon->width, pgicon->height, 0);
}

/*
 * Synopsis     void tw_gicons_draw(struct tw_gicons* pgicons);
 * Description  show ICON(OSD2)							鞞尨icon黺menu奻
 * Parameters   *tw_icon	- point of icon struct		icon昜璃硌
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICON
 */
void tw_gicons_draw(struct tw_gicons* pgicon)
{
	/* dynamic load ICON to OSDRAM */
	if(!(check_load_gicon(pgicon->tiles, pgicon->oram_addr)))
        return;

	_icons_draw(pgicon->x, pgicon->y, pgicon->width, pgicon->height, pgicon->table,
		pgicon->color, (SUPPORT_1BP_ROM?pgicon->offset:0), 0);
}


/*
 * Synopsis     void _icons_draw_at(unsigned char *table,
 *									unsigned char color,
 *									unsigned char x,
 *									unsigned char y,
 *									unsigned char width,
 *									unsigned char height,
 *									unsigned short menu_addr,
 *									unsigned char menu_width,
 *									unsigned short offset,
 *									unsigned char blinking);
 * Description  writting ICONS(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application) icons惿奻osd2 menu(筳蕆滲宒)
 * Parameters   *table		- point of font index of table                  icons index桶硌
 *				color		- color index of ICON                           伎屜��
 * 				x			- x of coordinate of OSD2-MENU (unit is font)   x嵞弇离(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)   y嵞弇离(tile)
 *              width		- width of ICON (unit is font)                  icon�僅
 *              height		- height of ICON (unit is font)                 icon詢僅
 *              menu_addr	- base address of OSD2-MENU                     menu oram 弇硊
 *              menu_width	- width of OSD2-MENU (unit is font)             memu �僅(tile)
 *				offset		- font index of offset                          index黺tile桶眳痄講
 *				blinking	- 0: none, 1: blinking                          嬐
 * Return       none
 * NOTE         OSD2-MENU of every ROW of char count need same
 */
void _icons_draw_at(unsigned char *table, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height,
		 unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking)
{
	unsigned char w,h = 0;
	unsigned short i = 0;
	unsigned short temp = 0;

	i=0;

	color <<= 1;	// lut index[8:5]
	if(blinking)
		color |= 0x20;

    for(h=0; h< height; h++) {
        osd_set_location_addr(x, y+h,  menu_addr, menu_width);
#if 0
		for(w=0; w< width; w++) {
			temp = table[i];
			if(temp==0xFF)	/* for Blank ROM Font */
				temp= (color<<8);
			else {
				temp = table[i]+offset;
				temp |= (color<<8);
			}
			osd_set_ram_data(temp);
			i++;
        }
#else
		Burst_A_MCU(TWIC_P0, 0xAB);
		for(w=0; w< width; w++) {
			temp = table[i];
			if(temp==0xFF)	/* for Blank ROM Font */
				temp= (color<<8);
			else {
				temp = table[i]+offset;
				temp |= (color<<8);
			}

			Burst_D_MCU((unsigned char)(temp&0xFF));
			Burst_D_MCU((unsigned char)(temp>>8));
			i++;	
		}
		Burst_P_MCU();
#endif
	}
}

/*
 * Synopsis     void _tw_icons_draw_at(struct tw_icon* picon,
 *									unsigned short menu_addr,
 *									unsigned char menu_width,
 *									unsigned short offset,
 *									unsigned char blinking);
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)  icon惿奻osd2 menu(筳蕆滲宒)
 * Parameters   menu_addr	- base address of OSD2-MENU                         menu oram 華硊
 *              menu_width	- width of OSD2-MENU (unit is font)                 menu �僅(tile)
 *				offset		- menu_addr of offset (for double buffer, etc.)     icon��黺tile桶笢眳痄講
 *				blinking	- 0: none, 1: blinking                              嬐
 * Return       none
 * NOTE         can assigned base address and width of OSD2-MENU
 *				for advance application,
 *				ex. double buffer
 */
void _tw_icons_draw_at(struct tw_icons* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking)
{   
#ifndef NO_SUPPORT_1BP_ROM_FONT
	unsigned char color= 0;

	color = check_color_config(picon->table[0], (SUPPORT_1BP_ROM?picon->offset:0), picon->color);

	_icons_draw_at(picon->table, color, picon->x, picon->y, picon->width, picon->height, menu_addr, menu_width, (SUPPORT_1BP_ROM?picon->offset:0)+offset, blinking);
#else
	_icons_draw_at(picon->table, (picon->color&0x0F), picon->x, picon->y, picon->width, picon->height, menu_addr, menu_width, offset, blinking);	
#endif
}

/*
 * Synopsis     void _icons_draw(unsigned char x,
 *									unsigned char y,
 *									unsigned char width,
 *									unsigned char height,
 *									unsigned char *table,
 *									unsigned char color,
 *									unsigned char offset,
 *									unsigned char blinking);
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)  icons惿奻osd2 menu(筳蕆滲宒)
 * Parameters   x			- x of coordinate of OSD2-MENU (unit is font)   x嵞弇离(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)   y嵞弇离(tile)
 *              width		- width of ICON (unit is font)                  icon�僅(tile)
 *              height		- height of ICON (unit is font)                 icon詢僅(tile)
 *				*table		- point of font index of table                  icon index桶硌
 *				color		- color index of ICONS                          伎屜��
 *				offset		- font index of offset                          index黺tile桶笢眳痄講
 *				blinking	- 0: none, 1: blinking                          嬐
 * Return       none
 */
void _icons_draw(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *table, unsigned char color, unsigned char blinking, unsigned short offset)
{
#ifndef NO_SUPPORT_1BP_ROM_FONT
	unsigned char ccolor= 0;

	ccolor = check_color_config(table[0], (SUPPORT_1BP_ROM?offset:0), color);

	_icons_draw_at(table, ccolor, x, y, width, height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, offset, blinking);
#else
	_icons_draw_at(table, (color&0x0F), x, y, width, height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, offset, blinking);
#endif
}

/*
 * Synopsis     void tw_icons_color_change(struct tw_icon* picon,
 *										unsigned char color);
 * Description  change color index of ICONS                     icons伎屜
 * Parameters   *tw_icon	- point of icon struct              icons昜璃硌
 *				color		- want to change of color index     郗眳伎屜��
 * Return       none
 * NOTE         for 1BP/ROM Font
 */
void tw_icons_color_change(struct tw_icons* picon, unsigned char color)
{
	_icons_draw_at(picon->table, color, picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, (SUPPORT_1BP_ROM?picon->offset:0), 0);
}

/*
 * Synopsis     void tw_icons_1st_color(struct tw_icon* picon);
 * Description  change to 1st color of ICONS        偞隅icons菴珨瞎伎屜
 * Parameters   *tw_icon	- point of icon struct  icons昜璃��
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icons_1st_color(struct tw_icons* picon)
{
	tw_icons_draw(picon);
}

/*
 * Synopsis     void tw_icons_2nd_color(struct tw_icon* picon);
 * Description  change to 2nd color of ICONS        偞隅icons菴媼瞎伎屜
 * Parameters   *tw_icon	- point of icon struct  icons昜璃��
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icons_2nd_color(struct tw_icons* picon)
{
	_icons_draw_at(picon->table, ((picon->color)>>4), picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, (SUPPORT_1BP_ROM?picon->offset:0), 0);
}

/*
 * Synopsis     void tw_icons_draw(struct tw_icon* picon);
 * Description  show ICONS(OSD2)                    鞞尨icons黺menu奻
 * Parameters   *tw_icon	- point of icon struct  icons昜璃硌
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICONS
 */
void tw_icons_draw(struct tw_icons* picon)
{
	_icons_draw(picon->x, picon->y, picon->width, picon->height, picon->table, 
				picon->color, 0, (SUPPORT_1BP_ROM?picon->offset:0));
}

/*
 * Synopsis     void tw_icon_color_change(struct tw_icon* picon,
 *										unsigned char color);
 * Description  change color index of ICON                      icon眳伎屜��
 * Parameters   *tw_icon	- point of icon struct              icon昜璃硌
 *				color		- want to change of color index     郗載眳伎屜��
 * Return       none
 * NOTE         for 1BP/ROM Font
 */
void tw_gicon_color_change(struct tw_gicon* picon, unsigned char color)
{
	_icon_draw_at(picon->index, color, picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, 0, 0);
}

/*
 * Synopsis     void tw_icons_color_change(struct tw_icon* picon,
 *										unsigned char color);
 * Description  change color index of ICONS                     icons伎屜
 * Parameters   *tw_icon	- point of icon struct              icons昜璃硌
 *				color		- want to change of color index     郗眳伎屜��
 * Return       none
 * NOTE         for 1BP/ROM Font
 */
void tw_gicons_color_change(struct tw_gicons* picon, unsigned char color)
{
	_icons_draw_at(picon->table, color, picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, (SUPPORT_1BP_ROM?picon->offset:0), 0);
}

#define FONT_ASCII_OFFSET	10
void _romfont_draw_at(struct tw_txt* ptxt, unsigned char x, unsigned char y, const unsigned char *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color, unsigned char blink)
{
	unsigned char i;
	unsigned short temp= 0;
	
	if(blink)
		color |= 0x20;
							
   	osd_set_location_addr(x, y, menu_addr, menu_width);

#if 0	
	for(i=0; ucs[i]!=0; i++) {
		if((x+i)>=menu_width) 
			break;
		if(i>=ptxt->width)
			break;
		temp = (color<<8);
		if(ucs[i]==' ')	//BLANK
			OSD_SET_RAM_DATA(temp);
		else if(ucs[i]=='^') {
			temp |= (ucs[i]+ptxt->index+2-FONT_ASCII_OFFSET);
			OSD_SET_RAM_DATA(temp);
		} else {
			temp |= (ucs[i]+ptxt->index-FONT_ASCII_OFFSET);
			OSD_SET_RAM_DATA(temp);
		}
	}
#else
	Burst_A_MCU(TWIC_P0, 0xAB);
	for(i=0; ucs[i]!=0; i++) {
		if((x+i)>=menu_width) 
			break;
		if(i>=ptxt->width)
			break;
		temp = (color<<8);
		if(ucs[i]==' ')	//BLANK
			temp = temp;
		else if(ucs[i]=='^') {
			temp |= (ucs[i]+ptxt->index+2-FONT_ASCII_OFFSET);
		} else {
			temp |= (ucs[i]+ptxt->index-FONT_ASCII_OFFSET);
		}

		Burst_D_MCU((unsigned char)(temp&0xFF));
		Burst_D_MCU((unsigned char)(temp>>8));	
	}
	Burst_P_MCU();
#endif	 
}

void _romfont_draw(struct tw_txt* ptxt, unsigned char x, unsigned char y, const unsigned char *ucs, unsigned char color, unsigned char blink)
{							
   	_romfont_draw_at(ptxt, x, y, ucs, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, color, blink);	 
}

void tw_romfont_draw(struct tw_txt* ptxt, const unsigned char *ucs)
{							
   	_romfont_draw_at(ptxt, ptxt->x, ptxt->y, ucs, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, ((ptxt->fg_color)<<0) + ((ptxt->bg_color)<<5), 0);	 
}

#ifdef RESOURCE_ON_NOR_FLASH
void _twfont_draw_at(struct tw_txt* ptxt, unsigned char x, unsigned char y, unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color, unsigned char blink)
{
#if 0
	unsigned char i, rc, draw_len, draw_offset, size_id;

	size_id = _tw_get_font_size_id();	
	tw_menu_reset_index_n(ptxt->index);	

	draw_offset = 0;
	for(i=0; i<ptxt->height; i+=TOF_FONT_SPLIT_H[size_id]) {
		draw_len = _tw_get_draw_len(ucs+draw_offset, ptxt->width, size_id);
		rc = tw_menu_draw_nstring(x, y + i, ucs+draw_offset, menu_addr, menu_width, (blink?(color|0x20):color), size_id, draw_len);	 	 
		draw_offset += draw_len;
	  	if(ucs[draw_offset] == 0) {
			break;
		}
	}
#else
	req_ser_draw_twfont(ptxt, CUR_MENU_P->tofs, x, y, ucs, menu_addr, menu_width, (blink?(color|0x20):color));
#endif
}

void _twfont_draw(struct tw_txt* ptxt, unsigned char x, unsigned char y, unsigned short *ucs, unsigned char color, unsigned char blink)
{	
	_twfont_draw_at(ptxt, x, y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, color, blink);	  
}

void tw_twfont_draw(struct tw_txt* ptxt, unsigned short *ucs)
{	
	_twfont_draw_at(ptxt, ptxt->x, ptxt->y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, ((ptxt->fg_color)<<0) + ((ptxt->bg_color)<<5), 0);	  
}

/*
 * Synopsis     void tw_text_draw(struct tw_txt* ptxt, unsigned short *ucs);
 * Description  show ICON(OSD2)                             霑庨txt祫menu
 * Parameters   *ptxt	- point of txt struct               txt昜璃硌
 *              *ucs	- start point of string (utf-8)     趼揹硌(utf-8)
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show text
 *              text types include twfont and ROMFont
 */
void tw_text_draw(struct tw_txt* ptxt, unsigned short *ucs)
{		
	if(ptxt->base_addr==0) {
		/* ROM FONT */
		tw_romfont_draw(ptxt, (unsigned char*)ucs);
	} else {
		/* TWFONT */
		tw_twfont_draw(ptxt, ucs);	
	}	
}

/*
 * Synopsis     void tw_text_color_change(struct tw_icon* picon,
 *                                      unsigned short *ucs,
 *										unsigned char color);
 * Description  change color index of ICONS                 趼倰眳伎屜��
 * Parameters   *ptxt	- point of txt struct               txt昜璃硌
 *              *ucs	- start point of string (utf-8)     趼揹硌(utf-8)
 *				color	- want to change of color index     郗眳伎屜��
 * Return       none
 * NOTE         for text (1BP/ROM Font)
 */
void tw_text_color_change(struct tw_txt* ptxt, unsigned short *ucs, unsigned char color)
{
	if(ptxt->base_addr==0) {
		/* ROM FONT */
		_romfont_draw(ptxt, ptxt->x, ptxt->y, (unsigned char*)ucs, color, 0);	 
	} else {
		/* TWFONT */
		_twfont_draw(ptxt, ptxt->x, ptxt->y, ucs, color, 0);	  
	}
}

/* T123 OSD Font Drawing Info Table    24x24 32x31 32x36 48x48  */
unsigned short code TOF_FONT_SPLIT_W[]    = { 2, 2, 2, 3, 4, 1};
unsigned short code TOF_FONT_SPLIT_H[]    = { 1, 1, 2, 2, 2, 2};
unsigned short code TOF_FONT_SPLIT_HSA[]  = { 1, 1, 1, 2, 2, 1};

struct tw_scl_txt tw_init_scrol(struct tw_txt* ptxt, unsigned short *ucs, unsigned char direct, unsigned char tail_blank)
{
	struct tw_scl_txt ret;
	
	tw_menu_reset_index_n(ptxt->index);
	
	req_ser_draw_twfont_scr(ptxt, CUR_MENU_P->tofs, 0, 0, ucs, 0, 0, 0);

	menu_string_start_index= IC_ReadByte(TWIC_P1, TWF_MENU_INDEX);
	ret.size_id = IC_ReadByte(TWIC_P1, TWF_SIZE_ID);
	ret.total_len = (menu_string_start_index - ptxt->index) / TOF_FONT_SPLIT_H[ret.size_id];	
	ret.direct = direct;  
	ret.tail_blank = tail_blank; 
	ret.cur_index = 0;
	ret.ptxt = ptxt;	
 
	tw_scrolling(&ret);
	
	return ret;  
}

/*
 * Synopsis     tw_scrolling(struct tw_scl_txt* scl_txt)
 * Description       
 *              --init
 *                   scr idx
 *                   00 01 02 03 04 05 06   07 08 09 10
 *                          txt str         blank tail
 *                   00 02 04 06 08 10 12 | xx xx xx xx
 *                   01 03 05 07 09 11 13 | xx xx xx xx
 *               
 *              --scroll 1
 *               scr 01 02 03 04 05 06 07   08 09 10 00
 *               
 *                   02 04 06 08 10 12 xx | xx xx xx 00
 *                   03 05 07 09 11 13 xx | xx xx xx 01  
 *               
 *              --scroll 0
 *               scr 10 00 01 02 03 04 05   06 07 08 09
 *               
 *                   xx 00 02 04 06 08 10 | 12 xx xx xx 
 *                   xx 01 03 05 07 09 11 | 13 xx xx xx
 * 
 * Parameters   scl_txt	- scroll structure generate by tw_init_scrol()
 * Return       none
 */
#define _GET_SCR_IDX(idx) (idx>=scr_len)?0:idx
                                    /*OSD_SET_RAM_DATA can't use parameter (color_h|idx) */
#define _DRAW_TILE(posx,posy,idx) osd_set_location_addr(scl_txt->ptxt->x+posx, scl_txt->ptxt->y+posy,\ 
                                      CUR_MENU_P->addr, CUR_MENU_P->width);\
                                  idx |= color_h;\					 
                                  OSD_SET_RAM_DATA(idx)
#define _GET_TILE_IDX(idx) (idx>=scl_txt->total_len)?0L:\
                               ( scl_txt->ptxt->index+(idx*(unsigned short)split_h+(unsigned short)i) )

void tw_scrolling(struct tw_scl_txt* scl_txt)
{
    unsigned short color_h = (unsigned short)(scl_txt->ptxt->fg_color) 
                           | ((unsigned short)scl_txt->ptxt->bg_color<<5L);
    unsigned char split_h = TOF_FONT_SPLIT_H[scl_txt->size_id];

    unsigned char scr_len = scl_txt->total_len + (scl_txt->tail_blank*split_h);
    unsigned char scr_idx = scl_txt->cur_index;
    unsigned short idx;
    unsigned char i, j;

    color_h<<=9L;

    for(i=0; i<split_h; i++){ //y axis
        scr_idx = scl_txt->cur_index;
        for(j=0; j<scl_txt->ptxt->width; j++,scr_idx++){ //x axis
            scr_idx = _GET_SCR_IDX(scr_idx);
            idx = _GET_TILE_IDX(scr_idx);
            _DRAW_TILE(j, i, idx);
        }
    }


    if(scl_txt->direct){
        scl_txt->cur_index++;
        if(scl_txt->cur_index>=scr_len){
            scl_txt->cur_index = 0;    
        }
    }else{
        if(scl_txt->cur_index == 0){
            scl_txt->cur_index = scr_len-1;
        }else{
            scl_txt->cur_index--;
        }
    }         
}

#else
void tw_text_draw(struct tw_txt* ptxt, const unsigned char *ucs)
{							
   	_romfont_draw_at(ptxt, ptxt->x, ptxt->y, ucs, CUR_MENU_ADDRESS, CUR_MENU_WIDTH, ((ptxt->fg_color)<<0) + ((ptxt->bg_color)<<5), 0);	 
}

void tw_text_color_change(struct tw_txt* ptxt, unsigned char *ucs, unsigned char color)
{
	_romfont_draw(ptxt, ptxt->x, ptxt->y, ucs, color, 0);
}
#endif

/*
 * Synopsis     void tw_osd_menu_write(unsigned char x,
 *									unsigned char y,
 *									unsigned short index,
 *									unsigned char color);
 * Description  writting are font index and color index at OSD2-MENU(OSD RAM) (advanced application)    �趼倰/伎屜��祫mrenu杻隅弇离(筳蕆滲)
 * Parameters   x		- x of coordinate of OSD2-MENU (unit is font)   x嵞弇离 (tile)
 *              y		- y of coordinate of OSD2-MENU (unit is font)   y嵞弇离 (tile)
 *              index	- font of index                                 tile ��
 *              color	- color of index                                伎屜 ��
 * Return       none
 */
void tw_osd_menu_write(unsigned char x, unsigned char y, unsigned short index, unsigned char color)
{
	osd_menu_write(x, y, index, color, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);	
}

/*
 * Synopsis     void tw_osd_menu_write_line(unsigned char y,
 *									unsigned short index,
 *									unsigned char color);
 * Description  writting are font index and color index at OSD2-MENU(OSD RAM)   �趼倰/伎屜��祫menu杻隅俴(筳蕆滲)
 *              and a line (menu_width)
 * Parameters   y		- y of coordinate of OSD2-MENU (unit is font)   y嵞弇离(tile)
 *              index	- font of index                                 tile ��
 *              color	- color of index                                伎屜 ��
 * Return       none
 */
void tw_osd_menu_write_line(unsigned char y, unsigned short index, unsigned char color)
{
	osd_menu_write_line(y, index, color, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}

/*
 * Synopsis     void tw_clear_icon(struct tw_icon* picon);
 * Description  clear(writting a 0) a ICON at OSD2-MENU(OSD RAM)    壺menu奻議icon
 * Parameters   *tw_icon    - point of icon struct                  icon昜璃硌
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_osd_clear_line(unsigned char y)
{
	osd_clear_line(y, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}

/*
 * Synopsis     void tw_osd_clear_char(unsigned char x, unsigned char y);
 * Description  clear(writting a 0) a char at OSD2-MENU(OSD RAM)    壺menu杻隅tile
 * Parameters   x	- x of coordinate of OSD2-MENU (unit is font)   x嵞弇离(tile)
 *              y	- y of coordinate of OSD2-MENU (unit is font)   y嵞弇离(tile)
 * Return       none
 * NOTE         default(must) blank font of index is 0
 *              just clear to blank font (LSB), no clear color index (MSB)
 */
void tw_osd_clear_char(unsigned char x, unsigned char y)
{
	osd_clear_char(x, y, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}

/*
 * Synopsis     void tw_clear_icon(struct tw_icon* picon);
 * Description  clear(writting a 0) a ICON at OSD2-MENU(OSD RAM)    壺menu奻議icon
 * Parameters   *tw_icon    - point of icon struct                  icon昜璃硌
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_icon(struct tw_icon* picon) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}

/*
 * Synopsis     void tw_clear_icons(struct tw_icons* picon);
 * Description  clear(writting a 0) a ICONS at OSD2-MENU(OSD RAM)   壺menu奻議icons
 * Parameters   *tw_icons    - point of icons struct                icons昜璃硌
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_icons(struct tw_icons* picon) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}

/*
 * Synopsis     void tw_clear_gicon(struct tw_icon* picon);
 * Description  clear(writting a 0) a ICON at OSD2-MENU(OSD RAM)    壺menu奻議icon
 * Parameters   *tw_icon    - point of icon struct                  icon昜璃硌
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_gicon(struct tw_gicon* picon) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}

/*
 * Synopsis     void tw_clear_gicons(struct tw_icons* picon);
 * Description  clear(writting a 0) a ICONS at OSD2-MENU(OSD RAM)   壺menu奻議icons
 * Parameters   *tw_icons    - point of icons struct                icons昜璃硌
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_gicons(struct tw_gicons* picon) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}


/*
 * Synopsis     void tw_clear_txt(struct tw_txt* ptxt);
 * Description  clear(writting a 0) a txt at OSD2-MENU(OSD RAM)     壺menu奻議txt
 * Parameters   *ptxt   - point of txt struct                       txt昜璃硌
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_txt(struct tw_txt* ptxt) {
	osd_clear_block(ptxt->x, ptxt->y, ptxt->width, ptxt->height, CUR_MENU_ADDRESS, CUR_MENU_WIDTH);
}

/*
 * Synopsis     void _tw_clear_icon_at(struct tw_icon* picon,
 *                                  unsigned short menu_addr,
 *                                  unsigned char menu_width);
 * Description  clear(writting a 0) a ICON at OSD2-MENU(OSD RAM) (advanced application)     壺menu奻議icon(筳蕆滲宒)
 * Parameters   *picon    - point of icon struct                    icon昜璃硌
 *              menu_addr	- base address of OSD2-MENU             menu oram 華硊
 *              menu_width	- width of OSD2-MENU (unit is font)     menu �僅(tile)
 * Return       none
 * NOTE         default(must) blank font of index is 0
 *              can assigned base address and width of OSD2-MENU
 */
void _tw_clear_icon_at(struct tw_icon* picon, unsigned short menu_addr, unsigned char menu_width) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, menu_addr, menu_width);
}

/*
 * Synopsis     void _tw_clear_icons_at(struct tw_icons* picons,
 *                                  unsigned short menu_addr,
 *                                  unsigned char menu_width);
 * Description  clear(writting a 0) a ICONS at OSD2-MENU(OSD RAM) (advanced application)    壺menu奻議icons(筳蕆滲宒)
 * Parameters   *picons    - point of icons struct                  icons昜璃硌
 *              menu_addr	- base address of OSD2-MENU             menu oram 華硊
 *              menu_width	- width of OSD2-MENU (unit is font)     menu �僅(tile)
 * Return       none
 * NOTE         default(must) blank font of index is 0
 *              can assigned base address and width of OSD2-MENU
 */
void _tw_clear_icons_at(struct tw_icons* picon, unsigned short menu_addr, unsigned char menu_width) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, menu_addr, menu_width);
}

/*
 * Synopsis     void _tw_clear_txt_at(struct tw_txt* ptxt,
 *                                  unsigned short menu_addr,
 *                                  unsigned char menu_width);
 * Description  clear(writting a 0) a txt at OSD2-MENU(OSD RAM) (advanced application)  壺menu奻議txt(筳蕆滲宒)
 * Parameters   *ptxt       - point of txt struct                   txt昜璃硌
 *              menu_addr	- base address of OSD2-MENU             menu oram 華硊
 *              menu_width	- width of OSD2-MENU (unit is font)     menu �僅(tile)
 * Return       none
 * NOTE         default(must) blank font of index is 0
 *              can assigned base address and width of OSD2-MENU
 */
void _tw_clear_txt_at(struct tw_txt* ptxt, unsigned short menu_addr, unsigned char menu_width) {
	osd_clear_block(ptxt->x, ptxt->y, ptxt->width, ptxt->height, menu_addr, menu_width);
}

/*
 * Synopsis     void tw_osd_clear_menu(void);
 * Description  clear all char(0) at OSD2-MENU(OSD RAM)     壺menu
 * Parameters   none
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_osd_clear_menu(void)
{
	osd_clear_menu(CUR_MENU_ADDRESS, CUR_MENU_WIDTH, CUR_MENU_WIDTH);
	DBG_PRINT("CUR_MENU_ADDRESS=%bd, CUR_MENU_WIDTH=%bd, CUR_MENU_WIDTH=%bd\r\n",CUR_MENU_ADDRESS, CUR_MENU_WIDTH, CUR_MENU_WIDTH);
}
#endif /* ENABLE_EX_MCU */
