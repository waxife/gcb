#include <math.h>
#include <stdio.h>
#include "sys.h"
#include "tw_widget_sys.h"
#include "res.h"
#include "spiosd.h"
#include "osd2api.h"
#include "tw_timer.h"

#include "page_RGB.h"

#define	K_OFFSET			8

#define 	kyADD			1	
#define 	kyMENU  			2
#define 	kyDEC			3	
#define 	kySOURCE	  	4
#define 	kyPWR			5
#define 	kyNo		  		30

#define 	OSD_MIN			0
#define 	OSD_MID			50
#define 	OSD_MAX			100
#define 	OSD_GAUGE_W		10

///////////////////////////////////////////////////////////////////////////////
unsigned char code brightness_str[]="BRIGHTNESS";
unsigned char code contrast_str[]="CONTRAST";
unsigned char code color_str[]= "COLOR";
typedef enum 
{
	Bright_M=1,
	Contrast_M,
	Color_M,
	End_Menu,
} OSD_MENU;
///////////////////////////////////////////////////////////////////////////////

unsigned char key,flagkey=0;
unsigned char m_bOSDEnable;
unsigned char MenuIndex;
char Bright1T;
char ContrastT;
char ColorT;

unsigned char ReadKey(void)
{
	unsigned char i,j;
	unsigned char temp,temp1;
	unsigned char value=0x00;
	temp =IC_ReadByte(TWIC_P3, 0x24);
	
	j=0;
	for(i=0;i<10;i++)
	{
		twdDelay(20);
		temp1 =IC_ReadByte(TWIC_P3, 0x24);
		if(abs(temp1-temp)<K_OFFSET) j++;
	}
	if(j<8) return 0;
	
	if((temp1>=(0x80-K_OFFSET))&&(temp1<=(0x80+K_OFFSET)))
	{
		value=kyADD;  // +
	}
	else if((temp1>=(0x55-K_OFFSET))&&(temp1<=(0x55+K_OFFSET)))
	{
		value=kyMENU;  // menu
	}
	else if((temp1>=(0x2a-K_OFFSET))&&(temp1<=(0x2a+K_OFFSET)))
	{
		value=kyDEC;  // —
	}
		

	return value;
	
}

void OSDShow1BPString(unsigned char*Str, unsigned char x, unsigned char y, unsigned char color)
{
	unsigned char len;
	unsigned char ss[50];
	struct tw_icons temp ;
	unsigned char *pStr = Str;
	
	temp = page_rgb_rom_font_1;
	len = 0;
	while((*pStr) != 0) 
	{
		ss[len] = (*pStr++)-'0'+ 38; // 38是字符0的索引号
		len++;
	}
	temp.x = x;
	temp.y = y;
	temp.width = len;
	temp.table = ss;
	temp.color = color;
	tw_icons_draw(&temp);
}

void OSDShowGaugeFont(unsigned short index, unsigned char x, unsigned char y, unsigned char color)
{
	unsigned char ss[5];
	struct tw_icons tempIcons;
	ss[0] = index+3; // 3是进度条的起始索引号
	ss[1] = 0;
	tempIcons = page_rgb_rom_font_5;
	tempIcons.x = x;
	tempIcons.y = y;
	tempIcons.table = ss;
	tempIcons.color = color;
	tw_icons_draw(&tempIcons);
}

void OSDShowGauge(unsigned char value, unsigned char x, unsigned char y, unsigned char cColor)
{
	unsigned short i,temp;

	if(value>OSD_MAX) value=OSD_MAX;
	if(value<OSD_MIN) value=OSD_MIN;
	temp=(OSD_GAUGE_W*value);
	temp=temp/OSD_MAX;
	OSDShowGaugeFont(5,x++,y,cColor);
	for(i=0;i<temp;i++) OSDShowGaugeFont(4,x++,y,cColor);
	i++; 
	if(i<=OSD_GAUGE_W)
	{
		temp=4*OSD_GAUGE_W*value;
		temp=temp/OSD_MAX%4;
		switch (temp)
		{
			case 0:	OSDShowGaugeFont(0,x++,y,cColor);	break;
			case 1:	OSDShowGaugeFont(1,x++,y,cColor);	break;
			case 2:	OSDShowGaugeFont(2,x++,y,cColor);	break;
			case 3:	OSDShowGaugeFont(3,x++,y,cColor);	break;
		}
	}
	for(;i<OSD_GAUGE_W;i++) OSDShowGaugeFont(0,x++,y,cColor);
	OSDShowGaugeFont(6,x++,y,cColor);
}

void OSDShowMenu(unsigned char item)
{
	char ss[5];
	struct tw_icons temp;
	osd_clear_block(0,0,15,2,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);
	if(item==Bright_M)
	{
		temp = page_rgb_rom_font_1;
		temp.x = 1;
		temp.y = 1;
		tw_icons_draw(&temp);
		OSDShow1BPString(brightness_str,4,1,2);
		OSDShowGauge(Bright1T,1,2,2);
		sprintf(ss,"%03d",(int)Bright1T);
		OSDShow1BPString(ss,8,1,2);
	}
	else if(item==Contrast_M)
	{
		temp = page_rgb_rom_font_2;
		temp.x = 12;
		temp.y = 16;
		tw_icons_draw(&temp);
		OSDShow1BPString(contrast_str,14,16,1);
		OSDShowGauge(ContrastT,11,17,1);
		sprintf(ss,"%03d",(int)ContrastT);
		OSDShow1BPString(ss,33,16,1);
	}
	else if(item==Color_M)
	{
		temp = page_rgb_rom_font_3;
		temp.x = 12;
		temp.y = 16;
		tw_icons_draw(&temp);
		OSDShow1BPString(color_str,14,16,1);
		OSDShowGauge(ColorT,11,17,1);
		sprintf(ss,"%03d",(int)ColorT);
		OSDShow1BPString(ss,33,16,1);
	}
}

void OSDMenuAdj(unsigned char item,char dir)
{
	if(item==Bright_M)
	{
		Bright1T=Bright1T+dir;
		if(Bright1T>OSD_MAX)		Bright1T=OSD_MAX;
		if(Bright1T<OSD_MIN)		Bright1T=OSD_MIN;
	}
	else if(item==Contrast_M)	
	{
		ContrastT=ContrastT+dir;
		if(ContrastT>OSD_MAX)	ContrastT=OSD_MAX;
		if(ContrastT<OSD_MIN)	ContrastT=OSD_MIN;
		
	}
	else if(item==Color_M)
	{
		ColorT=ColorT+dir;
		if(ColorT>OSD_MAX)		ColorT=OSD_MAX;
		if(ColorT<OSD_MIN)		ColorT=OSD_MIN;
	}
	OSDShowMenu(item);
}

void OSDMenu_ctr(void)
{
	if(!m_bOSDEnable)	
	{
		m_bOSDEnable=1;
		MenuIndex=0x01;
	}
	else MenuIndex++;
	
	if(MenuIndex>=End_Menu) 
	{
		m_bOSDEnable=0;
		MenuIndex=0x00;
		osd_clear_block(0,16,48,2,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);
	}
	else	OSDShowMenu(MenuIndex);
	//DBG_PRINT("m_bOSDEnable = %x\n", (unsigned short)m_bOSDEnable);
	//DBG_PRINT("MenuIndex = %x\n", (unsigned short)MenuIndex);
}

unsigned char page_RGB_process (TW_EVENT* event)
{
	switch(event->type)
	{
		case TW_EVENT_TYPE_ENTER_SYSTEM:
			sosd_sp_disable();
			osd_disable();
			tw_emu_exit();
			tw_menu_draw(&page_rgb_osd2_0_menu);
			//tw_icons_draw(&page_rgb_rom_font_1);
			osd_enable();
			m_bOSDEnable = 0;
			MenuIndex = 0;
			Bright1T = 48;
			ContrastT = 50;
			ColorT = 52;
			IC_WritByte( TWIC_P0,0x69, Bright1T*2 + 28);
			IC_WritByte( TWIC_P0,0x68, ContrastT*2 + 28 );
			IC_WritByte( TWIC_P0,0x6c, ColorT*2 + 28 );

#if 1		// 开机显示osd，不用是关闭该段代码
			m_bOSDEnable = 1;
			MenuIndex = 1;
			OSDShowMenu(MenuIndex);
			//tw_timer_open(500);
#endif
		break;

		case TW_EVENT_TYPE_QUIT_SYSTEM:
		break;

		default:	/* put to nodify if no process this event */
			key = ReadKey();
			if(!key)flagkey=1;	//判断按键是否弹起。
			
			if((kyDEC==key)||(kyADD==key)) // 这两个按键不弹起也强行使能,长按连续有效
			{
				twdDelay(600);		// 调节按键长按的反应速度
				flagkey=1;
			}
			
			if(flagkey && key ) //之前已经有释放 ，且按键有按下
			{
				flagkey=0;
				if(key<kyPWR)
				{
					osd_enable();
					tw_timer_open(500);
				}
				switch(key)
				{
					case kyMENU:
						OSDMenu_ctr();
						break;

					case kyADD:
						OSDMenuAdj(MenuIndex,1);
						break;

					case kyDEC:
						OSDMenuAdj(MenuIndex,-1);
						break;
					default:
						break;
				}
				IC_WritByte( TWIC_P0,0x69, Bright1T*2 + 28);
				IC_WritByte( TWIC_P0,0x68, ContrastT*2 + 28 );
				IC_WritByte( TWIC_P0,0x6c, ColorT*2 + 28 );
			}
			if(TW_TIMER_OUT_FLAG)
			{
				tw_timer_close();
				m_bOSDEnable=0;
				MenuIndex=0x00;
				osd_disable();
				osd_clear_block(0,16,48,2,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);
				osd_enable();
			}

			return TW_RETURN_NO_PROCESS;
	}
	return TW_RETURN_NONE;
}
