/**
 *  @file   tw_widget_sys.h
 *  @brief  head flie for tw_widget_sys
 *  $Id: tw_widget_sys.h,v 1.25 2013/11/21 10:46:40 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.25 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 *
 *  @date   2011/09/15  ken     New file.
 *
 */

#ifndef __TW_WIDGET_SYS_H__
#define __TW_WIDGET_SYS_H__

#include "rc.h"

	/* Event type */	
	#define TW_EVENT_TYPE_NONE              0
    #define TW_EVENT_TYPE_EXPOSURE          1
    #define TW_EVENT_TYPE_ENTER_SYSTEM      2
    #define TW_EVENT_TYPE_QUIT_SYSTEM       3
    #define TW_EVENT_TYPE_TOUCH_DOWN        4
    #define TW_EVENT_TYPE_TOUCH_RELEASE     5
    #define TW_EVENT_TYPE_TOUCH_MOTION      6
    #define TW_EVENT_TYPE_KEY_DOWN          7
    #define TW_EVENT_TYPE_KEY_UP            8
    #define TW_EVENT_TYPE_KEY_REPEAT        9
    #define TW_EVENT_TYPE_IR_DOWN           10
    #define TW_EVENT_TYPE_IR_UP             11
    #define TW_EVENT_TYPE_IR_REPEAT         12
    #define TW_EVENT_TYPE_TIMEOUT           13
    #define TW_EVENT_TYPE_UART              14
	#define TW_EVENT_TYPE_GPIO              15
	#define TW_EVENT_TYPE_OTHER             16
	#define TW_EVENT_TYPE_SINGNAL           17
    #define TW_EVENT_TYPE_BT_PB             18
	#define TW_EVENT_TYPE_TOUCH_REPEAT      19

	/* UI type */
    #define TW_UI_TYPE_IMAGE             	1
	#define TW_UI_TYPE_SPRITE             	2
	#define TW_UI_TYPE_OSD2             	3

	/* Return code */
	#define TW_RETURN_NONE              	0
    #define TW_RETURN_NOT_PAGE          	1
    #define TW_RETURN_NO_ACTIVE      		2
    #define TW_RETURN_NO_PROCESS       		3
	#define TW_RETURN_NO_PAGE       		4
	/* Error code */
	#define TW_ERROR_NO_PAGE				5

	/* TIMER ID */
	#define INTERNAL_TIMER0					0
	#define INTERNAL_TIMER1					1
	#define TW_10MS_TIMER					2

    /**
     * Event for a keystroke typed for the window with has focus.
     */
    typedef struct {
      unsigned char type;                     /**< event type */
      unsigned char ch;                       /**< 8-bit key value */
    } TW_EVENT_KEYSTROKE;
    
    /**
     * Event for a ir code typed for the window with has focus.
     */
    typedef struct {
      unsigned char type;                     /**< event type */
      unsigned char ch;                       /**< 16-bit key value */
    } TW_EVENT_IRCODE;
    
    /**
     * TW_EVENT_TYPE_TIMER
     */
    typedef struct {
      unsigned char type;                     /**< event type */
      unsigned char tid;                      /**< ID of expired timer */
    } TW_EVENT_TIMER;
    
    /**
     * Events for touch motion or touch position.
     */
    typedef struct {
      unsigned char type;                   /**< event type */
      int x;                                /**< window x coordinate of touch */
      int y;                                /**< window y coordinate of touch */
      unsigned int cpu_count;
    } TW_EVENT_TOUCH;
    
    /**
    * Events for uart
    */
    typedef struct {
      unsigned char type;                   /**< event type */
      unsigned char ch;             		/**< 8-bit key value */
    } TW_EVENT_UART;

	/**
    * Events for page
    */
    typedef struct {
      unsigned char type;                   /**< event type */
      unsigned char id;             		/**< page id */
	  unsigned char mode;					/**< 0: normal, 1: slide */
    } TW_EVENT_PAGE;

		/**
    * Events for page
    */
    typedef struct {
    unsigned char type;                   /**< event type */
    unsigned char SigActive;    /**/
	unsigned char SigSrc;       /**/
	unsigned char SigStd;       /**/
    } TW_EVENT_SIGNAL;
		

	/**
    * Events for other
    */
    typedef struct {
      unsigned char type;                   /**< event type */
      void	*p;             				/**< other define point */
	  unsigned char addr;
      unsigned char len;
    } TW_EVENT_OTHER;

    /**
     * Union of all possible event structures.
     * This is the structure returned by get_event() and similar routines.
     */
    typedef union {
      unsigned char type;                   /**< event type */
      TW_EVENT_KEYSTROKE keystroke;         /**< keystroke events */
      TW_EVENT_IRCODE   ircode;             /**< ir code events */
      TW_EVENT_TOUCH touch_panel;           /**< mouse motion events */
      TW_EVENT_TIMER timer;                 /**< Timer events */
      TW_EVENT_UART uart;                   /**<Uart event */
	  TW_EVENT_PAGE page;                   /**<Page event */
	  TW_EVENT_SIGNAL signal;
	  TW_EVENT_OTHER other;                 /**<Other define event */
    } TW_EVENT;

	/**
	 * OSD type order 
	 */
	enum OBJECT_TYPE {
	    IMAGE_TYPE = 1,
	    SPRITE_TYPE,
	    MENU_TYPE,
	    ICON_TYPE,
	    TEXT_TYPE
	};

/**
 * sOSD Sprite structure
 */
struct tw_spr{
    unsigned short          sprID;
    unsigned short          x;          // unit is pixel
    unsigned short          y;          // unit is pixel
    unsigned char          	width;      // unit is pixel
    unsigned short          height;     // unit is pixel
	unsigned char 			cnt;
};

/**
 * sOSD Image structure
 */
struct tw_img{
    unsigned short          imgID;
    unsigned short          x;          // unit is pixel
    unsigned short          y;          // unit is pixel
    unsigned short          width;      // unit is pixel
    unsigned short          height;     // unit is pixel
};

/**
 * sOSD TD structure
 */
struct tw_td{
    unsigned short          tdID;
    unsigned short          x;          // unit is pixel
    unsigned short          y;          // unit is pixel
    unsigned short          width;      // unit is pixel
    unsigned short          height;     // unit is pixel
};

/**
 * OSD2 Icon structure
 */
struct tw_icon{
    unsigned short          icon_id;
    unsigned char          	color;		// lut index | 2nd lut index
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
#ifdef	NOSUPPORT_1BP_ROM_FONT
    unsigned char          	index;
#else
	unsigned short          index;
#endif
};

/**
 * OSD2 Icon structure
 */
struct tw_icons{
	unsigned short          icon_id;
    unsigned char          	color;		// lut index | 2nd lut index
	unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
    unsigned char          *table;
	unsigned short			offset;
};

/**
 * OSD2 Icon structure
 */
struct tw_gicon {
    unsigned short          gicon_id;
    unsigned char          	color;		// lut index | 2nd lut index
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
#ifdef	NOSUPPORT_1BP_ROM_FONT
    unsigned char          	index;
#else
	unsigned short          index;
#endif
    unsigned short          oram_addr;
};

/**
 * OSD2 Icon structure
 */
struct tw_gicons {
	unsigned short          gicons_id;
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
    unsigned short          icon_id;   
	unsigned char           fg_color;	   
	unsigned char           bg_color; 
    unsigned char           x;          // unit is Character
    unsigned char           y;          // unit is Character
    unsigned char           width;      // unit is Character
    unsigned char           height;     // unit is Character
    unsigned short          index;	
};

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

#define DO_ISSUE	1
#define DONT_ISSU	0

#define PAGE_EVENT_NORMAL	0
#define PAGE_EVENT_SLIDE	1

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


extern unsigned char CUR_PAGE_ID;

extern unsigned char region_check(TW_EVENT* event, unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned char type);

/* for show spiosd using */
extern void tw_img_draw(struct tw_img* pimg);
extern void tw_spr_draw(struct tw_spr* spr);
extern void tw_spr_action(struct tw_spr* pspr);
extern void tw_td_draw(struct tw_td* ptd);

/* for show osd2 using */
extern void tw_menu_draw(struct tw_menu* pmenu);
extern void tw_icon_draw(struct tw_icon* picon);
extern void tw_icons_draw(struct tw_icons* picon);
extern void tw_text_draw(struct tw_txt* ptxt, unsigned short *ucs);	
extern void _icon_draw(unsigned short index, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char blinking);
extern void _icons_draw(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *table, unsigned char color, unsigned short offset, unsigned char blinking);
extern void tw_load_icon(unsigned short id, unsigned short *oaddr);
extern void _tw_load_menu_at(unsigned short id, unsigned short addr);
extern void _tw_icon_draw_at(struct tw_icon* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking); 
extern void tw_load_menu(struct tw_menu* pmenu);

extern void _icon_draw_at(unsigned short index, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking);
extern void _tw_icon_draw_at(struct tw_icon* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking);
extern void _icons_draw_at(unsigned char *table, unsigned char color, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking);
extern void _tw_icons_draw_at(struct tw_icons* picon, unsigned short menu_addr, unsigned char menu_width, unsigned short offset, unsigned char blinking);
		 

/* for color change using */
extern void tw_icon_1st_color(struct tw_icon* picon);
extern void tw_icon_2nd_color(struct tw_icon* picon);
extern void tw_icon_color_change(struct tw_icon* picon, unsigned char color);
extern void tw_icons_1st_color(struct tw_icons* picon);
extern void tw_icons_2nd_color(struct tw_icons* picon);
extern void tw_icons_color_change(struct tw_icons* picon, unsigned char color);
extern void tw_text_color_change(struct tw_txt* ptxt, unsigned short *ucs, unsigned char color);	
extern void tw_text_index_change(struct tw_txt* ptxt, unsigned char index);	

/* for clear osd2 using */
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

extern unsigned short tw_menu_draw_nstring(unsigned char x, unsigned char y, unsigned char max_w, const unsigned short *ucs, unsigned short menu_addr, unsigned char menu_width, unsigned char color, unsigned char size_id, unsigned char is2BP);
extern void tw_menu_reset_index(); 
extern void tw_menu_reset_index_n(unsigned short idx); 
extern void tw_menu_fill(unsigned char x, unsigned char y, unsigned char dat ,unsigned char color, unsigned char length);
extern void tw_menu_row_moveV(unsigned char row, unsigned short y);
extern void put_enter_page_event(unsigned char id);
extern void put_quit_page_event(unsigned char id);
extern void put_enter_slide_page_event(unsigned char id);
extern void put_quit_slide_page_event(unsigned char id);
extern void tw_widget_init();

/* for emulation script using */
extern void tw_load_emu(unsigned short id);
extern void tw_wait_emu_done();
extern void tw_emu_exit(void);
extern void tw_emu_trig(void); 


/* for iconset (multi-language) using */
extern void next_iconset(void);
extern void set_iconset_num(unsigned char num);
extern unsigned char get_iconset_num(void);

/* for twf numbers drawing seriously using*/
extern void tw_menu_reset_index_rev_nums();
extern void tw_menu_nums_init(); 

/* for slide effect using */
extern void tw_slide_init(void);
extern unsigned char tw_page_slide(struct tw_layout* playout, unsigned char dir, unsigned short offset);

/* for scrolling text (marquee) drawing */
extern struct tw_scl_txt tw_init_scrol(struct tw_txt* ptxt, unsigned short *ucs, unsigned char direct, unsigned char tail_blank);
extern struct tw_scl_txt tw_init_scrol_2bp(struct tw_txt* ptxt, unsigned short *ucs, unsigned char direct, unsigned char tail_blank);
extern void tw_scrolling(struct tw_scl_txt* scl_txt);


extern void tw_img_force_alpha(unsigned char en);
extern void tw_get_spr_idx(unsigned short id, unsigned long *addr);

extern void put_enter_page_event(unsigned char id);
extern void put_quit_page_event(unsigned char id);
extern void put_enter_slide_page_event(unsigned char id);
extern void put_quit_slide_page_event(unsigned char id);
extern void tw_widget_init();
extern unsigned char tw_page_process(TW_EVENT* event);
extern void put_event (TW_EVENT* event);
extern TW_EVENT* get_event (void);
extern void clear_event(void);

extern unsigned short find_ch_icon(unsigned short id, unsigned short tilesID);
/* APIs for group icon */
extern unsigned short check_gicon_id (unsigned short id);
extern void tw_gicon_draw(struct tw_gicon* pgicon);
extern void tw_gicons_draw(struct tw_gicons* pgicon);


#endif	/* __TW_WIDGET_SYS_H__ */
