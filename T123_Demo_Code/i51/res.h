#ifndef __RES_H__
#define __RES_H__

enum PAGE_ID_DEF {
	ID_PAGE_RGB = 1,
	TOTAL_PAGE_NUM
};

#define ID_LAYOUT_1_RGB 1
//[TD Struct]
// color_addr : 0x000000
// index_addr : 0x004F70
// line_jump  : 320
// width      : 320
// height     : 240
#define ID_ICON_SPACE_W12_H18_1BP 5008
//[ICON Struct]
// base_addr  : 0x009A70
// font_w     : 12
// font_h     : 18
// width      : 1
// height     : 1
// bp         : 1
// count      : 1
#define ID_ICON_ROM_FONT_1 2
//[ICON Struct]
// base_addr  : 0x009A8C
// font_w     : 12
// font_h     : 18
// width      : 2
// height     : 1
// bp         : 1
// count      : 0
#define ID_ICON_ROM_FONT_2 3
//[ICON Struct]
// base_addr  : 0x009A8C
// font_w     : 12
// font_h     : 18
// width      : 2
// height     : 1
// bp         : 1
// count      : 0
#define ID_ICON_ROM_FONT_3 4
//[ICON Struct]
// base_addr  : 0x009A8C
// font_w     : 12
// font_h     : 18
// width      : 2
// height     : 1
// bp         : 1
// count      : 0
#define ID_ICON_ROM_FONT_4 5
//[ICON Struct]
// base_addr  : 0x009A8C
// font_w     : 12
// font_h     : 18
// width      : 1
// height     : 1
// bp         : 1
// count      : 0
#define ID_ICON_ROM_FONT_5 6
//[ICON Struct]
// base_addr  : 0x009A8C
// font_w     : 12
// font_h     : 18
// width      : 1
// height     : 1
// bp         : 1
// count      : 0
#define ID_ICON_DUMMY_W12_H18_1BP 5013
//[ICON Struct]
// base_addr  : 0x009A8C
// font_w     : 12
// font_h     : 18
// width      : 1
// height     : 1
// bp         : 1
// count      : 1
#define ID_OSD2_0_ORG 5005
//[OREG Struct]
// base_addr  : 0x009AA8
// length     : 54
#define ID_OSD2_0_MENU 5004
//[MENU Struct]
// base_addr  : 0x009ADE
// oram_addr  : 0x3E4B
// length     : 868
// width      : 27
// height     : 14
#define ID_OSD2_0_LUT 5006
//[LUT Struct]
// lut_addr   : 0x009E42
// main_len   : 16
// sec_len    : 0
// re_2bp_len : 0
// re_bg_len  : 4
#define ID_OSD2_0_TILES 5007
//[Tile Struct]
// base_addr  : 0x009E86
// length     : 7
//#define ID_OSD2_0_TILES 5007
//[Tile Struct]
// base_addr  : 0x009E86
// length     : 7
#endif
