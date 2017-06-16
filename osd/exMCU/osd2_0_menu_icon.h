#define OSD2_0_MENU_FONT_W  12L
#define OSD2_0_MENU_FONT_H  18L

#define OSD2_0_MENU_1BP_SIZE ((OSD2_0_MENU_FONT_W*OSD2_0_MENU_FONT_H)/8L+1) //bytes
#define OSD2_0_MENU_2BP_SIZE ((OSD2_0_MENU_FONT_W*OSD2_0_MENU_FONT_H)/4L) //bytes
#define OSD2_0_MENU_4BP_SIZE ((OSD2_0_MENU_FONT_W*OSD2_0_MENU_FONT_H)/2L) //bytes
#define OSD2_0_MENU_8BP_SIZE ((OSD2_0_MENU_FONT_W*OSD2_0_MENU_FONT_H)/1L) //bytes

#define OSD2_0_MENU_1BP_TOTAL (1+1L)
#define OSD2_0_MENU_2BP_TOTAL (0)
#define OSD2_0_MENU_4BP_TOTAL (0)

#define OSD2_0_MENU_TOTAL_LENGTH ((OSD2_0_MENU_1BP_SIZE*OSD2_0_MENU_1BP_TOTAL)+(OSD2_0_MENU_2BP_SIZE*OSD2_0_MENU_2BP_TOTAL)+(OSD2_0_MENU_4BP_TOTAL*OSD2_0_MENU_4BP_SIZE))
extern unsigned short code osd2_0_menu_menuTable[];

#define _ICON_SPACE_W12_H18_1BP_WIDTH  1
#define _ICON_SPACE_W12_H18_1BP_HEIGHT 1
extern unsigned char code _icon_space_w12_h18_1bp[];

#define _ROM_FONT_1_WIDTH  2
#define _ROM_FONT_1_HEIGHT 1
extern unsigned char code _rom_font_1[];

#define _ROM_FONT_2_WIDTH  2
#define _ROM_FONT_2_HEIGHT 1
extern unsigned char code _rom_font_2[];

#define _ROM_FONT_3_WIDTH  2
#define _ROM_FONT_3_HEIGHT 1
extern unsigned char code _rom_font_3[];

#define _ROM_FONT_4_WIDTH  1
#define _ROM_FONT_4_HEIGHT 1
extern unsigned char code _rom_font_4[];

#define _ROM_FONT_5_WIDTH  1
#define _ROM_FONT_5_HEIGHT 1
extern unsigned char code _rom_font_5[];

#define _ICON_DUMMY_W12_H18_1BP_WIDTH  1
#define _ICON_DUMMY_W12_H18_1BP_HEIGHT 1
extern unsigned char code _icon_dummy_w12_h18_1bp[];

