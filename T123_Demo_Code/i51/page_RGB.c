#include <math.h>
#include <stdio.h>
#include "sys.h"
#include "tw_widget_sys.h"
#include "res.h"
#include "spiosd.h"
#include "osd2api.h"
#include "tw_timer.h"
#include "Iccontrol.h"

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
unsigned char code video_str[]="Video_Input_Test";
unsigned char code seek_str[]="Cable_Searching";
unsigned char code match_str[]="Line_Match_Test";
unsigned char code tel_str[]="Tele_Line_Test";
unsigned char code sys_str[]="System_Setting";

unsigned char code title_str[]="Select_Frequency";
unsigned char code high_f_str[]="High_Freq->3KHz";
unsigned char code low_f_str[]="Low_Freq->1KHz";

unsigned char code video_f_str1[]="video_test_ongoing";
unsigned char code video_f_str2[]="press_MENU_key_back";

unsigned char code tel_f_str1[]="Line_Inject_Test";
unsigned char code tel_f_str2[]="Loop_Back_Test";


unsigned char code brightness_str[]="BRIGHTNESS";
unsigned char code contrast_str[]="CONTRAST";
unsigned char code color_str[]= "COLOR";

unsigned char highlight_flag[5]= {5,2,2,2,2};
unsigned char item_num = 0;

extern bit CD4017_EN_FLAG;
extern bit CD4066_EN_FLAG;
extern bit VIDEO_FLAG;
extern bit TEL0_FLAG;
extern bit TEL1_FLAG;

extern unsigned char dur_cb_sc; /*duration for cable searching*/

typedef enum 
{
	Main_M=1,
	Bright_M,
	Contrast_M,
	Color_M,
	Freq_M,
	Video_M,
	Tel_M,
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
	
	//osd_clear_block(0,0,15,2,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);
	//osd_clear_menu(CUR_MENU_P->addr, CUR_MENU_P->width, CUR_MENU_P->height);
	//tw_osd_clear_menu();

	if(item==Main_M)
	{
		temp = page_rgb_rom_font_1;/*tubiao*/
		temp.x = 6;
		temp.y = 0;
		tw_icons_draw(&temp);
		
		//DBG_PRINT("OSDShowMenu color of line match = %bd\r\n", highlight_flag[2]);

		OSDShow1BPString(video_str,6,2,highlight_flag[0]);/*biaoti*/
		OSDShow1BPString(seek_str,6,4,highlight_flag[1]);/*biaoti*/
		OSDShow1BPString(match_str,6,6,highlight_flag[2]);/*biaoti*/
		OSDShow1BPString(tel_str,6,8,highlight_flag[3]);/*biaoti*/
		OSDShow1BPString(sys_str,6,10,highlight_flag[4]);/*biaoti*/
	}
	else if(item==Bright_M)
	{

		//osd_clear_block(0,0,25,12,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);

		tw_osd_clear_menu();
		temp = page_rgb_rom_font_1;/*tubiao*/
		temp.x = 4;
		temp.y = 0;
		tw_icons_draw(&temp);
		//OSDShow1BPString(brightness_str,4,1,2);
		OSDShow1BPString(brightness_str,8,0,2);/*biaoti*/
		//OSDShowGauge(Bright1T,1,2,2);
		OSDShowGauge(Bright1T,6,2,2); /*daizhuangtiao*/
		sprintf(ss,"%03d",(int)Bright1T);
		//OSDShow1BPString(ss,8,1,2);
		OSDShow1BPString(ss,11,1,2); /*shuzixianshi*/
	}
	else if(item==Contrast_M)
	{
		//osd_clear_block(0,0,25,12,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);

		tw_osd_clear_menu();
		temp = page_rgb_rom_font_2;
		temp.x = 4;
		temp.y = 0;
		tw_icons_draw(&temp);
		//OSDShow1BPString(contrast_str,14,16,1);
		OSDShow1BPString(contrast_str,8,0,2);
		//OSDShowGauge(ContrastT,11,17,1);
		OSDShowGauge(ContrastT,6,2,2);
		sprintf(ss,"%03d",(int)ContrastT);
		//OSDShow1BPString(ss,33,16,1);
		OSDShow1BPString(ss,11,1,2);
	}
	else if(item==Color_M)
	{
		//osd_clear_block(0,0,25,12,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);

		tw_osd_clear_menu();
		temp = page_rgb_rom_font_3;
		temp.x = 4;
		temp.y = 0;
		tw_icons_draw(&temp);
		//OSDShow1BPString(color_str,14,16,1);
		OSDShow1BPString(color_str,8,0,2);
		//OSDShowGauge(ColorT,11,17,1);
		OSDShowGauge(ColorT,6,2,2);
		sprintf(ss,"%03d",(int)ColorT);
		//OSDShow1BPString(ss,33,16,1);
		OSDShow1BPString(ss,11,1,2);
		if(dur_cb_sc==1)
		{
			highlight_flag[0] = 1;
			highlight_flag[1] = 2;
		}
		else
		{
			highlight_flag[0] = 2;
			highlight_flag[1] = 1;
		}

	}
	else if(item==Freq_M)
	{
		//osd_clear_block(0,0,25,12,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);

		tw_osd_clear_menu();
		temp = page_rgb_rom_font_4;
		temp.x = 4;
		temp.y = 0;
		tw_icons_draw(&temp);

		DBG_PRINT("OSDShowMenu highlight_flag[0] = %bd\r\n", highlight_flag[0]);
		DBG_PRINT("OSDShowMenu highlight_flag[1] = %bd\r\n", highlight_flag[1]);
		
		OSDShow1BPString(title_str,6,0,2);
		OSDShow1BPString(high_f_str,6,4,highlight_flag[0]);
		OSDShow1BPString(low_f_str,6,6,highlight_flag[1]);
	}
	else if(item==Video_M)
	{
		//osd_clear_block(0,0,25,12,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);

		tw_osd_clear_menu();
		OSDShow1BPString(video_f_str1,2,4,2);
		OSDShow1BPString(video_f_str2,2,6,2);
		VIDEO_FLAG = 1;
		DBG_PRINT("VIDEO_M enable\r\n");
		
	}
	else if(item==Tel_M)
	{
		//osd_clear_block(0,0,25,12,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);

		tw_osd_clear_menu();
				
		OSDShow1BPString(tel_f_str1,4,4,highlight_flag[0]);
		OSDShow1BPString(tel_f_str2,4,6,highlight_flag[1]);

		DBG_PRINT("Telephone test\r\n");
		
	}	
}

void OSDMenuAdj(unsigned char item,char dir)
{
	unsigned char i;

	//if((highlight_flag[2]==3)||(highlight_flag[1]==3))
	if((item==1)&&(dir==1))
	{
		for(i=0;i<5;i++)
		{
			highlight_flag[i] = 2;
		}
		item_num++;
		if(item_num>4)
		item_num = 0;
		highlight_flag[item_num] = 5;
	}
	else if((item==1)&&(dir!=1))
	{
		for(i=0;i<5;i++)
		{
			highlight_flag[i] = 2;
		}
		if(item_num>0)
		item_num--;
		else
		item_num = 4;
		highlight_flag[item_num] = 5;
	}
	
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
	else if(item==Freq_M) /*to be achieved*/
	{
		//DBG_PRINT("highlight_flag[0] = %bd\r\n", highlight_flag[0]);
		
		if(highlight_flag[0]==1)
		{
			dur_cb_sc = 3; /*1KHz*/
			highlight_flag[0] = 2;
			highlight_flag[1] = 1;
			
		}
		else
		{
			dur_cb_sc = 1; /*3KHz*/
			highlight_flag[0] = 1;
			highlight_flag[1] = 2;
		}
	}
	else if(item==Tel_M) /*to be achieved*/
	{
		//DBG_PRINT("highlight_flag[0] = %bd\r\n", highlight_flag[0]);
		
		if(highlight_flag[0]==1)
		{
			TEL0_FLAG = 0;
			TEL1_FLAG = 1; /*self test*/
			highlight_flag[0] = 2;
			highlight_flag[1] = 1;
			
		}
		else
		{
			TEL1_FLAG = 0;
			TEL0_FLAG = 1; /*line inject test*/
			highlight_flag[0] = 1;
			highlight_flag[1] = 2;

		}
	}
	DBG_PRINT("item = %bd\r\n", item);
	OSDShowMenu(item);
}

void OSDMenu_ctr(void)
{
	
	if(MenuIndex==1)
	{
		switch(item_num)
		{
			case 0:
			/*video input test*/			
			VIDEO_FLAG = 1;
			OpenPanel();
			DBG_PRINT("VIDEO_FLAG = 1\r\n"); 

			#if 0
			highlight_flag[0] = 3;
			VIDEO_FLAG = ~VIDEO_FLAG;
			if(VIDEO_FLAG)
			DBG_PRINT("VIDEO_FLAG = 1\r\n");
			#endif
			
			MenuIndex = 6;
			break;
			case 1:
			/*seek cable*/
			CD4066_EN_FLAG = 1;
			highlight_flag[1] = 1;
			break;
			case 2:
			CD4017_EN_FLAG = 1;
			//CD4017_EN_FLAG = ~CD4017_EN_FLAG;
			highlight_flag[2] = 1;
			#endif 
			break;
			case 3:
				
			highlight_flag[0] = 1;
			highlight_flag[1] = 2;

			TEL1_FLAG = 0;

			TEL0_FLAG = 1;
			
			MenuIndex = 7;

			break;
			case 4:
			MenuIndex = 2;
			break;
			default:

			break;
		}
	}
	else
	if(MenuIndex==2)/*Brightness*/
	{MenuIndex = 3;}
	else
	if(MenuIndex==3)/*Contrast*/
	{MenuIndex = 4;}
	else
	if(MenuIndex==4)/*Color*/
	{MenuIndex = 5;}
	else
	if(MenuIndex==5)/*Freq*/
	{
		highlight_flag[0] = 2;
		highlight_flag[1] = 2;
		highlight_flag[2] = 2;
		highlight_flag[3] = 2;
		highlight_flag[4] = 5;
		
		MenuIndex = 1;
		tw_osd_clear_menu();//when come back to first page, need clean the old display.
	}
	else
	if(MenuIndex==6)/*Video*/
	{
		VIDEO_FLAG = 0;

		ClosePanel(0x1D, 0xF0, 0x6C);
		DBG_PRINT("VIDEO_FLAG = 0\r\n"); 

		MenuIndex = 1;
		tw_osd_clear_menu();//when come back to first page, need clean the old display.
	}
	else
	if(MenuIndex==7)/*Tel*/
	{
		TEL0_FLAG = 0;
		TEL1_FLAG = 0;

		highlight_flag[0] = 2;
		highlight_flag[1] = 2;
		highlight_flag[2] = 2;
		highlight_flag[3] = 5;
		highlight_flag[4] = 2;
			
		MenuIndex = 1;
		tw_osd_clear_menu();//when come back to first page, need clean the old display.
	}

#if 0
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
#endif
	OSDShowMenu(MenuIndex);
	//DBG_PRINT("m_bOSDEnable = %x\n", (unsigned short)m_bOSDEnable);
	DBG_PRINT("MenuIndex = %x\n", (unsigned short)MenuIndex);
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

			tw_osd_clear_menu();//clean the display.

#if 1		// 开机显示osd，不用是关闭该段代码
			m_bOSDEnable = 1;
			MenuIndex = 1;
			OSDShowMenu(MenuIndex);
			tw_timer_open(500);
#endif
		break;

		case TW_EVENT_TYPE_QUIT_SYSTEM:
		break;

		default:	/* put to nodify if no process this event */
			key = ReadKey();
			//DBG_PRINT("Key = %bd\n", key);
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
				//DBG_PRINT("switch case Key value = %d\n", key);
				switch(key)
				{
					case kyMENU:
						OSDMenu_ctr();
						DBG_PRINT("Key = kyMENU \n");
						break;

					case kyADD:
						//DBG_PRINT("MenuIndex in Key scan = %bd\r\n", MenuIndex);
						OSDMenuAdj(MenuIndex,1);
						DBG_PRINT("Key = kyADD \n");
						break;

					case kyDEC:
						OSDMenuAdj(MenuIndex,-1);
						DBG_PRINT("Key = kyDEC \n");
						break;
					default:
						DBG_PRINT("Key = default \n");
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
				//MenuIndex=0x00;
				osd_disable();
				osd_clear_block(0,16,48,2,page_rgb_osd2_0_menu.addr,page_rgb_osd2_0_menu.width);
				osd_enable();
			}

			return TW_RETURN_NO_PROCESS;
	}
	return TW_RETURN_NONE;
}
