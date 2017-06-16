#include "sys.h"
#include "tw_ex_sys.h"
#include "res_full.h"
#include "page_RGB.h"

struct tw_td code layout_1_rgb = {
  0x000000+RES_OFFSET,
  0x004F70+RES_OFFSET,
  0,
  0,
  320,
  240,
  320
};

struct tw_menu code	page_rgb_osd2_0_menu = {
  0x009ADE+RES_OFFSET,
  &page_rgb_osd2_0_tile,
  &page_rgb_osd2_0_lut,
  &page_rgb_osd2_0_org,
  0,
  0,
  0,
  27,
  14,
  15947,
  12,
  18,
};

unsigned char code page_rgb_rom_font_1_ids[] = {
	13, 14, };

struct tw_icons code page_rgb_rom_font_1 = {
  0x009A8C+RES_OFFSET,
  1,
  0,
  0,
  2,
  1,
  (unsigned char*)page_rgb_rom_font_1_ids,
  2,
};

unsigned char code page_rgb_rom_font_2_ids[] = {
	15, 16, };

struct tw_icons code page_rgb_rom_font_2 = {
  0x009A8C+RES_OFFSET,
  2,
  24,
  0,
  2,
  1,
  (unsigned char*)page_rgb_rom_font_2_ids,
  2,
};

unsigned char code page_rgb_rom_font_3_ids[] = {
	19, 20, };

struct tw_icons code page_rgb_rom_font_3 = {
  0x009A8C+RES_OFFSET,
  3,
  0,
  12,
  2,
  1,
  (unsigned char*)page_rgb_rom_font_3_ids,
  2,
};

unsigned char code page_rgb_rom_font_4_ids[] = {
	38, };

struct tw_icons code page_rgb_rom_font_4 = {
  0x009A8C+RES_OFFSET,
  4,
  25,
  12,
  1,
  1,
  (unsigned char*)page_rgb_rom_font_4_ids,
  2,
};

unsigned char code page_rgb_rom_font_5_ids[] = {
	3, };

struct tw_icons code page_rgb_rom_font_5 = {
  0x009A8C+RES_OFFSET,
  5,
  3,
  0,
  1,
  1,
  (unsigned char*)page_rgb_rom_font_5_ids,
  2,
};

struct tw_luts code page_rgb_osd2_0_lut = {
  0x009E42+RES_OFFSET,
  16,//main len
  0,//sec len
  0,//2bp len
  4//bg len
};

struct tw_regs code page_rgb_osd2_0_org = {
  0x009AA8+RES_OFFSET,
  54
};

unsigned long code osd2_0_tile_tbl[] = {
  0x009A70+RES_OFFSET,  _OSD2_0_MENU_1BP_SIZE*1,
  //0,                  _OSD2_0_MENU_1BP_SIZE*0,  //rom font
  //0,                  _OSD2_0_MENU_1BP_SIZE*0,  //rom font
  //0,                  _OSD2_0_MENU_1BP_SIZE*0,  //rom font
  //0,                  _OSD2_0_MENU_1BP_SIZE*0,  //rom font
  //0,                  _OSD2_0_MENU_1BP_SIZE*0,  //rom font
  0x009A8C+RES_OFFSET,  _OSD2_0_MENU_1BP_SIZE*1,
};

struct tw_tiles code page_rgb_osd2_0_tile = {
  osd2_0_tile_tbl,
  2//length
};

