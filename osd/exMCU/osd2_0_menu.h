#define OSD2_0_MENU_WIDTH  27
#define OSD2_0_MENU_HEIGHT 14
#define OSD2_0_MENU_MENU_START_ADDR 0x3E4B
#define OSD2_0_MENU_MENU_END_ADDR 0x3FFD
#define OSD2_0_MENU_SIZE ((OSD2_0_MENU_WIDTH*2+8)*OSD2_0_MENU_HEIGHT) //byte
extern unsigned char code osd2_0_menu[];
