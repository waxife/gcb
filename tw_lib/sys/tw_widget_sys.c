/**
 *  @file   tw_widget_sys.c
 *  @brief  terawins UI widget system
 *  $Id: tw_widget_sys.c,v 1.51 2013/11/21 10:46:44 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.51 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 *
 *  @date   2011/09/15  ken     New file.
 *
 */

#include "tw_widget_sys.h"
#include "sys.h"
#include "spiosd.h"
#include "osd2api.h"
#include "spirw.h"
#include "iccontrol.h"
#include "rc.h"
#include "reg_tw.h"
//#include "res.h"

/* DEBUG MODE SETTING */
//#define TW_WG_DEBUG_MODE

/* Status variable 
 * 		CUR_PAGE_ID		current of page ID
 *		CUR_MENU_P		current of OSD2 menu of struct
 *		CUR_MENU_RES	current of OSD2 menu of resource
 *		CUR_TOF_RES		current of TOF	of resource
 *		CUR_BK_ID		current of Background (IMG/TWBC) of ID
 *		CUR_SPR_ID	    current of Sprite of ID
 *		CUR_ICONSET_NUM current of ICON SET of number
 *      CUR_2BP_IDX 	current of OSD2 2BP start index
 */
unsigned char CUR_PAGE_ID=0xFF;
struct tw_menu* CUR_MENU_P;
struct res_menu_t CUR_MENU_RES;  
struct res_tof_t CUR_TOF_RES;
static unsigned short CUR_BK_ID= 0xFFFF;
static unsigned short CUR_SPR_ID= 0xFFFF; 
static unsigned short CUR_2BP_IDX _at_ 0x00A8;     //add for 2bp twfont, use _at_ preempt important memory.
static unsigned char CUR_ICONSET_NUM= 0;
static unsigned short menu_string_start_index = 0;

#ifndef NO_SUPPORT_1BP_ROM_FONT
static unsigned char SUPPORT_1BP_ROM= 0;
#endif

extern unsigned char tw_page_handler(unsigned char id, TW_EVENT* event);

/*
 * Synopsis     unsigned char region_check(TW_EVENT* event, 
 *											unsigned int x,
 *											unsigned int y,
 *											unsigned int w,
 *											unsigned int h,
 *											unsigned char type);
 * Description  check touch location whether in region. 				確認觸碰點是否位於設定區域
 * Parameters   event	- event (touch event) of tw widget system		?面系統之事件
 * 				x		- x of coordinate of region						x軸座標
 *              y		- y of coordinate of region						y軸座標
 *              w		- width of region								區域?度 
 *              h		- height of region								區域高度
 *              type	- 1: image/twbc, 2: sprite, 3: OSD2
 * Return       1 for in region.
 */
unsigned char region_check(TW_EVENT* event, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char type)
{
    char ret = 0;

	if(type==TW_UI_TYPE_OSD2) {
		x=x*CUR_MENU_P->font_w; y=y*CUR_MENU_P->font_h; w=w*CUR_MENU_P->font_w; h=h*CUR_MENU_P->font_h;
	}		

    dbg(3, ("x=[%u] y=[%u] w=[%u] h=[%u]!\n", x, y, w, h));

    if((event->type==TW_EVENT_TYPE_TOUCH_DOWN)||(event->type==TW_EVENT_TYPE_TOUCH_RELEASE)||(event->type==TW_EVENT_TYPE_TOUCH_MOTION)||(event->type==TW_EVENT_TYPE_TOUCH_REPEAT))
        if((event->touch_panel.x > x) && (event->touch_panel.x < x+w) && (event->touch_panel.y > y) && (event->touch_panel.y < y+h)) {
            dbg(3, ("in region!\n"));
            ret = 1;
        }

    return ret;
}

/*
 * Synopsis     void next_iconset(void);
 * Description  change to next icon set. (multi-language)	切換至下一組圖標(多國語言)
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
 * Description  setting number of icon set. (multi-language)	設定顯示圖標組別(多國語言)
 * Parameters   num	- number of icon set		圖標組??
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
 * Description  getting number of icon set. (multi-language)	讀取圖標組數量(多國語言)
 * Parameters   none
 * Return       current of number of icon set
 * NOTE         max of icon set is 8
 */
unsigned char get_iconset_num(void)
{	
	dbg(3, ("CUR_ICONSET_NUM= %bu!\n\r", CUR_ICONSET_NUM));
	return CUR_ICONSET_NUM;
}

/*
 * Synopsis     void tw_load_oregs(unsigned short id);
 * Description  loading for OSD2 of registers table				讀取OSD2設定表
 * Parameters   id	- OSD2 registers table of resource ID		設定表物件ID
 * Return       none
 * NOTE         OSD_31[0] as a use 1BP/ROM font of bit for firmware,
 *				so pattern fill function is can't use 
 */
void tw_load_oregs(unsigned short id)
{
	char rc= 0;
	struct res_oreg_t oreg_s;
	
#ifdef TW_WG_DEBUG_MODE
	show_resource_header(id);
#endif
	
	get_res_oreg(id, &oreg_s);
	
	osd_wr_regs_tbl(oreg_s.base_addr, oreg_s.length);

	SUPPORT_1BP_ROM= 0;

#ifndef NO_SUPPORT_1BP_ROM_FONT
	if(osd_cfg_read(OSD_PT_LUT_BASEADD)==0x01) {
		SUPPORT_1BP_ROM= 1;
		dbg(3, ("Support 1BP/ROM Font version!\n"));
	}
#endif        
}

/*
 * Synopsis     void _tw_load_menu_at(unsigned short id, unsigned short addr);
 * Description  loading for OSD2 of MEMU data (advaced application)		讀取OSD2 menu資料(進階函式)
 * Parameters   id		- OSD2 MENU data of resource ID					menu物件ID
 *				addr	- assingned a OSD2 RAM of base address			oRAM地址
 * Return       none
 * NOTE         assingend a OSD2 RAM of base address for application,
 *				ex. double buffer for OSD2 MENU swap 
 */
#define DMA_MAX_LENGTH	0x2000 
void _tw_load_menu_at(unsigned short id, unsigned short addr)
{
	char rc= 0;
	unsigned char i=0, count=0;
	struct res_menu_t menu_s;

#ifdef TW_WG_DEBUG_MODE
	show_resource_header(id);
#endif

	get_res_menu(id, &menu_s);

	count = (menu_s.length/DMA_MAX_LENGTH);

#ifdef TW_WG_DEBUG_MODE
	dbg(0, (">>>> MENU LENGTH %u !\n", menu_s.length));
#endif

    for(i=0; i< count; i++) {
		spi_dma2oram((menu_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(addr+(i*DMA_MAX_LENGTH/2)), 
				DMA_MAX_LENGTH);
#ifdef TW_WG_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(menu_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(addr+(i*DMA_MAX_LENGTH/2)), 
				DMA_MAX_LENGTH));

		osd_oram_dump((addr+(i*DMA_MAX_LENGTH/2)), DMA_MAX_LENGTH/2);
#endif
	}
	if((menu_s.length%DMA_MAX_LENGTH)) {
		spi_dma2oram((menu_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(addr+(i*DMA_MAX_LENGTH/2)), 
				(menu_s.length%DMA_MAX_LENGTH));
#ifdef TW_WG_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(menu_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(addr+(i*DMA_MAX_LENGTH/2)), 
				(menu_s.length%DMA_MAX_LENGTH)));
		osd_oram_dump((addr+(i*DMA_MAX_LENGTH/2)), (menu_s.length%DMA_MAX_LENGTH)/2);
#endif	
	}

	/* setup to 2bp tof */
	CUR_2BP_IDX = osd_cfg_read(0x02) * 2;
}

/*
 * Synopsis     void tw_load_menu(struct tw_menu* pmenu);
 * Description  loading for OSD2 of MEMU data				讀取OSD2 menu資料
 * Parameters   pmenu	- point of OSD2 MENU of struct		menu物件指標
 * Return       none
 */
void tw_load_menu(struct tw_menu* pmenu)
{
	_tw_load_menu_at(pmenu->menuID, pmenu->addr);
}

/*
 * Synopsis     void tw_load_icon(unsigned short id, unsigned short *oaddr);
 * Description  loading for OSD2 of ICON(Fonts) data			讀取osd2 icon(字型)資料
 * Parameters   id		- OSD2 MENU data of resource ID			icon物件指標		
 *				*oaddr	- point of OSD2 base address (load after add a size of ICON)
 * Return       none
 */
void tw_load_icon(unsigned short id, unsigned short *oaddr)
{
	char rc= 0;
	unsigned char i= 0;
	unsigned short count= 0;
	struct res_icon_t icon_s;

	if(id == 0xFFFF)
		return;
#ifdef TW_WG_DEBUG_MODE
	show_resource_header(id);
#endif

	get_res_icon(id, &icon_s);

	count = (icon_s.font_h*icon_s.font_w/((8/icon_s.bp)));
	if(count%2) 
		count++;	 
	count*= icon_s.count;  
	count = (count/DMA_MAX_LENGTH);						  	 

    for(i=0; i< count; i++) {
		(*oaddr)+= (i==0?0:(DMA_MAX_LENGTH/2));
		spi_dma2oram((icon_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				DMA_MAX_LENGTH);
#ifdef TW_WG_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(icon_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				DMA_MAX_LENGTH));
		osd_oram_dump((*oaddr), DMA_MAX_LENGTH/2);
#endif
	}
	//count = ((icon_s.count*icon_s.font_h*icon_s.font_w)/((8/icon_s.bp)));
	count = (icon_s.font_h*icon_s.font_w/((8/icon_s.bp)));
	if(count%2) 
		count++;	 
	count*= icon_s.count;  

	if((count%DMA_MAX_LENGTH)) {
		(*oaddr)+= (i==0?0:(DMA_MAX_LENGTH/2));
		spi_dma2oram((icon_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				(count%DMA_MAX_LENGTH));
		
#ifdef TW_WG_DEBUG_MODE		
		dbg(0, ("spi_dma2oram(%08lX, %08X, %08X)\n", 
				(icon_s.base_addr+(i*DMA_MAX_LENGTH)), 
				(*oaddr), 
				(count%DMA_MAX_LENGTH)));
		osd_oram_dump((*oaddr), (count%DMA_MAX_LENGTH)/2);
#endif
		(*oaddr)+= ((count%DMA_MAX_LENGTH)/2);
	}
}

/*
 * Synopsis     void _tw_load_tiles_at(unsigned short id, unsigned short oram_addr);
 * Description  loading for OSD2 of tiles(ICONs) table (advaced application)	讀取OSD2 tile表(進階函數)
 * Parameters   id			- OSD2 tiles(ICONs) table of resource ID	tile表 ID
 *				oram_addr	- assingned a OSD2 RAM of base address		ORAM 地址
 * Return       none
 * NOTE         assingend a OSD2 RAM of base address for application,
 *				ex. double buffer for OSD2 ICON swap 
 */
void _tw_load_tiles_at(unsigned short id, unsigned short oram_addr)
{
	char rc= 0;
	unsigned char i= 0;
	unsigned short icon_id= 0;
	unsigned short tmp_addr= oram_addr;
	
	struct res_tile_t tile_s;

	if(id == 0xFFFF)
		return;

#ifdef TW_WG_DEBUG_MODE
	show_resource_header(id);
#endif	
	
	get_res_tile(id, &tile_s);
	spi_dma2xram(tile_s.base_addr, (unsigned short)(&icon_id), sizeof(unsigned short));
	if(icon_id==0) {
		//INFO(("This Cangeable Tables!\n\r"));

		if((CUR_ICONSET_NUM>=tile_s.length)||(CUR_ICONSET_NUM<=0))
			CUR_ICONSET_NUM= 1;

		spi_dma2xram((tile_s.base_addr+(2*CUR_ICONSET_NUM)), (unsigned short)(&icon_id), sizeof(unsigned short));
		get_res_tile(icon_id, &tile_s);
	}

	for(i=0; i<tile_s.length; i++) {
		spi_dma2xram((tile_s.base_addr+(i*2)), (unsigned short)(&icon_id), sizeof(unsigned short));		
		tw_load_icon(icon_id, &tmp_addr);		
	}	
}

/**
 * @func    find_ch_icon
 * @brief   icon ID of mapping function with ICONSET (changeable).
 * @param   id		dynamic of icon ID
 *			tilesID	icon ID belongs to the this menu of tiles ID 
 * @return  0 if not changeable menu or other errors
 */
unsigned short find_ch_icon(unsigned short id, unsigned short tilesID)
{
	char rc= 0;
	unsigned char i= 0;
	unsigned short icon_id= 0;	
	unsigned long tmp_addr= 0;	
	struct res_tile_t tile_s;

	if(id == 0xFFFF)
		return 0;	
	
	/* get menu of tiles structure with tiles ID */
	get_res_tile(tilesID, &tile_s);
	/* get table of first icon ID with base address */
	spi_dma2xram(tile_s.base_addr, (unsigned short)(&icon_id), sizeof(unsigned short));

	if(icon_id==0) {	// this change table if icon ID is '0'

		/* check current of ICONSET */
		if((CUR_ICONSET_NUM>=tile_s.length)||(CUR_ICONSET_NUM<=1)) {
			CUR_ICONSET_NUM= 1;
			return id;	// default ICON SET if '1' or unknow
		}
		
		/* get changeable of defalut table of tiles structure */
		spi_dma2xram(tile_s.base_addr+2, (unsigned short)(&icon_id), sizeof(unsigned short));
		tmp_addr = tile_s.base_addr;
		get_res_tile(icon_id, &tile_s);

		/* search icon ID of order in default table */
		for(i=0; i<tile_s.length; i++) {
			spi_dma2xram((tile_s.base_addr+(i*2)), (unsigned short)(&icon_id), sizeof(unsigned short));		
			if(id == icon_id)
				break;		// find	
		}

		if(i >= tile_s.length) {
			ERROR(("can't find chageable of icon id!\n"));
			return 0;
		}

		/* get current ICON SET of tiles structure */
		spi_dma2xram((tmp_addr+(2*CUR_ICONSET_NUM)), (unsigned short)(&icon_id), sizeof(unsigned short));
		get_res_tile(icon_id, &tile_s);

		/* get mapping of icon ID */
		spi_dma2xram((tile_s.base_addr+(i*2)), (unsigned short)(&icon_id), sizeof(unsigned short));
		return icon_id;

	} else {
	 	ERROR(("this menu not has chageable fucntion!\n"));
		return 0;	
	}	
}

/*
 * Synopsis     void tw_load_tiles(unsigned short id);
 * Description  loading for OSD2 of tiles(ICONs) table			讀取OSD2 tile表
 * Parameters   id	- OSD2 tiles(ICONs) table of resource ID	tile表 ID
 * Return       none
 * NOTE         default ICON(Font) of start address is 0 for firmware
 */
void tw_load_tiles(unsigned short id)
{
	_tw_load_tiles_at(id, 0);
}

/*
 * Synopsis     void tw_load_luts(unsigned short id);
 * Description  loading for OSD2 of LUT table			讀取OSD2 lut表
 * Parameters   id	- OSD2 LUT table of resource ID		lut表 ID
 * Return       none
 * NOTE         include are Main LUT, 2BP remap LUT, BG remap LUT
 */
void tw_load_luts(unsigned short id)
{
	char rc= 0;
	struct res_lut_t lut_s;	
	
#ifdef TW_WG_DEBUG_MODE
	show_resource_header(id);
#endif

	get_res_lut(id, &lut_s);

	osd_wr_lut_tbl(lut_s.lut_addr, (lut_s.main_len*4));	
	osd_wr_2bp_lut_tbl((lut_s.lut_addr+(lut_s.main_len*4)), (lut_s.re_2bp_len*1));	
	osd_wr_bg_lut_tbl((lut_s.lut_addr+(lut_s.main_len*4)+(lut_s.re_2bp_len*1)), (lut_s.re_bg_len*1));
        
}

/*
 * Synopsis     void tw_img_draw(struct tw_img* pimg);
 * Description  setting registers for show image (SPIOSD)	設定img顯示所需暫存器
 * Parameters   pimg	- point of image of struct			img物件指標
 * Return       none
 * NOTE         this function just setting registers,
 *				if SPIOSD of status is disable, 
 *				need change to enable (sosd_enable)
 */
void tw_img_draw(struct tw_img* pimg)
{
	char rc= 0;
	struct res_img_t img_s;
	
#ifdef TW_WG_DEBUG_MODE
	show_resource_header(pimg->imgID);
#endif
	
	get_res_img(pimg->imgID, &img_s);
	
	_sosd_pLUT_addr(img_s.lut_addr);
    _sosd_img_addr(img_s.index_addr);
    _sosd_img_ls(img_s.line_jump);
    _sosd_img_window(pimg->width, pimg->height);
	_sosd_img_loca(pimg->x, pimg->y);

	CUR_BK_ID = pimg->imgID;

} 

/*
 * Synopsis     void tw_spr_draw(struct tw_spr* pspr);
 * Description  setting registers for show sprite (SPIOSD)	設定spr顯示所需暫存器
 * Parameters   pspr	- point of sprite of struct			spr物件指標
 * Return       none
 * NOTE         this function just setting registers,
 *				if SPIOSD-Sprite of status is disable, 
 *				need change to enable (sosd_spr_enable)
 */
void tw_spr_draw(struct tw_spr* pspr)
{
	char rc= 0;
	struct res_spr_t spr_s;
	unsigned long tmpAddr= 0;

#ifdef TW_WG_DEBUG_MODE
	show_resource_header(pspr->sprID);
#endif
		
	get_res_spr(pspr->sprID, &spr_s);

    _sosd_sLUT_addr(spr_s.lut_addr);
	if(pspr->cnt<spr_s.count) {
		tmpAddr = pspr->width*pspr->height;
		tmpAddr *= pspr->cnt;
		tmpAddr += spr_s.index_addr;
	} else if(pspr->cnt>spr_s.count) {
		tmpAddr = (spr_s.index_addr);
		ERROR(("Sprite count is outrange!\n"));
	} else {
		tmpAddr = (spr_s.index_addr);
	}						
    _sosd_spr_addr(tmpAddr);
	_sosd_spr_ls(spr_s.line_jump);
    _sosd_spr_window(pspr->width, pspr->height);
	_sosd_spr_loca(pspr->x, pspr->y); 
	
	CUR_SPR_ID = pspr->sprID;  

}

/*
 * Synopsis     void tw_get_spr_idx(unsigned short id, unsigned long *addr);
 * Description  getting base address of sprite (SPIOSD)		讀取spr物件地址
 * Parameters   id		- SPIOSD-Sprite of resource ID		spr物件ID
 *				*addr	- result was save in this point		spr物件地址(指標)
 * Return       none
 */
void tw_get_spr_idx(unsigned short id, unsigned long *addr)
{
	struct res_spr_t spr_s;
		
	get_res_spr(id, &spr_s);

	*addr = spr_s.index_addr;
}

/*
 * Synopsis     void tw_spr_action(struct tw_spr* pspr);
 * Description  change a sprite. (animation)			切換顯示sprite
 * Parameters   pspr	- point of sprite of struct		sprite物件指標
 * Return       none
 * NOTE			maybe some Sprite of LUT is same
 */
void tw_spr_action(struct tw_spr* pspr)
{	
	unsigned short id = CUR_SPR_ID;
	
	tw_spr_draw(pspr);

	if(pspr->sprID == id)
		sosd_sp_enable(DONT_LOAD);
	else
		sosd_sp_enable(LOAD_LUT);
}

/*
 * Synopsis     void tw_td_draw(struct tw_td* ptd);
 * Description  setting registers for show twbc (SPIOSD)	設定TD暫存器
 * Parameters   ptd	- point of twbc of struct				TD物件指標
 * Return       none
 * NOTE         this function just setting registers,				ㄔㄨㄉㄗㄉㄌ!
 *				if SPIOSD and SPIOSD-TD of status is disable, 
 *				need change to enable (sosd_td_enable, sosd_enable)
 */
void tw_td_draw(struct tw_td* ptd)
{
	char rc= 0;
	struct res_td_t td_s;
	
#ifdef TW_WG_DEBUG_MODE
	show_resource_header(ptd->tdID);
#endif
	
	get_res_td(ptd->tdID, &td_s);
	
	_sosd_tdc_addr(td_s.color_addr);
    _sosd_img_addr(td_s.index_addr);
    _sosd_td_ls(td_s.line_jump);
	_sosd_tdc_h_size(td_s.line_jump);
    _sosd_td_window(ptd->width, ptd->height);
	_sosd_td_loca(ptd->x, ptd->y);
	_sosd_td_spilt_mode_en();

	CUR_BK_ID = ptd->tdID;

}

/*
 * Synopsis     static void tw_fixed_menu2(void);
 * Description  avoided incorrect of display when OSD2 MENU-2 no assigned a space	
 * Parameters   none
 * Return       none
 * NOTE         assigned 2 words space in last RAM for OSD2 MENU-2 
 */
static void tw_fixed_menu2(void)
{
	osd_set_ram_addr(0x3FFE);
	osd_set_ram_data(0x0801);
	osd_set_ram_data(0x0000);	// 0 char
}

/*
 * Synopsis     void tw_menu_draw(struct tw_menu* pmenu);
 * Description  load OSD2 data for show OSD2			載入OSD2資料
 * Parameters   *tw_menu	- point of OSD2 of struct	OSD2物件指標
 * Return       none
 * NOTE         this function just load OSD2 data to OSD RAM,
 *				need disable OSD2 before, 
 *				and change to enable (sosd_enable) when load finish
 */
void tw_menu_draw(struct tw_menu* pmenu)
{   	
	CUR_MENU_P = pmenu;

	get_res_menu(CUR_MENU_P->menuID, &CUR_MENU_RES);
	if(CUR_MENU_P->tofID != 0xffff)
		get_res_tof(CUR_MENU_P->tofID, &CUR_TOF_RES);

    tw_load_oregs(pmenu->regsID);
	tw_load_luts(pmenu->lutsID);
    tw_load_tiles(pmenu->tilesID);
    tw_load_menu(pmenu);

	if(osd_cfg_read(OSD_PT_LUT_BASEADD)==0x01)
		tw_fixed_menu2();

	tw_menu_reset_index();
}

/*
 * Synopsis     unsigned char check_color_config(unsigned short index, 
 *												unsigned char offset,
 *												unsigned char color);
 * Description  check type of color index					確認色盤??之?型
 * Parameters   index	- font of start index of ICON		icon第一?tile之??
 *				offset	- font of offset					icon於tile表之偏移量
 *				color	- color index of ICON of struct		icon色盤??
 * Return       color index (type of in RAM)				icon色盤??(根據bp數不同，進行偏移)
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

#ifdef  VERSION_102
		if(index<0xFF)
			index+= offset;
		else
			index= offset;
#else
        /* add on version v1.04 */
        index+= offset;
#endif			
	
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
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application) 將icon貼上menu(進階函式)
 * Parameters   index		- start font index of ICON							icon起始tile??
 *				color		- color index of ICON								icon色盤??
 * 				x			- x of coordinate of OSD2-MENU (unit is font)		menu X軸位置(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)		menu Y軸位置(tile)
 *              width		- width of ICON (unit is font)						icon ?度(tile) 						
 *              height		- height of ICON (unit is font)						icon 高度(tile)
 *              menu_addr	- base address of OSD2-MENU							menu oram地址
 *              menu_width	- width of OSD2-MENU (unit is font)					menu ?度(tile)
 *				offset		- font index of offset (for double buffer, etc.)	tile 偏移量
 *				blinking	- 0: none, 1: blinking								閃爍
 * Return       none
 * NOTE         OSD2-MENU of every ROW of char count need same
 */
void _icon_draw_at(unsigned short index, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height,
		 unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking)
{
	unsigned char w,h = 0;
	unsigned short i = 0, temp = 0;

#ifdef EX_I2C_SAFE_MODE
	IC_WRITBYTE(TWIC_P0, 0xF0, SLAVE_SAFE_ADDR);
#endif
	i=0;

	color <<= 1;	// lut index[8:5]
	if(blinking)
		color |= 0x20;

    for(h=0; h< height; h++) {
        osd_set_location_addr(x, y+h,  menu_addr, menu_width);
		for(w=0; w< width; w++) {
			temp = index+i+offset;
			temp |= (color<<8);
			osd_set_ram_data(temp);
			i++;
        }
    }
#ifdef EX_I2C_SAFE_MODE	
	IC_WRITBYTE(TWIC_P0, 0xF0, SLAVE_WORK_ADDR);
#endif
}

/*
 * Synopsis     void _tw_icon_draw_at(struct tw_icon* picon,
 *									unsigned short menu_addr,
 *									unsigned char menu_width,
 *									unsigned short offset,
 *									unsigned char blinking);
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)	將icon貼上osd2 menu(進階函式)
 * Parameters   menu_addr	- base address of OSD2-MENU							menu oram 地址
 *              menu_width	- width of OSD2-MENU (unit is font)					menu ?度(tile)
 *				offset		- font index of offset (for double buffer, etc.)	icon??於tile表中之偏移量
 *				blinking	- 0: none, 1: blinking								閃爍
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
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)	將icon貼上osd2 menu(進階函式)	
 * Parameters   index		- start font index of ICON						icon 起始tile??
 *				color		- color index of ICON							icon 色盤??
 * 				x			- x of coordinate of OSD2-MENU (unit is font)	x軸位置(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)	y軸位置(tile)
 *              width		- width of ICON (unit is font)					icon ?度(tile) 
 *              height		- height of ICON (unit is font)					icon 高度(tile)
 *				blinking	- 0: none, 1: blinking							閃爍
 * Return       none
 */
void _icon_draw(unsigned short index, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char blinking)
{
	_icon_draw_at(index, color, x, y, width, height, CUR_MENU_P->addr, CUR_MENU_P->width, 0, blinking);
}

/*
 * Synopsis     void tw_icon_color_change(struct tw_icon* picon,
 *										unsigned char color);
 * Description  change color index of ICON						切換icon之色盤??
 * Parameters   *tw_icon	- point of icon struct				icon物件指標
 *				color		- want to change of color index		欲更換之色盤??
 * Return       none
 * NOTE         for 1BP/ROM Font
 */
void tw_icon_color_change(struct tw_icon* picon, unsigned char color)
{
	_icon_draw_at(picon->index, color, picon->x, picon->y, picon->width, picon->height, CUR_MENU_P->addr, CUR_MENU_P->width, 0, 0);
}

/*
 * Synopsis     void tw_icon_1st_color(struct tw_icon* picon);
 * Description  change to 1st color of ICON				切換至第一組色盤
 * Parameters   *tw_icon	- point of icon struct		icon物件指標
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icon_1st_color(struct tw_icon* picon)
{
	tw_icon_draw(picon);
}

/*
 * Synopsis     void tw_icon_2nd_color(struct tw_icon* picon);
 * Description  change to 2nd color of ICON				切換至第二組色盤
 * Parameters   *tw_icon	- point of icon struct		icon物件指標
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icon_2nd_color(struct tw_icon* picon)
{
	_icon_draw_at(picon->index, ((picon->color)>>4), picon->x, picon->y, picon->width, picon->height, CUR_MENU_P->addr, CUR_MENU_P->width, 0, 0);
}

/*
 * Synopsis     void tw_icon_draw(struct tw_icon* picon);
 * Description  show ICON(OSD2)							顯示icon於menu上
 * Parameters   *tw_icon	- point of icon struct		icon物件指標
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICON
 */
void tw_icon_draw(struct tw_icon* picon)
{   
#ifndef NO_SUPPORT_1BP_ROM_FONT
	unsigned char color= 0;

	color = check_color_config(picon->index, 0, picon->color);

	_icon_draw(picon->index, color, picon->x, picon->y, picon->width, picon->height, 0);
#else
	_icon_draw(picon->index, (picon->color&0x0F), picon->x, picon->y, picon->width, picon->height, 0);	
#endif
}

unsigned short check_gicon_id (unsigned short id)
{
	unsigned char type = 0;
	unsigned short icon_id = 0;
	struct res_tile_t tile_s;
	
	type = rc_type(id);
	if(type == TYPE_TILE) {
        /* get current ICON SET of group ICON */
        dbg(2, ("Group ICON + Changeable!\n\r"));
        get_res_tile(id, &tile_s);
        if((CUR_ICONSET_NUM>tile_s.length)||(CUR_ICONSET_NUM<=0))
			CUR_ICONSET_NUM= 1;
		spi_dma2xram((tile_s.base_addr+(2*(CUR_ICONSET_NUM-1))), (unsigned short)(&icon_id), sizeof(unsigned short));	    
    } else if(type == TYPE_ICON) {
        icon_id = id;
    } else {
        ERROR(("group icon of ID type is error!\n\r"));
        return NULL;    
    }
    
    return icon_id;
}

/*
 * Synopsis     void tw_gicon_draw(struct tw_gicon* pgicon);
 * Description  show ICON(OSD2)							顯示icon於menu上
 * Parameters   *tw_icon	- point of icon struct		icon物件指標
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICON
 */
void tw_gicon_draw(struct tw_gicon* pgicon)
{
	unsigned short icon_id = 0;
	unsigned char color = 0;
	unsigned short tmp_addr = 0;
	
    icon_id = check_gicon_id(pgicon->gicon_id);
    if(!icon_id)
        return;
    
    tmp_addr = pgicon->oram_addr;
    /* dynamic load ICON to OSDRAM */
    tw_load_icon(icon_id, &tmp_addr);

	color = check_color_config(pgicon->index, 0, pgicon->color);
	_icon_draw(pgicon->index, color, pgicon->x, pgicon->y, pgicon->width, pgicon->height, 0);
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
 * Description  writting ICONS(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)	將icons貼上osd2 menu(進階函式)
 * Parameters   *table		- point of font index of table					icons index表指標
 *				color		- color index of ICON							色盤??
 * 				x			- x of coordinate of OSD2-MENU (unit is font)	x軸位置(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)	y軸位置(tile)
 *              width		- width of ICON (unit is font) 					icon?度
 *              height		- height of ICON (unit is font)					icon高度
 *              menu_addr	- base address of OSD2-MENU						menu oram 位址
 *              menu_width	- width of OSD2-MENU (unit is font)				memu ?度(tile)
 *				offset		- font index of offset							index於tile表之偏移量
 *				blinking	- 0: none, 1: blinking							閃爍
 * Return       none
 * NOTE         OSD2-MENU of every ROW of char count need same
 */
void _icons_draw_at(unsigned char *table, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height,
		 unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking)
{
	unsigned char w,h = 0;
	unsigned short i = 0, temp = 0;

#ifdef EX_I2C_SAFE_MODE
	IC_WRITBYTE(TWIC_P0, 0xF0, SLAVE_SAFE_ADDR);
#endif
	i=0;

	color <<= 1;	// lut index[8:5]
	if(blinking)
		color |= 0x20;

    for(h=0; h< height; h++) {
        osd_set_location_addr(x, y+h,  menu_addr, menu_width);
		for(w=0; w< width; w++) {
			temp = table[i];
			if(temp==0xFF)	/* for Blank ROM Font */
				temp= 0;
			else {
				temp = table[i]+offset;
				temp |= (color<<8);
			}
			osd_set_ram_data(temp);
			i++;
        }
    }
#ifdef EX_I2C_SAFE_MODE	
	IC_WRITBYTE(TWIC_P0, 0xF0, SLAVE_WORK_ADDR);
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
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)	將icons貼上osd2 menu(進階函式)
 * Parameters   x			- x of coordinate of OSD2-MENU (unit is font)	x軸位置(tile)
 *              y			- y of coordinate of OSD2-MENU (unit is font)	y軸位置(tile)
 *              width		- width of ICON (unit is font) 					icon?度(tile)	
 *              height		- height of ICON (unit is font)					icon高度(tile)
 *				*table		- point of font index of table					icon index表指標
 *				color		- color index of ICONS							色盤??	
 *				offset		- font index of offset							index於tile表中之偏移量
 *				blinking	- 0: none, 1: blinking							閃爍
 * Return       none
 */
void _icons_draw(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *table, unsigned char color, unsigned short offset, unsigned char blinking)
{
#ifndef NO_SUPPORT_1BP_ROM_FONT
	unsigned char ccolor= 0;

	ccolor = check_color_config(table[0], (SUPPORT_1BP_ROM?offset:0), color);

	_icons_draw_at(table, ccolor, x, y, width, height, CUR_MENU_P->addr, CUR_MENU_P->width, offset, blinking);
#else
	_icons_draw_at(table, (color&0x0F), x, y, width, height, CUR_MENU_P->addr, CUR_MENU_P->width, offset, blinking);
#endif
}

/*
 * Synopsis     void _tw_icons_draw_at(struct tw_icon* picon,
 *									unsigned short menu_addr,
 *									unsigned char menu_width,
 *									unsigned short offset,
 *									unsigned char blinking);
 * Description  writting ICON(Font) of index(font, color) at OSD2-MENU(OSD RAM) (advanced application)	將icon貼上osd2 menu(進階函式)
 * Parameters   menu_addr	- base address of OSD2-MENU							menu oram 位置
 *              menu_width	- width of OSD2-MENU (unit is font)					menu ?度
 *				offset		- menu_addr of offset (for double buffer, etc.)		menu 偏移量
 *				blinking	- 0: none, 1: blinking								閃爍
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
 * Synopsis     void tw_icons_color_change(struct tw_icons* picon,
 *										unsigned char color);
 * Description  change color index of ICONS						切換icons色盤
 * Parameters   *tw_icons	- point of icon struct				icons物件指標
 *				color		- want to change of color index		欲切換之色盤??
 * Return       none
 * NOTE         for 1BP/ROM Font
 */
void tw_icons_color_change(struct tw_icons* picon, unsigned char color)
{
	_icons_draw_at(picon->table, color, picon->x, picon->y, picon->width, picon->height, CUR_MENU_P->addr, CUR_MENU_P->width, (SUPPORT_1BP_ROM?picon->offset:0), 0);
}

/*
 * Synopsis     void tw_icons_1st_color(struct tw_icons* picon);
 * Description  change to 1st color of ICONS			設定icons為第一組色盤
 * Parameters   *tw_icons	- point of icon struct		icons物件??
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icons_1st_color(struct tw_icons* picon)
{
	tw_icons_draw(picon);
}

/*
 * Synopsis     void tw_icons_2nd_color(struct tw_icons* picon);
 * Description  change to 2nd color of ICONS			設定icons為第二組色盤
 * Parameters   *tw_icons	- point of icon struct		icons物件??
 * Return       none
 * NOTE         for 2BP/4BP Font
 */
void tw_icons_2nd_color(struct tw_icons* picon)
{
	_icons_draw_at(picon->table, ((picon->color)>>4), picon->x, picon->y, picon->width, picon->height, CUR_MENU_P->addr, CUR_MENU_P->width, (SUPPORT_1BP_ROM?picon->offset:0), 0);
}

/*
 * Synopsis     void tw_gicons_draw(struct tw_gicons* pgicons);
 * Description  show ICON(OSD2)							顯示icon於menu上
 * Parameters   *tw_icon	- point of icon struct		icon物件指標
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICON
 */
void tw_gicons_draw(struct tw_gicons* pgicon)
{
	unsigned char icon_id = 0;
	unsigned short tmp_addr = 0;
	
    icon_id = check_gicon_id(pgicon->gicons_id);
    if(!icon_id)
        return;
    
    tmp_addr = pgicon->oram_addr;
    /* dynamic load ICON to OSDRAM */
    tw_load_icon(icon_id, &tmp_addr);

	_icons_draw(pgicon->x, pgicon->y, pgicon->width, pgicon->height, pgicon->table, 
				pgicon->color, (SUPPORT_1BP_ROM?pgicon->offset:0), 0);
}

/*
 * Synopsis     void tw_icons_draw(struct tw_icons* picon);
 * Description  show ICONS(OSD2)						顯示icons於menu上
 * Parameters   *tw_icons	- point of icon struct		icons物件指標
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show ICONS
 */
void tw_icons_draw(struct tw_icons* picon)
{
	_icons_draw(picon->x, picon->y, picon->width, picon->height, picon->table, 
				picon->color, (SUPPORT_1BP_ROM?picon->offset:0), 0);
}

#define FONT_ASCII_OFFSET	10
void _romfont_draw_at(struct tw_txt* ptxt, unsigned char x, unsigned char y, const unsigned char *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color, unsigned char blink)
{
	unsigned char i;
	unsigned short temp= 0;
	
	if(blink)
		color |= 0x20;
							
   	osd_set_location_addr(x, y, menu_addr, menu_width);
	
	for(i=0; (ucs[i]!=0) && (i < 255); i++) {
		if((x+i)>=CUR_MENU_P->width) 
			break;
		if(i>=ptxt->width)
			break;
		temp = (color<<9);
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
}

void _romfont_draw(struct tw_txt* ptxt, unsigned char x, unsigned char y, const unsigned char *ucs, unsigned char color, unsigned char blink)
{							
   	_romfont_draw_at(ptxt, x, y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, color, blink);	 
}

void tw_romfont_draw(struct tw_txt* ptxt, const unsigned char *ucs)
{							
   	_romfont_draw_at(ptxt, ptxt->x, ptxt->y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, ((ptxt->fg_color)<<0) + ((ptxt->bg_color)<<5), 0);	 
}
 

/* T123 OSD Font Drawing Info Table         24x24 32x31 32x36 48x48(16x24) 48x48(12x24)  rev */
unsigned short code TOF_FONT_SPLIT_W[]    = { 2,    2,    2,    3,           4,           1};
unsigned short code TOF_FONT_SPLIT_H[]    = { 1,    1,    2,    2,           2,           2};
unsigned short code TOF_FONT_SPLIT_HSA[]  = { 1,    1,    1,    2,           2,           1};

#define SINGLE_PIX 0
#define SUB_PIX    1 

#define GB_ERROR_CODE 0xFFFF

unsigned char _tw_half_alphabet_num()
{
	return spi_read_byte(CUR_TOF_RES.base_addr + 0x54L);
}

unsigned char _tw_get_font_size_id()
{
	return spi_read_byte(CUR_TOF_RES.base_addr + 0x55L);	
}

unsigned char _tw_get_file_type() {
	return spi_read_byte(CUR_TOF_RES.base_addr + 0x01L);
}

unsigned short _Unicode_len(unsigned short *ucs) {
    unsigned short i;

	for(i=0; ucs[i]!= 0 && i<0xffff; i++) {	
	}

	return i;
}

unsigned short _GB_code_len(unsigned short *ucs) {
	unsigned char *gbs = ucs;
	unsigned short len = 0;
	int i;

	for(i=0, len=0; gbs[i]!=0 && len<GB_ERROR_CODE; len++) {
		if(gbs[i] >= 0xA1) { 
			if(gbs[i+1] < 0xA0) {
				len = GB_ERROR_CODE - 1;
			}
			i+=2;
		}
		else {	
			i++;
		}		
	}

	if(len == GB_ERROR_CODE) { 
		ERROR(("Fail To Get GB String Length!\n"));
	}
		 
	return len;
}

unsigned short _get_offset_GBS(unsigned short *ucs, unsigned short n) {	 
	unsigned char *gbs = ucs;
	int i, idx;

	for(i=0, idx = 0;  gbs[idx]!=0 && i<0xFFFF; i++) { 
		if(i == n) {
			break;	
		}
		if(gbs[idx] >= 0xA1) {  
			idx+=2;
		}
		else {
			idx++;
		}		
	}
	
	if(i == 0xFFFF) {  
		idx = GB_ERROR_CODE;
		ERROR(("Fail To Get GB String Offset!\n"));
	}	  

	return idx;
}

unsigned short _get_code_from_GBS(unsigned short *ucs, unsigned short n) {	 
	unsigned char *gbs = ucs;
	unsigned short cod = GB_ERROR_CODE;
	int i, idx;

	for(i=0, idx=0; gbs[idx]!=0 && i<0xFFFF; i++) { 
		if(i == n) {
			if(gbs[idx] >= 0xA1) {  
				cod = gbs[idx+1L];
				cod += (gbs[idx]<<8L);
			}
			else {				
				cod = gbs[idx];
			}	
			break;	
		}
		if(gbs[idx] >= 0xA1) {  
			idx+=2;
		}
		else {
			idx++;
		}		
	}
	
	if(cod == GB_ERROR_CODE) {
		ERROR(("Fail To Get GB Code!\n"));
	}	  

	return cod;
}


unsigned char _tw_get_font_quality()
{	
	return spi_read_byte(CUR_TOF_RES.base_addr + 0x50L);
}


void _twfont_draw_at(struct tw_txt* ptxt, unsigned char x, unsigned char y, unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color, unsigned char blink)
{
	unsigned char size_id, font_quality;	
							   
	font_quality = _tw_get_font_quality();
	size_id      = _tw_get_font_size_id();	
	tw_menu_reset_index_n(ptxt->index);	
	tw_menu_draw_nstring(x, y, ptxt->width, ucs, menu_addr, menu_width, (blink?(color|0x20):color), size_id, font_quality!=SINGLE_PIX);	  
}

#define TOF_DIST_ERROR_CODE              0
#define TOF_DIST_BASIC_LATIN_START_CODE  0x0021 
#define TOF_DIST_BASIC_LATIN_END_CODE    0x007F
	 											 
#define TOF_DIST_LATIN_1_START_CODE      0x00A1
#define TOF_DIST_LATIN_1_END_CODE        0x00FF

#define TOF_DIST_CYRILLIC_START_CODE     0x0400
#define TOF_DIST_CYRILLIC_END_CODE       0x051D

unsigned short _check_and_get_idx(unsigned short uc) {
	if(uc >= TOF_DIST_BASIC_LATIN_START_CODE && uc <= TOF_DIST_BASIC_LATIN_END_CODE)
		return uc - TOF_DIST_BASIC_LATIN_START_CODE;
	else
		return TOF_DIST_ERROR_CODE;													 	
}

void _twfont_2bppic_st_draw_at(struct tw_txt* ptxt, unsigned char x, unsigned char y, unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color)
{
	unsigned char i, j;
	unsigned short index;
	unsigned char draw_offset = 0;

	for(i=0; i<ptxt->height; i++) {
		for(j=0; j<ptxt->width && ucs[draw_offset]!= 0; j++, draw_offset++) {	
			index = _check_and_get_idx(ucs[draw_offset]) + ptxt->index;						 					 
			osd_menu_write(x+j, y+i, index, color, menu_addr, menu_width);	
		}	 	 
	}
}
  
#define ID_TEXT_ROM          0	
#define ID_TEXT_PIC_2BP 0xFFFF	
//#define ID_TEXT_TOF_2BP 0xFFFE	 
//#define ID_TEXT_TOF_1BP    1  

void _twfont_draw(struct tw_txt* ptxt, unsigned char x, unsigned char y, unsigned short *ucs, unsigned char color, unsigned char blink)
{	
	if(ptxt->icon_id == ID_TEXT_PIC_2BP)
		_twfont_2bppic_st_draw_at(ptxt, x, y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, ((ptxt->fg_color)<<1));	
	else
		_twfont_draw_at(ptxt, x, y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, color, blink);	  
}

void tw_twfont_draw(struct tw_txt* ptxt, unsigned short *ucs)
{	   
	if(ptxt->icon_id == ID_TEXT_PIC_2BP)
		_twfont_2bppic_st_draw_at(ptxt, ptxt->x, ptxt->y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, ((ptxt->fg_color)<<0));	  
	else
		_twfont_draw_at(ptxt, ptxt->x, ptxt->y, ucs, CUR_MENU_P->addr, CUR_MENU_P->width, ((ptxt->fg_color)<<0) + ((ptxt->bg_color)<<5), 0);	  
}

unsigned char _tw_draw_char_at(unsigned char x, unsigned char y, unsigned char w, unsigned h, 
				          unsigned short idx, unsigned char color, unsigned short menu_addr, unsigned char menu_width)
{
	unsigned char i, j;

	for(j=0; j<w; j++) {  
		for(i=0; i<h; i++) {
			osd_menu_write(x+j, y+i, idx+i+j*h, color, menu_addr, menu_width);			
		}
	}

	return w*h;
}

void _tw_burst_draw_line(unsigned char x, unsigned char y, unsigned char w, unsigned h, 
				          unsigned short idx, unsigned char color, unsigned short menu_addr, unsigned char menu_width)
{
	unsigned char i, j;
	unsigned short dat;
						  
	for(i=0; i<h; i++) {		
		osd_set_location_addr(x, y+i, menu_addr, menu_width);
		for(j=0; j<w; j++) { 
			dat = (color<<9) + idx + j*h + i; 
			OSD_SET_RAM_DATA(dat);	
		}
	}
}

unsigned short tw_menu_draw_nstring(unsigned char x, unsigned char y, unsigned char max_w, const unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, 
                                       unsigned char color, unsigned char size_id, unsigned char is2BP)
{
	unsigned short i;
	unsigned short font_size;  
	unsigned char is_hsa = (_tw_half_alphabet_num() > 0);
	unsigned short real_oaddr;
	unsigned char bp = is2BP ? 2 : 1;

	unsigned char is_gb = ((_tw_get_file_type()=='G') ? 1 : 0);	  
	unsigned short len = ((is_gb) ? _GB_code_len(ucs) : _Unicode_len(ucs));
	unsigned short uc;
		
	unsigned char w_count = 0;
	unsigned char w_sum = 0;
	unsigned short temp_idx = menu_string_start_index;

	font_size = (CUR_MENU_P->font_w*CUR_MENU_P->font_h)/16;
					
   	for(i=0; (i<len)&&(w_sum<=max_w); i++) { 
		uc = ((is_gb) ? _get_code_from_GBS(ucs, i) : ucs[i]);
		real_oaddr = (is2BP) ? (CUR_2BP_IDX*font_size + (menu_string_start_index-CUR_2BP_IDX)*font_size*2) : font_size*menu_string_start_index;  
	    if(is_hsa) {
			w_count = osd_put_twf_char_to_oram(CUR_TOF_RES.base_addr, real_oaddr, uc, TOF_FONT_SPLIT_H[size_id]*font_size*2*bp, is_hsa);  
			menu_string_start_index += w_count* TOF_FONT_SPLIT_H[size_id]; 
			w_sum += w_count;	
		}
		else {																				   
			osd_put_twf_char_to_oram(CUR_TOF_RES.base_addr, real_oaddr, uc, font_size*TOF_FONT_SPLIT_W[size_id]*TOF_FONT_SPLIT_H[size_id]*2*bp, is_hsa); 
			menu_string_start_index += TOF_FONT_SPLIT_W[size_id]*TOF_FONT_SPLIT_H[size_id]; 
			w_sum += TOF_FONT_SPLIT_W[size_id];
		}
	}
	
	if(w_sum > max_w) {
		w_sum = max_w;
	}	
	 			
	_tw_burst_draw_line(x, y, w_sum, TOF_FONT_SPLIT_H[size_id], temp_idx, color, menu_addr, menu_width);
	
	return i;	
}

void tw_menu_row_moveV(unsigned char row, unsigned short y)
{
	unsigned short dat = 0x4000 + y;
	unsigned short address = CUR_MENU_P->addr + ((CUR_MENU_P->width + 4) * row); 
												
	IC_WRITBYTE(0x00, 0xAA, (unsigned char)(address & 0xff));
	IC_WRITBYTE(0x00, 0xAA, (unsigned char)(address>>8));
				  
	//osd_set_location_addr(x, y, MENU_BASE_ADDRESS_TBL[CUR_PAGE_ID-1][1], CUR_MENU_P->width);
	osd_set_ram_data(dat);
}

void tw_menu_reset_index()
{
	menu_string_start_index = 0;
}

void tw_menu_reset_index_n(unsigned short idx)
{
	menu_string_start_index = idx;	
}

void tw_menu_reset_index_rev_nums()
{
	menu_string_start_index = 20;	
}

void tw_menu_nums_init()
{
	unsigned char i;
	unsigned short font_size;

	font_size = CUR_TOF_RES.font_width*CUR_TOF_RES.font_height/16;
							
   	for(i=0; i<10; i++) {	 
		osd_put_twf_char_to_oram(CUR_TOF_RES.base_addr, font_size*i, i+48, font_size*2, 0); 
	}
	menu_string_start_index = 20;	 
	
}
 
/*
 * Synopsis     void tw_text_draw(struct tw_txt* ptxt, unsigned short *ucs);
 * Description  show ICON(OSD2)								繪製txt至menu
 * Parameters   *ptxt	- point of txt struct				txt物件指標
 *              *ucs	- start point of string (utf-8)		字串指標(utf-8)	
 * Return       none
 * NOTE         OSD2 need init and load data (tw_menu_draw) before the show text
 *              text types include twfont and ROMFont
 */
void tw_text_draw(struct tw_txt* ptxt, unsigned short *ucs)
{
//#ifndef ENABLE_EX_MCU
//	IC_WritByte(TWIC_P1,0xFE,0xAD);
//#endif
		
	if(ptxt->icon_id==0) {
		/* ROM FONT */
		tw_romfont_draw(ptxt, (unsigned char*)ucs);
	} else {
		/* TWFONT */
		tw_twfont_draw(ptxt, ucs);	
	}
//#ifndef ENABLE_EX_MCU
//	IC_WritByte(TWIC_P1,0xFE,0xA5);
//#endif	
}

/*
 * Synopsis     void tw_text_color_change(struct tw_txt* ptxt,
 *                                      unsigned short *ucs,
 *										unsigned char color);
 * Description  change color index of ICONS					切換字型之色盤??
 * Parameters   *ptxt	- point of txt struct				txt物件指標
 *              *ucs	- start point of string (utf-8)		字串指標(utf-8)
 *				color	- want to change of color index		欲切換之色盤??
 * Return       none
 * NOTE         for text (1BP/ROM Font)
 */
void tw_text_color_change(struct tw_txt* ptxt, unsigned short *ucs, unsigned char color)
{
	if(ptxt->icon_id==0) {
		/* ROM FONT */
		_romfont_draw(ptxt, ptxt->x, ptxt->y, (unsigned char*)ucs, color, 0);	 
	} else {
		/* TWFONT */
		_twfont_draw(ptxt, ptxt->x, ptxt->y, ucs, color, 0);	  
	}
}

/*
 * Synopsis     void tw_osd_menu_write(unsigned char x,
 *									unsigned char y,
 *									unsigned short index,
 *									unsigned char color);
 * Description  writting are font index and color index at OSD2-MENU(OSD RAM) (advanced application)	?入字型/色盤??至mrenu特定位置(進階函數)
 * Parameters   x		- x of coordinate of OSD2-MENU (unit is font)		x軸位置 (tile)
 *              y		- y of coordinate of OSD2-MENU (unit is font)		y軸位置 (tile)
 *              index	- font of index										tile ?? 
 *              color	- color of index									色盤 ??
 * Return       none
 */
void tw_osd_menu_write(unsigned char x, unsigned char y, unsigned short index, unsigned char color)
{
	osd_menu_write(x, y, index, color, CUR_MENU_P->addr, CUR_MENU_P->width);	
}

/*
 * Synopsis     void tw_osd_menu_write_line(unsigned char y,
 *									unsigned short index,
 *									unsigned char color);
 * Description  writting are font index and color index at OSD2-MENU(OSD RAM)	?入字型/色盤??至menu特定行(進階函數) 
 *              and a line (menu_width)
 * Parameters   y		- y of coordinate of OSD2-MENU (unit is font)	y軸位置(tile)
 *              index	- font of index 								tile ??
 *              color	- color of index								色盤 ??
 * Return       none
 */
void tw_osd_menu_write_line(unsigned char y, unsigned short index, unsigned char color)
{
	osd_menu_write_line(y, index, color, CUR_MENU_P->addr, CUR_MENU_P->width);
}

/*
 * Synopsis     void tw_osd_clear_line(unsigned char y);
 * Description  clear(writting a 0) a line (menu_width) at OSD2-MENU(OSD RAM)	清除menu特定行
 * Parameters   y	- y of coordinate of OSD2-MENU (unit is font)				y軸位置(tile)
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_osd_clear_line(unsigned char y)
{
	osd_clear_line(y, CUR_MENU_P->addr, CUR_MENU_P->width);
}

/*
 * Synopsis     void tw_osd_clear_char(unsigned char x, unsigned char y);
 * Description  clear(writting a 0) a char at OSD2-MENU(OSD RAM)		清除menu特定tile
 * Parameters   x	- x of coordinate of OSD2-MENU (unit is font)		x軸位置(tile)
 *              y	- y of coordinate of OSD2-MENU (unit is font)		y軸位置(tile)
 * Return       none
 * NOTE         default(must) blank font of index is 0
 *              just clear to blank font (LSB), no clear color index (MSB)
 */
void tw_osd_clear_char(unsigned char x, unsigned char y)
{
	//osd_clear_char(x, y, CUR_MENU_P->addr, CUR_MENU_P->width);
	osd_clear_block(x, y, 1, 1, CUR_MENU_P->addr, CUR_MENU_P->width);
}

/*
 * Synopsis     void tw_clear_icon(struct tw_icon* picon);
 * Description  clear(writting a 0) a ICON at OSD2-MENU(OSD RAM)	清除menu上某icon
 * Parameters   *tw_icon    - point of icon struct					icon物件指標
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_icon(struct tw_icon* picon) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, CUR_MENU_P->addr, CUR_MENU_P->width);
}

/*
 * Synopsis     void tw_clear_icons(struct tw_icons* picon);
 * Description  clear(writting a 0) a ICONS at OSD2-MENU(OSD RAM)	清除menu上某icons
 * Parameters   *tw_icons    - point of icons struct				icons物件指標
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_icons(struct tw_icons* picon) {
	osd_clear_block(picon->x, picon->y, picon->width, picon->height, CUR_MENU_P->addr, CUR_MENU_P->width);
}

/*
 * Synopsis     void tw_clear_txt(struct tw_txt* ptxt);
 * Description  clear(writting a 0) a txt at OSD2-MENU(OSD RAM)	清除menu上某txt
 * Parameters   *ptxt   - point of txt struct					txt物件指標
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_clear_txt(struct tw_txt* ptxt) {
	osd_clear_block(ptxt->x, ptxt->y, ptxt->width, ptxt->height, CUR_MENU_P->addr, CUR_MENU_P->width);
}

/*
 * Synopsis     void _tw_clear_icon_at(struct tw_icon* picon,
 *                                  unsigned short menu_addr,
 *                                  unsigned char menu_width);
 * Description  clear(writting a 0) a ICON at OSD2-MENU(OSD RAM) (advanced application)		清除menu上某icon(進階函式)
 * Parameters   *picon    - point of icon struct					icon物件指標
 *              menu_addr	- base address of OSD2-MENU				menu oram 地址
 *              menu_width	- width of OSD2-MENU (unit is font)		menu ?度(tile)
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
 * Description  clear(writting a 0) a ICONS at OSD2-MENU(OSD RAM) (advanced application)	清除menu上某icons(進階函式)
 * Parameters   *picons    - point of icons struct					icons物件指標
 *              menu_addr	- base address of OSD2-MENU				menu oram 地址
 *              menu_width	- width of OSD2-MENU (unit is font)		menu ?度(tile)
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
 * Description  clear(writting a 0) a txt at OSD2-MENU(OSD RAM) (advanced application)	清除menu上某txt(進階函式)
 * Parameters   *ptxt       - point of txt struct					txt物件指標
 *              menu_addr	- base address of OSD2-MENU				menu oram 地址
 *              menu_width	- width of OSD2-MENU (unit is font)		menu ?度(tile)
 * Return       none
 * NOTE         default(must) blank font of index is 0
 *              can assigned base address and width of OSD2-MENU
 */
void _tw_clear_txt_at(struct tw_txt* ptxt, unsigned short menu_addr, unsigned char menu_width) {
	osd_clear_block(ptxt->x, ptxt->y, ptxt->width, ptxt->height, menu_addr, menu_width);
}

/*
 * Synopsis     void tw_osd_clear_menu(void);
 * Description  clear all char(0) at OSD2-MENU(OSD RAM)		清除menu
 * Parameters   none
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_osd_clear_menu(void)
{
	osd_clear_menu(CUR_MENU_P->addr, CUR_MENU_P->width, CUR_MENU_P->height);
}

/*
 * Synopsis     void tw_menu_fill(unsigned char x,
 *                                unsigned char y,
 *                                unsigned char dat,
 *                                unsigned char color,
 *                                unsigned char length);
 * Description  fill a block include dat(LSB)&color(MSB) at OSD2-MENU(OSD RAM)
 * Parameters   x		- x of coordinate of OSD2-MENU (unit is font)
 *              y		- y of coordinate of OSD2-MENU (unit is font)
 *              dat		- font of index
 *              color	- color of index
 *              length	- fill data of length
 * Return       none
 * NOTE         default(must) blank font of index is 0
 */
void tw_menu_fill(unsigned char x, unsigned char y, unsigned char dat, unsigned char color, unsigned char length)
{
	unsigned char i;

	for(i=0; i< length; i++) {
		osd_menu_write(x+i, y, dat, color, CUR_MENU_P->addr, CUR_MENU_P->width);		
	}		 
}

/*
 * Synopsis     void tw_emu_trig(void);
 * Description  emulation trig
 * Parameters   none
 * Return       none
 * NOTE         need load emulation (tw_load_emu) before
 */
void tw_emu_trig(void)
{
	IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_ENABLE);	// for SPIOSD shadow issue
	sosd_emu_trig();
}

/*
 * Synopsis     void tw_load_emu(unsigned short id);
 * Description  setting registers for emulation
 * Parameters   id	- emulation script of resource ID
 * Return       none
 */
void tw_load_emu(unsigned short id)
{
	char rc= 0;
	struct res_emu_t emu_s;
	
#ifdef TW_WG_DEBUG_MODE
	show_resource_header(id);
#endif	
	
	get_res_emu(id, &emu_s);
	
	tw_emu_exit();

	sosd_set_emu_base_addrss(emu_s.base_addr);
	tw_emu_trig();
}

/*
 * Synopsis     void tw_emu_trig(void);
 * Description  waitting emulation script done
 * Parameters   none
 * Return       none
 * NOTE         some emulation scripts was always run, ex. animation,
 *              these emulation should force exit (tw_emu_exit) by fw.
 */
void tw_wait_emu_done(void)
{
	sosd_wait_emu_done();
	IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_DISABLE);	// for SPIOSD shadow issue
}

/*
 * Synopsis     void tw_emu_exit(void);
 * Description  force exit from emulation script
 * Parameters   none
 * Return       none
 */
void tw_emu_exit(void)
{
	sosd_emu_exit();
	IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_DISABLE);	// for SPIOSD shadow issue
}

/*
 * Synopsis     void tw_img_force_alpha(unsigned char en);
 * Description  force image/twbc (SPIOSD) of alpha
 * Parameters   en	- 0/1: disable/enable force alpha
 * Return       none
 */
void tw_img_force_alpha(unsigned char en)
{
	if(en) {
		sosd_img_force_ctrl(64);
		sosd_img_force_enable();
	} else
		sosd_img_force_disable();
}

/*
 * Synopsis     void put_enter_page_event(unsigned char id);
 * Description  put a enter page ID of event to event queue
 * Parameters   id	- page of resource ID
 * Return       none
 */
void put_enter_page_event(unsigned char id)
{					
	TW_EVENT events;					  
	events.type = TW_EVENT_TYPE_ENTER_SYSTEM;
	events.page.id = id;
	events.page.mode = PAGE_EVENT_NORMAL;
	put_event(&events);
}

/*
 * Synopsis     void put_quit_page_event(unsigned char id);
 * Description  put a quit page ID of event to event queue		插入"離開?面事件"進入事件堆疊
 * Parameters   id	- page of resource ID						?面ID
 * Return       none
 */
void put_quit_page_event(unsigned char id)
{
	TW_EVENT events;
	events.type = TW_EVENT_TYPE_QUIT_SYSTEM;
	events.page.id = id;
	events.page.mode = PAGE_EVENT_NORMAL;
	put_event(&events);
}

/*
 * Synopsis     void put_enter_slide_page_event(unsigned char id);
 * Description  put a enter page ID of event to event queue		插入"滑動進入?面事件"進入事件堆疊		
 * Parameters   id	- page of resource ID						?面ID
 * Return       none
 * NOTE         enter page of type is slide for touch slide
 */
void put_enter_slide_page_event(unsigned char id)
{					
	TW_EVENT events;					  
	events.type = TW_EVENT_TYPE_ENTER_SYSTEM;
	events.page.id = id;
	events.page.mode = PAGE_EVENT_SLIDE;
	put_event(&events);
}

/*
 * Synopsis     void put_enter_slide_page_event(unsigned char id);
 * Description  put a quit page ID of event to event queue		插入"滑動離開?面事件"進入事件堆疊
 * Parameters   id	- page of resource ID						?面ID
 * Return       none
 * NOTE         quit page of type is slide for touch slide
 */
void put_quit_slide_page_event(unsigned char id)
{
	TW_EVENT events;
	events.type = TW_EVENT_TYPE_QUIT_SYSTEM;
	events.page.id = id;
	events.page.mode = PAGE_EVENT_SLIDE;
	put_event(&events);
}

/*
 * Synopsis     unsigned char tw_page_process(TW_EVENT* event);
 * Description  call a process function (current of page ID) to processing events	呼叫目前?面之?理函式 
 * Parameters   event	- events (all events) of tw widget system					?面系統送出之事件
 * Return       0: none						沒事
 *              1: not page					此ID非?面
 *              2: no active page			目前無運作中?面
 *              3: no process any event		?面閒置
 *              4: no page					目前ID無對應?面
 */
unsigned char tw_page_process(TW_EVENT* event)
{
	if(event->type==TW_EVENT_TYPE_ENTER_SYSTEM) {
		CUR_PAGE_ID = event->page.id;
	}	
	if(CUR_PAGE_ID==0xFF) {
		dbg(3, ("no active page!\n"));
		return TW_RETURN_NO_ACTIVE;	
	}
	
	return tw_page_handler(CUR_PAGE_ID, event);
}

/*
 * Synopsis     void tw_widget_init(void);
 * Description  init for tw widget system	初始化?面系統
 * Parameters   none
 * Return       none
 * NOTE         enter first page (first page of ID default is 1)
 */
void tw_widget_init(void)
{
	TW_EVENT page_event;
	page_event.type = TW_EVENT_TYPE_ENTER_SYSTEM;
	page_event.page.id = 1; 	// allways first page
	put_event(&page_event);
}

/* Status variable 
 * 		SLIDE_LOCK		    slide of direction
 *		LAYOUT_GROUP_NUM	remember layout of group for performance
 *		SECOND_IMG_ID	    remember second imag/twbc of ID for performance
 *		FIRST_IMG_ADDR		remember 1st imag/twbc of base address for performance
 *		SECOND_IMG_ADDR		remember 2nd imag/twbc of base address for performance
 */
static unsigned char data SLIDE_LOCK= SLIDE_NONE;
static unsigned char 	LAYOUT_GROUP_NUM= 0xFF;
static unsigned short 	SECOND_IMG_ID	= 0x0000;
static unsigned long 	FIRST_IMG_ADDR	= 0x0000;
static unsigned long 	SECOND_IMG_ADDR	= 0x0000;

/*
 * Synopsis     unsigned char tw_img_slide(struct tw_layout* playout,	
 *                                         unsigned char dir,
 *                                         unsigned long offset,
 *                                         unsigned char i,
 *                                         unsigned char j);
 * Description  image of slide effect							img?面滑動效果
 * Parameters   *playout	- point of tw_layout of struct		?面布局結?指標
 * 				dir		    - slide of direction				滑動方向
 *              offset		- slide of offset					滑動量
 *              i		    - number of row in layout			?面行數
 *              j		    - number of column in layout		?面列數
 * Return       0xFE: slide successful							滑動成功
 *              0xFF: slide error								滑動失敗
 *              other value: current stay of number in layout	目前顯示布局物件??
 */
unsigned char tw_img_slide(struct tw_layout* playout, unsigned char dir, unsigned long offset, unsigned char i, unsigned char j)
{
	unsigned char c_w= 0, c_h= 0;
	unsigned long addr= 0, shift= 0;
	struct res_img_t			img_s;
	struct tw_img			*pimg;
	struct tw_layout_item	*playout_item;

	unsigned long temp= 0;

	//offset = (offset/TD_LINE_ALING)*TD_LINE_ALING;	// ALIGN 4
	if(offset<=0)
		return SLIDE_HV_ERROR;

	if(i!=LAYOUT_GROUP_NUM) {
		LAYOUT_GROUP_NUM = i;
		FIRST_IMG_ADDR = 0;
		/* disable sprite & td */
		sosd_td_disable();
		sosd_sp_disable();
	}
		
	/* get current image base address */
	_get_img_idx_addr(&addr);

	/* get first image base address */
	pimg = playout->group[i].item[0].tw_object;
	get_res_img_by_addr(pimg->imgID, &img_s, &FIRST_IMG_ADDR);

	/* calc current of cnt at group */
	shift = addr-img_s.index_addr;
	c_w = ((shift%(img_s.line_jump))/img_s.width);
	c_h = ((shift/(img_s.line_jump))/(img_s.height));	
	c_w = c_w+c_h*playout->group[i].width;
    
    /* get current of ID & Object */
	pimg = playout->group[i].item[c_w].tw_object;
	if(pimg->imgID!=SECOND_IMG_ID) {
		SECOND_IMG_ID = pimg->imgID;
		SECOND_IMG_ADDR = 0;
	}
	get_res_img_by_addr(pimg->imgID, &img_s, &SECOND_IMG_ADDR);

    shift = addr-img_s.index_addr;
	
	switch(dir) {
		case DIR_LEFT:	// left dir of slide
			/* check next image if current address == n image start address */
			if((addr==img_s.index_addr)) {
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
							get_res_img(pimg->imgID, &img_s);
							_sosd_sLUT_addr(img_s.lut_addr);
							_sosd_load_slut();
							_sosd_swtc_offset(img_s.width);
							_sosd_swtc_h_mode();
							_sosd_swtc_enable();
							sosd_update();
							_sosd_swtc_offset(img_s.width);
						}
					}		
				} else {
					/* Load sLUT */
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].right;
					pimg = playout_item->tw_object;
					get_res_img(pimg->imgID, &img_s);
					_sosd_sLUT_addr(img_s.lut_addr);
					_sosd_load_slut();					
					_sosd_swtc_h_mode();
					_sosd_swtc_enable();
					sosd_update();
					_sosd_swtc_offset(img_s.width);
				}
			} 
			/* next page */
			if((((shift)%(img_s.line_jump))+offset)
				>=img_s.width) {
				_sosd_swtc_disable();
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].right;
				pimg = playout_item->tw_object;
				tw_img_draw(pimg);
				sosd_enable(LOAD_LUT);
				CUR_BK_ID= pimg->imgID;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* left slide */
			else {
				_sosd_img_addr(addr+offset);
				addr = offset;
				_get_swtc_offset((unsigned short*)&offset);
				offset >>= 16;				
				sosd_update();
				_sosd_swtc_offset(offset-addr);
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_RIGHT:	// right dir of slide
			/* check next image if current address == n image start address */
			if((addr==img_s.index_addr)) {
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
							get_res_img(pimg->imgID, &img_s);
							_sosd_sLUT_addr(img_s.lut_addr);
							tw_img_draw(pimg);
							playout_item = 
								(struct tw_layout_item*)playout_item->left;
							pimg = playout_item->tw_object;
							_sosd_pLUT_addr(img_s.lut_addr);
							_sosd_load_plut();							
							_sosd_swtc_h_mode();
							_sosd_swtc_offset(0);
							
							_sosd_swtc_enable();
							sosd_pLUTsLUT_load();
							
							pimg = playout->group[i].item[c_w].tw_object;
							get_res_img(pimg->imgID, &img_s);
						}
					}		
				} else {
					/* Load sLUT */
					_sosd_sLUT_addr(img_s.lut_addr);
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].left;
					pimg = playout_item->tw_object;
					get_res_img(pimg->imgID, &img_s);
					_sosd_pLUT_addr(img_s.lut_addr);					
					_sosd_swtc_h_mode();
					_sosd_swtc_offset(0);
					
					_sosd_swtc_enable();
					sosd_pLUTsLUT_load();

					pimg = playout->group[i].item[c_w].tw_object;
					get_res_img(pimg->imgID, &img_s);
				}
			} 
			/* next page */
			if((((shift)%(img_s.line_jump))<=offset)
				&&(addr!=img_s.index_addr)) {
				_sosd_swtc_disable();
				pimg = playout->group[i].item[c_w].tw_object;
				tw_img_draw(pimg);
				sosd_update();
				CUR_BK_ID= pimg->imgID;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* right slide */
			else {
				_sosd_img_addr(addr-offset);
				addr = offset;
				_get_swtc_offset((unsigned short*)&offset);
				offset >>= 16;
				sosd_update();
				_sosd_swtc_offset(addr+offset);
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_UP:	// up dir of slide
			if(c_w!=j) {
				CUR_BK_ID= pimg->imgID;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(addr==img_s.index_addr) {
				if(playout->group[i].item[j].down==NULL) {
					dbg(0, ("Can't UP slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;
					return SLIDE_HV_ERROR;		
				} else {
					/* Load sLUT */
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].down;
					pimg = playout_item->tw_object;
					get_res_img(pimg->imgID, &img_s);
					_sosd_sLUT_addr(img_s.lut_addr);
					_sosd_load_slut();
					_sosd_swtc_v_mode();
					_sosd_swtc_enable();
					sosd_update();
					_sosd_swtc_offset(img_s.height+2);
				}
			}
			/* next page */
			if((((shift)/(img_s.line_jump))+offset)
				>=((img_s.height))) {
				_sosd_swtc_disable();
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].down;
				pimg = playout_item->tw_object;
				tw_img_draw(pimg);
				sosd_enable(LOAD_LUT);
				CUR_BK_ID= pimg->imgID;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* up slide */
			else {
				_sosd_img_addr(addr+(offset*(img_s.line_jump)));
				addr = offset;
				_get_swtc_offset((unsigned short*)&offset);				
				offset >>= 16;
				sosd_update();
				_sosd_swtc_offset(offset-addr);
				SLIDE_LOCK = SLIDE_V_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_DOWN:	// down dir of slide
			if(c_w!=j) {
				CUR_BK_ID= pimg->imgID;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(addr==img_s.index_addr) {
				if(playout->group[i].item[j].up==NULL) {
					dbg(0, ("Can't down slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;	// slide init
					return SLIDE_HV_ERROR;
				} else {
					/* Load sLUT */
					_sosd_sLUT_addr(img_s.lut_addr);
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].up;
					pimg = playout_item->tw_object;
					get_res_img(pimg->imgID, &img_s);
					_sosd_pLUT_addr(img_s.lut_addr);
					_sosd_swtc_v_mode();
					_sosd_swtc_offset(2);
					
					_sosd_swtc_enable();
					sosd_pLUTsLUT_load();

					pimg = playout->group[i].item[c_w].tw_object;
					get_res_img(pimg->imgID, &img_s);
				}
			}
			/* next page */
			if(((((shift)/(img_s.line_jump))
				%(img_s.height))<=offset)&&(addr!=img_s.index_addr)) {
				_sosd_swtc_disable();

				tw_img_draw(pimg);
				sosd_update();
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			} 
			/* down slide */
			else {
				_sosd_img_addr(addr-(offset*(img_s.line_jump)));
				addr = offset;
				_get_swtc_offset((unsigned short*)&offset);
				offset >>= 16;
				sosd_update();
				_sosd_swtc_offset(addr+offset);
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
 * Description  twbc of slide effect							TD移動效果函數
 * Parameters   *playout	- point of tw_layout of struct		?面布局結?指標
 * 				dir		    - slide of direction				滑動方向
 *              offset		- slide of offset					滑動量
 *              i		    - number of row in layout			?面行數
 *              j		    - number of column in layout		?面列數
 * Return       0xFE: slide successful							滑動成功
 *              0xFF: slide error								滑動失敗
 *              other value: current stay of number in layout	目前顯示布局物件??
 */
#define TD_LINE_DUMMY	8
#define TD_LINE_ALING	4
unsigned char tw_td_slide(struct tw_layout* playout, unsigned char dir, unsigned long offset, unsigned char i, unsigned char j)
{
	unsigned char c_w= 0, c_h= 0;
	unsigned long addr= 0, shift= 0;
	struct res_td_t			img_s;
	struct tw_td			*pimg;
	struct tw_layout_item	*playout_item;

	offset = (offset/TD_LINE_ALING)*TD_LINE_ALING;	// ALIGN 4
	if(offset<=0)
		return SLIDE_HV_ERROR;

	if(i!=LAYOUT_GROUP_NUM) {
		LAYOUT_GROUP_NUM = i;
		FIRST_IMG_ADDR = 0;
	}

	/* get current image base address */
	_get_tdc_addr(&addr);

	/* get first image base address */
	pimg = playout->group[i].item[0].tw_object;
	get_res_td_by_addr(pimg->tdID, &img_s, &FIRST_IMG_ADDR);

	/* calc current of cnt at group */
	shift = addr-img_s.color_addr;
	c_w = ((shift%(img_s.line_jump+TD_LINE_DUMMY))/img_s.width);
	c_h = ((shift/(img_s.line_jump+TD_LINE_DUMMY))/(img_s.height/TD_LINE_ALING));	
	c_w = c_w+c_h*playout->group[i].width;

	/* get current of ID & Object */
	pimg = playout->group[i].item[c_w].tw_object;
	if(pimg->tdID!=SECOND_IMG_ID) {
		SECOND_IMG_ID = pimg->tdID;
		SECOND_IMG_ADDR = 0;
	}
	get_res_td_by_addr(pimg->tdID, &img_s, &SECOND_IMG_ADDR);

	switch(dir) {
		case DIR_LEFT:	// left dir of slide
			/* check next image if current address == n image start address */
			if((addr==img_s.color_addr)) {
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
			if((shift%(img_s.line_jump+TD_LINE_DUMMY))
				>=(img_s.line_jump-img_s.width)) {
				if((playout->group[i].item[j].jump!=NULL)) {
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].jump;
					pimg = playout_item->tw_object;
					tw_td_draw(pimg);
					get_res_td(pimg->tdID, &img_s);
					_sosd_tdc_addr(img_s.color_addr+offset);
					_sosd_img_addr(img_s.index_addr+(offset/TD_LINE_ALING));
					sosd_update();
					dbg(3, ("tw_td_draw imgID= %u!\n", pimg->tdID));
					return SLIDE_MOTION;
				} else {
					dbg(0, ("Can't jump!\n"));
					return SLIDE_HV_ERROR;
				}
			} 
			/* next page */
			else if((((addr-img_s.color_addr)%(img_s.line_jump+TD_LINE_DUMMY))+offset)
				>=img_s.width) {
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].right;
				pimg = playout_item->tw_object;
				tw_td_draw(pimg);
				sosd_update();
				CUR_BK_ID= pimg->tdID;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* left slide */
			else {
				_sosd_tdc_addr(addr+offset);
				_get_img_idx_addr(&addr);
				_sosd_img_addr(addr+(offset/TD_LINE_ALING));
				sosd_update();
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_RIGHT:	// right dir of slide
			/* check next image if current address == n image start address */
			if((addr==img_s.color_addr)) {
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
			if((shift%(img_s.line_jump+TD_LINE_DUMMY))==0) {
				if((playout->group[i].item[j].jump!=NULL)) {
					playout_item = 
						(struct tw_layout_item*)playout->group[i].item[j].jump;
					pimg = playout_item->tw_object;
					tw_td_draw(pimg);
					get_res_td(pimg->tdID, &img_s);
					_sosd_tdc_addr(img_s.color_addr-offset);
					_sosd_img_addr(img_s.index_addr-(offset/TD_LINE_ALING));
					sosd_update();
					dbg(3, ("tw_td_draw imgID= %u!\n", pimg->tdID));
					return SLIDE_MOTION;
				} else {
					dbg(0, ("Can't jump!\n"));
					return SLIDE_HV_ERROR;
				}
			} 
			/* next page */
			else if((((addr-img_s.color_addr)%(img_s.line_jump+TD_LINE_DUMMY))<=offset)
				&&(addr!=img_s.color_addr)) {
				pimg = playout->group[i].item[c_w].tw_object;
				tw_td_draw(pimg);
				sosd_update();
				CUR_BK_ID= pimg->tdID;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* right slide */
			else {				
				_sosd_tdc_addr(addr-offset);
				_get_img_idx_addr(&addr);
				_sosd_img_addr(addr-(offset/TD_LINE_ALING));
			    sosd_update();
				CUR_BK_ID= pimg->tdID;
				SLIDE_LOCK = SLIDE_H_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_UP:	// up dir of slide
			if(c_w!=j) {
				CUR_BK_ID= pimg->tdID;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(addr==img_s.color_addr) {
				if(playout->group[i].item[j].down==NULL) {
					dbg(0, ("Can't UP slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;
					return SLIDE_HV_ERROR;		
				}
			}
			offset = offset/TD_LINE_ALING;
			/* next page */
			if((((addr-img_s.color_addr)/(img_s.line_jump+TD_LINE_DUMMY))+offset)
				>=((img_s.height/TD_LINE_ALING))) {
				playout_item = 
					(struct tw_layout_item*)playout->group[i].item[c_w].down;
				pimg = playout_item->tw_object;
				tw_td_draw(pimg);
				sosd_update();
				CUR_BK_ID= pimg->tdID;
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			}
			/* up slide */
			else {
				_sosd_tdc_addr(addr+(offset*(img_s.line_jump+TD_LINE_DUMMY)));
				_get_img_idx_addr(&addr);
				_sosd_img_addr(addr+(offset*img_s.line_jump));
				sosd_update();
				SLIDE_LOCK = SLIDE_V_DIR;
				return SLIDE_MOTION;
			}
			break;
		case DIR_DOWN:	// down dir of slide
			if(c_w!=j) {
				CUR_BK_ID= pimg->tdID;
				j= c_w;
			}
			/* check next image if current address == n image start address */
			if(addr==img_s.color_addr) {
				if(playout->group[i].item[j].up==NULL) {
					dbg(0, ("Can't down slide!\n"));
					SLIDE_LOCK = SLIDE_NONE;	// slide init
					return SLIDE_HV_ERROR;
				}
			}
			offset = offset/TD_LINE_ALING;
			/* next page */
			if(((((addr-img_s.color_addr)/(img_s.line_jump+TD_LINE_DUMMY))
				%(img_s.height/TD_LINE_ALING))<=offset)&&(addr!=img_s.color_addr)) {
				_sosd_tdc_addr(img_s.color_addr);
				_sosd_img_addr(img_s.index_addr);
			    sosd_update();
				SLIDE_LOCK = SLIDE_NONE;	// slide init
				return c_w;
			} 
			/* down slide */
			else {
				_sosd_tdc_addr(addr-(offset*(img_s.line_jump+TD_LINE_DUMMY)));
				_get_img_idx_addr(&addr);
				_sosd_img_addr(addr-(offset*img_s.line_jump));
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
 * Description  slide init	滑動效果初始化
 * Parameters   none
 * Return       none
 */
void tw_slide_init(void)
{
	LAYOUT_GROUP_NUM= 0xFF;
	SECOND_IMG_ID	= 0x0000;
	FIRST_IMG_ADDR	= 0x0000;
	SECOND_IMG_ADDR	= 0x0000;
	SLIDE_LOCK = SLIDE_NONE;
	sosd_sw_tc_disable();
}

/*
 * Synopsis     unsigned char tw_search_ele(struct tw_layout* playout,
 *                                        unsigned char *g,
 *                                        unsigned char *w);
 * Description  search current of ele in layout					搜尋目前顯示?面??			
 * Parameters   *playout	- point of tw_layout of struct		?面布局結?指標			
 *              *g		    - point of row						?面行數
 *              *w		    - point of column					?面列數	
 * Return       0xFE: slide successful							滑動成功
 *              0xFF: slide error								滑動失敗
 *              other value: current stay of number in layout	目前顯示布局物件??
 */
unsigned char tw_search_ele(struct tw_layout* playout, unsigned char *g, unsigned char *w)
{
	unsigned char i= 0, j= 0, c_w= 0;
	struct tw_img	*pimg;

	for(i=0; i<playout->cnt; i++) {
		for(j=0; j<(playout->group[i].width*playout->group[i].height); j++) {
			pimg = (struct tw_img*)playout->group[i].item[j].tw_object;
			if(pimg->imgID==CUR_BK_ID) {
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
 * Description  background of slide effect						背景滑動效果
 * Parameters   *playout	- point of tw_layout of struct		?面布局結?指標
 * 				dir		    - slide of direction				滑動方向
 *              offset		- slide of offset					移動量
 * Return       0xFE: slide successful							移動完成	
 *              0xFF: slide error								移動失敗
 *              other value: current stay of number in layout	目前顯示布局物件??
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


struct tw_scl_txt _init_scrol(struct tw_txt* ptxt, unsigned short *ucs, unsigned char direct, unsigned char tail_blank, unsigned char is2bp) {
 	struct tw_scl_txt ret;
	unsigned char is_half_alphabet = (_tw_half_alphabet_num() > 0); 	
	unsigned short font_size = CUR_MENU_P->font_w*CUR_MENU_P->font_h/16;  
	unsigned char i;
	unsigned char size_id = _tw_get_font_size_id();
	unsigned char w_count;
	unsigned char bp = (is2bp) ? 2 : 1;
    unsigned short real_oaddr;
					 
	tw_menu_reset_index_n(ptxt->index);
	for(i=0; ucs[i]!=0 && i < 0xff; i++) {
        real_oaddr = (is2bp) ? (CUR_2BP_IDX*font_size + (menu_string_start_index-CUR_2BP_IDX)*font_size*2) : font_size*menu_string_start_index;  
	    if(is_half_alphabet && ucs[i] < 256) {
			w_count = osd_put_twf_char_to_oram(CUR_TOF_RES.base_addr, real_oaddr, ucs[i],
									     font_size*TOF_FONT_SPLIT_H[size_id]*2*bp, is_half_alphabet);   
			menu_string_start_index += w_count*TOF_FONT_SPLIT_H[size_id];
		}
		else {																				   
			osd_put_twf_char_to_oram(CUR_TOF_RES.base_addr, real_oaddr, ucs[i], 
									     font_size*TOF_FONT_SPLIT_W[size_id]*TOF_FONT_SPLIT_H[size_id]*2*bp, is_half_alphabet); 
			menu_string_start_index += TOF_FONT_SPLIT_W[size_id]*TOF_FONT_SPLIT_H[size_id];
		}
	}
    real_oaddr = (is2bp) ? (CUR_2BP_IDX*font_size + (menu_string_start_index-CUR_2BP_IDX)*font_size*2) : font_size*menu_string_start_index;   	 
	osd_put_twf_char_to_oram(CUR_TOF_RES.base_addr, real_oaddr, 0x20, font_size*2*bp, is_half_alphabet); 
	
	ret.total_len = (menu_string_start_index - ptxt->index) / TOF_FONT_SPLIT_H[size_id];	
	ret.direct = direct;  
	ret.tail_blank = tail_blank; 
	ret.cur_index = ptxt->index; 
	ret.size_id = size_id; 
	ret.ptxt = ptxt;	
 
	tw_scrolling(&ret);
	
	return ret;	

}

 /*
 * Synopsis     struct tw_scl_txt tw_init_scrol(struct tw_txt* ptxt,
 *                                              unsigned short *ucs,
 *                                              unsigned char direct,
 *		                                        unsigned char tail_blank);
 * Get a marquee struct from a txt.
 * Parameters   *ptxt	    - the txt going to make this marquee 標
 * 				*ucs		- the words going to draw on marquee
 *              direct		- the direction of this marquee (0:left  1:right)
 *              tail_blank  - the sum of blanks between the head and tail of each sequence
 * Return       tw_scl_txt	- the marquee struct used to drawing the input string						
 */
struct tw_scl_txt tw_init_scrol(struct tw_txt* ptxt, unsigned short *ucs, unsigned char direct, unsigned char tail_blank)
{
	return _init_scrol(ptxt, ucs, direct, tail_blank, 0);										
}

 /*
 * Synopsis     struct tw_scl_txt tw_init_scrol_2bp(struct tw_txt* ptxt,
 *                                                  unsigned short *ucs,
 *                                                  unsigned char direct,
 *		                                            unsigned char tail_blank);
 * Get a 2bp marquee struct from a txt.
 * Parameters   *ptxt	    - the txt going to make this marquee 標
 * 				*ucs		- the words going to draw on marquee
 *              direct		- the direction of this marquee (0:left  1:right)
 *              tail_blank  - the sum of blanks between the head and tail of each sequence
 * Return       tw_scl_txt	- the marquee struct used to drawing the input string						
 */
struct tw_scl_txt tw_init_scrol_2bp(struct tw_txt* ptxt, unsigned short *ucs, unsigned char direct, unsigned char tail_blank)
{
	return _init_scrol(ptxt, ucs, direct, tail_blank, 1);										
}

void tw_scrolling(struct tw_scl_txt* scl_txt)
{						  
	unsigned char color = (scl_txt->ptxt->fg_color<<1L) + (scl_txt->ptxt->bg_color<<6L);
	unsigned short i, j, dW;  
	unsigned short dat;
	unsigned char split_h = TOF_FONT_SPLIT_H[scl_txt->size_id];
 
	unsigned short max_index = scl_txt->ptxt->index + scl_txt->total_len * split_h;	  	
	unsigned short color_h = (color<<8L);
	unsigned char blank_index_count = scl_txt->tail_blank*split_h;
	unsigned short disp_index_count = scl_txt->ptxt->width*split_h;	 

	for(i=0; i<scl_txt->ptxt->width; i++) {
		for(j=0; j<split_h; j++) {			
			if(scl_txt->cur_index+i*split_h > max_index) {
				dW = (scl_txt->cur_index+i*split_h) - max_index;
				if(dW < blank_index_count) {
					dat = color_h + 0;	
					osd_set_location_addr(scl_txt->ptxt->x+i, scl_txt->ptxt->y+j, CUR_MENU_P->addr, CUR_MENU_P->width);				 
					OSD_SET_RAM_DATA(dat);
				}
				else {
					dW = dW - blank_index_count;	  
					dat = color_h + scl_txt->ptxt->index+dW+j;
					osd_set_location_addr(scl_txt->ptxt->x+i, scl_txt->ptxt->y+j, CUR_MENU_P->addr, CUR_MENU_P->width);						 
					OSD_SET_RAM_DATA(dat);					 
				}
			}
			else {	 
				dat = color_h + scl_txt->cur_index + i*split_h+j;	
				osd_set_location_addr(scl_txt->ptxt->x+i, scl_txt->ptxt->y+j, CUR_MENU_P->addr, CUR_MENU_P->width);					 
				OSD_SET_RAM_DATA(dat);
			}
		}
	}

	if(scl_txt->direct) {
		scl_txt->cur_index += split_h;	
		scl_txt->cur_index = scl_txt->ptxt->index + (scl_txt->cur_index - scl_txt->ptxt->index)%((scl_txt->total_len + scl_txt->tail_blank)*split_h);
	}
	else {
		scl_txt->cur_index = (scl_txt->cur_index < split_h) ? 0 : scl_txt->cur_index - split_h;
		if(scl_txt->cur_index < scl_txt->ptxt->index) {
			scl_txt->cur_index = scl_txt->ptxt->index + (scl_txt->total_len + scl_txt->tail_blank - 1)*split_h;	
		}	
	}	
}

