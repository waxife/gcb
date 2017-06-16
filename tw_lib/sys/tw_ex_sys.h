/**
 *  @file   tw_ex_sys.h
 *  @brief  head file for tw_ex_sys
 *  $Id: tw_ex_sys.h,v 1.19 2014/02/21 06:42:30 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.19 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/11/06  ken 	New file.
 *
 */

#ifndef _TW_EX_SYS_H
#define _TW_EX_SYS_H

#include "config.h"

#ifdef RESOURCE_ON_NOR_FLASH

/**
 * sOSD Image structure
 */
struct tw_img{
    unsigned long          	lut_addr;
	unsigned long          	index_addr;
    unsigned short          x;          // unit is pixel
    unsigned short          y;          // unit is pixel
    unsigned short          width;      // unit is pixel
    unsigned short          height;     // unit is pixel
	unsigned short			line_jump;
};

/**
 * sOSD Sprite structure
 */
struct tw_spr{
    unsigned long          	lut_addr;
	unsigned long          	index_addr;
    unsigned short          x;          // unit is pixel
    unsigned short          y;          // unit is pixel
    unsigned char          	width;      // unit is pixel
    unsigned short          height;     // unit is pixel
	unsigned char			line_jump;
	unsigned char 			cnt;
};

/**
 * sOSD TD structure
 */
struct tw_td{
    unsigned long          	color_addr;
	unsigned long          	index_addr;
    unsigned short          x;          // unit is pixel
    unsigned short          y;          // unit is pixel
    unsigned short          width;      // unit is pixel
    unsigned short          height;     // unit is pixel
	unsigned short			line_jump;
};

/**
 * OSD2 Icon structure
 */
struct tw_icon{
    unsigned long          	base_addr;
    unsigned char          	color;		// lut index | 2nd lut index
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
	unsigned short          index;
};

/**
 * OSD2 Icon structure
 */
struct tw_icons{
	unsigned long          	base_addr;
    unsigned char          	color;		// lut index | 2nd lut index
	unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
    unsigned char          *table;
	unsigned short			offset;
};

/**
 * OSD2 Group Icon structure
 */
struct tw_gicon {
	struct tw_tiles			*tiles;
    unsigned char          	color;		// lut index | 2nd lut index
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
	unsigned short          index;
    unsigned short          oram_addr;
};

/**
 * OSD2 Group Icons structure
 */
struct tw_gicons {
	struct tw_tiles			*tiles;
    unsigned char          	color;		// lut index | 2nd lut index
	unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
    unsigned char          *table;
	unsigned short			offset;
	unsigned short          oram_addr;
};

/**
 * OSD2 text (twfont) structure
 */
struct tw_txt{
    unsigned long          	base_addr;
	unsigned char           fg_color;	   
	unsigned char           bg_color; 
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
    unsigned short          index;	
};

/**
 * OSD2 Scrolling text
 */
struct tw_scl_txt {
	unsigned short	total_len;
	unsigned char	direct;	 
	unsigned char	tail_blank;	 
	unsigned short	cur_index;	 
	unsigned char	size_id;
  	struct tw_txt*  ptxt;
};

struct tw_tiles{
	unsigned long			*tables;
	unsigned char			cnt;	
};

struct tw_luts{
	unsigned long			base_addr;
	unsigned short			main_ln;
	unsigned char			sec_ln;
	unsigned char			re_2bp_ln;
	unsigned char			re_bg_ln;
};

struct tw_regs{
	unsigned long			base_addr;
	unsigned char			length;
};

struct tw_tofs{
	unsigned long			base_addr;
	unsigned char			font_w;
	unsigned char			font_h;
};

/**
 * OSD2 Page structure
 */
struct tw_menu{
    unsigned long          	base_addr;
    struct tw_tiles			*tiles;	
	struct tw_luts			*luts;
	struct tw_regs			*regs;
	struct tw_tofs			*tofs;
    unsigned char           x;          // start x
    unsigned char           y;          // start y
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is ROW
	unsigned short          addr;
    unsigned char           font_w;
	unsigned char           font_h;
};

struct tw_layout_item {
	void*				tw_object;
	void*				up;
	void*				down;
	void*				left;
	void*				right;
	void*				jump;
};

struct tw_layout_group {
	unsigned char			width;
	unsigned char			height;
	struct tw_layout_item*	item;
};

/**
 * sOSD Layout
 */
struct tw_layout {
	unsigned char			type;		// 0: image, 1: twbc
	unsigned char			cnt;		// group count
	struct tw_layout_group*	group;	
};

#else	/* RESOURCE_ON_NOR_FLASH */

/**
 * OSD2 Page structure
 */
struct tw_menu{
    unsigned short          menuID;
    unsigned short          tilesID;
	unsigned short			lutsID;
	unsigned short          regsID;
	unsigned short          tofID;
    unsigned char           x;          // start x
    unsigned char           y;          // start y
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is ROW
	unsigned short          addr;
    unsigned char           font_w;
	unsigned char           font_h;
};

/**
 * OSD2 Icon structure
 */
struct tw_icon{						    
    unsigned char          	color;		// lut index | 2nd lut index
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
	unsigned short          index;
};

/**
 * OSD2 Icon structure
 */
struct tw_icons{
    unsigned char          	color;		// lut index | 2nd lut index
	unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
    unsigned char          *table;
	unsigned short			offset;
};

/**
 * OSD2 text (twfont) structure
 */
struct tw_txt{   
	unsigned char           fg_color;	   
	unsigned char           bg_color; 
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
    unsigned short          index;	
};

#endif

/*
 *	TOUCH DIRVERS DEFINE
 */
	/* UI type */
    #define TW_UI_TYPE_IMAGE             	1
	#define TW_UI_TYPE_SPRITE             	2
	#define TW_UI_TYPE_OSD2             	3
	#define EVENT_TYPE_TOUCH_DOWN		0
	#define EVENT_TYPE_TOUCH_MOTION		1
	#define EVENT_TYPE_TOUCH_RELEASE	2
	#define EVENT_TYPE_TOUCH_REPEAT		3
	#define DIR_LEFT		0
	#define DIR_RIGHT		1
	#define DIR_UP			2
	#define DIR_DOWN		3
	#define SLIDE_NONE		0
	#define SLIDE_H_DIR		1
	#define SLIDE_V_DIR		2
	#define IMG_SLIDE		0
	#define TD_SLIDE		1
	#define SLIDE_MOTION	0xFE
	#define SLIDE_HV_ERROR	0xFF


#ifdef RESOURCE_ON_NOR_FLASH
/* for show osd2 using */
extern void tw_load_menu_dma(unsigned long base_addr, unsigned short oram_addr, unsigned short length);
extern void tw_load_icon_dma(unsigned long base_addr, unsigned short *oaddr, unsigned short length);
extern void tw_load_luts_dma(unsigned long base_addr, unsigned short main_ln, unsigned char r2bp_ln, unsigned char rbg_ln);
extern void tw_load_oregs_dma(unsigned long base_addr, unsigned char length);
extern void tw_load_oregs(struct tw_regs *pregs);
extern void tw_load_luts(struct tw_luts *pluts);
extern void tw_load_tiles(struct tw_tiles *ptiles);
extern void tw_menu_draw(struct tw_menu *pmenu);
extern void tw_text_draw(struct tw_txt* ptxt, unsigned short *ucs);	 
extern void tw_text_color_change(struct tw_txt* ptxt, unsigned short *ucs, unsigned char color);	
extern void _romfont_draw(struct tw_txt* ptxt, unsigned char x, unsigned char y, const unsigned char *ucs, unsigned char color, unsigned char blink);

/* for show spiosd using */
extern void tw_img_draw_ex(unsigned long lut_addr, unsigned long index_addr, unsigned short line_jump, unsigned short width, unsigned short height, unsigned short x, unsigned short y);
extern void tw_spr_draw_ex(unsigned long lut_addr, unsigned long index_addr, unsigned char line_jump, unsigned char width, unsigned short height, unsigned short x, unsigned short y);
extern void tw_td_draw_ex(unsigned long color_addr, unsigned long index_addr, unsigned short line_jump, unsigned short width, unsigned short height, unsigned short x, unsigned short y);
extern void tw_img_draw(struct tw_img *img);
extern void tw_spr_draw(struct tw_spr *spr);
extern void tw_td_draw(struct tw_td *td);

/* for emulation script using */
extern void tw_emu_trig(void);
extern void tw_load_emu(unsigned long base_addr);
extern void tw_wait_emu_done(void);
extern void tw_emu_exit(void);

/* for scrolling text drawing */
extern struct tw_scl_txt tw_init_scrol(struct tw_txt* ptxt, unsigned short *ucs, unsigned char direct, unsigned char tail_blank);
extern void tw_scrolling(struct tw_scl_txt* scl_txt);

/* for slide effect using */
extern void tw_slide_init(void);
extern unsigned char tw_page_slide(struct tw_layout* playout, unsigned char dir, unsigned short offset);

extern void tw_text_color_change(struct tw_txt* ptxt, unsigned short *ucs, unsigned char color);

#else /* RESOURCE_ON_NOR_FLASH */
/* for show osd2 using */
extern void tw_load_oregs(unsigned char *pconfig, unsigned char length);
extern void tw_load_luts(unsigned char *plut, unsigned short s_addr, unsigned long length);
extern void tw_load_remap(unsigned char *plut, unsigned short s_addr, unsigned long length);
extern void tw_load_bg_lut(unsigned char *plut, unsigned short s_addr, unsigned long length);
extern void tw_load_tiles(unsigned short *ptable, unsigned short s_addr, unsigned short d_addr, unsigned short length);
extern void tw_load_menu(unsigned char *pmenu, unsigned short d_addr, unsigned char width, unsigned short length, unsigned char font_w, unsigned char font_h);
extern void tw_text_draw(struct tw_txt* ptxt, const unsigned char *ucs);
extern void _romfont_draw(struct tw_txt* ptxt, unsigned char x, unsigned char y, const unsigned char *ucs, unsigned char color, unsigned char blink);

extern void tw_text_color_change(struct tw_txt* ptxt, unsigned char *ucs, unsigned char color);
#endif

extern unsigned char region_check(unsigned int tp_x, unsigned int tp_y, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char type);

/* for iconset (multi-language) using */
extern void next_iconset(void);
extern void set_iconset_num(unsigned char num);
extern unsigned char get_iconset_num(void);

/* for show osd2 using */
extern void _icon_draw(unsigned short index, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char blinking);
extern void _tw_icon_draw_at(struct tw_icon* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking);
extern void tw_icon_draw(struct tw_icon* picon);
extern void _icons_draw(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *table, unsigned char color, unsigned char blinking, unsigned short offset);
extern void _tw_icons_draw_at(struct tw_icons* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking);
extern void tw_icons_draw(struct tw_icons* picon);

/* for color change using */
extern void tw_icon_1st_color(struct tw_icon* picon);
extern void tw_icon_2nd_color(struct tw_icon* picon);
extern void tw_icon_color_change(struct tw_icon* picon, unsigned char color);
extern void tw_icons_1st_color(struct tw_icons* picon);
extern void tw_icons_2nd_color(struct tw_icons* picon);
extern void tw_icons_color_change(struct tw_icons* picon, unsigned char color);	
extern void tw_text_index_change(struct tw_txt* ptxt, unsigned char index);

/* for clear osd2 using */
extern void _tw_icons_draw_at(struct tw_icons* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking);
extern void _tw_clear_icon_at(struct tw_icon* picon, unsigned short menu_addr, unsigned char menu_width);
extern void _tw_clear_icons_at(struct tw_icons* picon, unsigned short menu_addr, unsigned char menu_width);
extern void _tw_clear_txt_at(struct tw_txt* ptxt, unsigned short menu_addr, unsigned char menu_width);
extern void tw_osd_clear_line(unsigned char y);
extern void tw_osd_clear_char(unsigned char x, unsigned char y);
extern void tw_osd_clear_menu(void);
extern void tw_clear_icon(struct tw_icon* picon);
extern void tw_clear_icons(struct tw_icons* picon);
extern void tw_clear_txt(struct tw_txt* ptxt);

/* for write menu data using */
extern void tw_menu_write(unsigned char x, unsigned char y, unsigned char index, unsigned char color);
extern void tw_osd_menu_write(unsigned char x, unsigned char y, unsigned short index, unsigned char color);
extern void tw_osd_menu_write_line(unsigned char y, unsigned short index, unsigned char color);
extern void tw_menu_fill(unsigned char x, unsigned char y, unsigned char dat ,unsigned char color, unsigned char length);

/* APIs for group icon */
extern unsigned char check_load_gicon (struct tw_tiles *ptiles, unsigned short oram_addr);
extern void tw_gicon_draw(struct tw_gicon* pgicon);
extern void tw_gicons_draw(struct tw_gicons* pgicon);
extern void tw_gicon_color_change(struct tw_gicon* picon, unsigned char color);
extern void tw_gicons_color_change(struct tw_gicons* picon, unsigned char color);
extern void tw_clear_gicon(struct tw_gicon* picon);
extern void tw_clear_gicons(struct tw_gicons* picon);

#endif	/* _TW_EX_SYS_H */
