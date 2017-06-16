/**
 *  @file   c_root.c
 *  @brief  main program for demo code
 *  $Id: c_root.c,v 1.4 2012/12/17 05:30:48 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/06/04  ken 	New file.
 *
 */

#include <reg51.h>
#include "sys.h"
#include "tw_widget_sys.h"
#include "spiosd.h"
#include "panel.h"
#include "check_event.h"
#include "rc.h"
#include "osd2api.h"
#include "video.h"
#include "gpio.h"
#include "videodetect.h"

/* init.c */
extern void init(void);
extern void nvp6114a_outport_1mux_chseq_mode(unsigned char vformat,unsigned char mode);

//SPI_CLK   GPC6 
//SPI_DI    GPC5
GPIO GPC5 = {OUTPUT,OFF,OFF,HIGH,5,GPIOC};  //DI DATA
GPIO GPC6 = {OUTPUT,OFF,OFF,HIGH,6,GPIOC};  //CLK




void reset_nvp6114a(void)
{
    GPIO GPA5 = {OUTPUT,OFF,OFF,HIGH,5,GPIOA};
	GPIO GPA6 = {OUTPUT,OFF,OFF,HIGH,6,GPIOA};
	GPIO GPA7 = {OUTPUT,OFF,OFF,HIGH,7,GPIOA};

	GPA5.gpio_out_val = HIGH;
	setgpio(GPA5, 5);
	
	GPA5.gpio_out_val = LOW;
	setgpio(GPA5, 5);
	twdDelay(1);
	GPA5.gpio_out_val = HIGH;
	setgpio(GPA5, 5);
    //twdDelay(1000);
    setgpio(GPA6, 6);
	setgpio(GPA7, 7);
}


void SPI_SendData(unsigned char i)
{
	unsigned char n;
	for (n=0; n<8; n++)
	{
		GPC6.gpio_out_val = LOW;
		setgpio(GPC6, 6);
		if ( (i&0x80)!=0 )
		{
			GPC5.gpio_out_val = HIGH;
			setgpio(GPC5, 5);
		}
		else
		{
			GPC5.gpio_out_val = LOW;
			setgpio(GPC5, 5);
		}
		GPC6.gpio_out_val = HIGH;
		setgpio(GPC6, 6);
		i<<=1;
	}
}

void SPI_WriteComm(unsigned char i)
{
	GPC6.gpio_out_val = LOW;
	setgpio(GPC6, 6);
	GPC5.gpio_out_val = LOW;
	setgpio(GPC5, 5);
	GPC6.gpio_out_val = HIGH;
	setgpio(GPC6, 6);
	SPI_SendData(i);
}




void SPI_WriteData(unsigned char i)
{ 
	GPC6.gpio_out_val = LOW;
	setgpio(GPC6, 6);
	GPC5.gpio_out_val = HIGH;
	setgpio(GPC5, 5);
	GPC6.gpio_out_val = HIGH;
	setgpio(GPC6, 6);
	SPI_SendData(i);
} 


/*  模拟io 写spi 初始化数据lzx  */
#if 0
void LCD_Init(void)
{
	
			//SPI_RES=1;
			//Delay(50);
            twdDelay(50);

			//SPI_RES=0;
			//Delay(800);
			//twdDelay(800);
			//SPI_RES=1;
			//Delay(800);
			
			twdDelay(800);
	SPI_WriteComm(0x11);
	//Delay(120); //Delay 120ms
	//twdDelay(700);
	//--------------------------------Display and color format setting----------------------------//
	SPI_WriteComm(0x36);
	SPI_WriteData(0xc8);
	SPI_WriteComm(0x3a);
	SPI_WriteData(0x06);
		
	SPI_WriteComm(0xB0);
	SPI_WriteData(0x11);
	SPI_WriteData(0x00);
		
	SPI_WriteComm(0xB1);
	SPI_WriteData(0x40);//0x40
	SPI_WriteData(0x04);	
	SPI_WriteData(0x0a);
	//--------------------------------ST7789S Frame rate setting----------------------------------//
	SPI_WriteComm(0xb2);
	SPI_WriteData(0x0c);
	SPI_WriteData(0x0c);
	SPI_WriteData(0x00);
	SPI_WriteData(0x33);
	SPI_WriteData(0x33);
	SPI_WriteComm(0xb7);
	SPI_WriteData(0x35);
	//---------------------------------ST7789S Power setting--------------------------------------//
	SPI_WriteComm(0xbb);
	SPI_WriteData(0x35);
	SPI_WriteComm(0xc0);
	SPI_WriteData(0x2c);
	SPI_WriteComm(0xc2);
	SPI_WriteData(0x01);
	SPI_WriteComm(0xc3);
	SPI_WriteData(0x11);
	SPI_WriteComm(0xc4);
	SPI_WriteData(0x20);
	SPI_WriteComm(0xc6);
	SPI_WriteData(0x0f);
	SPI_WriteComm(0xd0);
	SPI_WriteData(0xa4);
	SPI_WriteData(0xa1);
	//--------------------------------ST7789S gamma setting---------------------------------------//
	SPI_WriteComm(0xe0);
	SPI_WriteData(0xd0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x06);
	SPI_WriteData(0x09);
	SPI_WriteData(0x0b);
	SPI_WriteData(0x2a);
	SPI_WriteData(0x3c);
	SPI_WriteData(0x55);
	SPI_WriteData(0x4b);
	SPI_WriteData(0x08);
	SPI_WriteData(0x16);
	SPI_WriteData(0x14);
	SPI_WriteData(0x19);
	SPI_WriteData(0x20);
	
	SPI_WriteComm(0xe1);
	SPI_WriteData(0xd0);
	SPI_WriteData(0x00);
	SPI_WriteData(0x06);
	SPI_WriteData(0x09);
	SPI_WriteData(0x0b);
	SPI_WriteData(0x29);
	SPI_WriteData(0x36);
	SPI_WriteData(0x54);
	SPI_WriteData(0x4b);
	SPI_WriteData(0x0d);
	SPI_WriteData(0x16);
	SPI_WriteData(0x14);
	SPI_WriteData(0x21);
	SPI_WriteData(0x20);
	
	SPI_WriteComm(0x29);
	
}
#else
void LCD_Init(void)
{
	
            twdDelay(50);
			
			twdDelay(800);
	SPI_WriteComm(0x11);
//--------------------------------Display and color format setting----------------------------//
SPI_WriteComm(0x36);
SPI_WriteData(0x70);
SPI_WriteComm(0x3a);
SPI_WriteData(0x66);
	
SPI_WriteComm(0xB0);
SPI_WriteData(0x11);
SPI_WriteData(0x80);
	
SPI_WriteComm(0xB1);
SPI_WriteData(0x40);//0x40
SPI_WriteData(0x04);	
SPI_WriteData(0x0a);
//--------------------------------ST7789S Frame rate setting----------------------------------//
SPI_WriteComm(0xb2);
SPI_WriteData(0x0c);
SPI_WriteData(0x0c);
SPI_WriteData(0x00);
SPI_WriteData(0x33);
SPI_WriteData(0x33);

SPI_WriteComm(0xb3);
SPI_WriteData(0x10);
SPI_WriteData(0x00);//0x10
SPI_WriteData(0x00);//0x10

SPI_WriteComm(0xb7);
SPI_WriteData(0x35);
//---------------------------------ST7789S Power setting--------------------------------------//
SPI_WriteComm(0xbb);
SPI_WriteData(0x35);
SPI_WriteComm(0xc0);
SPI_WriteData(0x2c);
SPI_WriteComm(0xc2);
SPI_WriteData(0x01);
SPI_WriteComm(0xc3);
SPI_WriteData(0x11);
SPI_WriteComm(0xc4);
SPI_WriteData(0x20);
SPI_WriteComm(0xc6);
SPI_WriteData(0x0f);
SPI_WriteComm(0xd0);
SPI_WriteData(0xa4);
SPI_WriteData(0xa1);
//--------------------------------ST7789S gamma setting---------------------------------------//
SPI_WriteComm(0xe0);
SPI_WriteData(0xd0);
SPI_WriteData(0x00);
SPI_WriteData(0x06);
SPI_WriteData(0x09);
SPI_WriteData(0x0b);
SPI_WriteData(0x2a);
SPI_WriteData(0x3c);
SPI_WriteData(0x55);
SPI_WriteData(0x4b);
SPI_WriteData(0x08);
SPI_WriteData(0x16);
SPI_WriteData(0x14);
SPI_WriteData(0x19);
SPI_WriteData(0x20);

SPI_WriteComm(0xe1);
SPI_WriteData(0xd0);
SPI_WriteData(0x00);
SPI_WriteData(0x06);
SPI_WriteData(0x09);
SPI_WriteData(0x0b);
SPI_WriteData(0x29);
SPI_WriteData(0x36);
SPI_WriteData(0x54);
SPI_WriteData(0x4b);
SPI_WriteData(0x0d);
SPI_WriteData(0x16);
SPI_WriteData(0x14);
SPI_WriteData(0x21);
SPI_WriteData(0x20);


SPI_WriteComm(0x2a);
SPI_WriteData(0x00);
SPI_WriteData(0x0);
SPI_WriteData(0x01);
SPI_WriteData(0x3f);

SPI_WriteComm(0x2b);
SPI_WriteData(0x00);
SPI_WriteData(0x0);
SPI_WriteData(0x0);
SPI_WriteData(239);

SPI_WriteComm(0x29);

}

#endif

void main(void)
{
	char data rc=0;
	TW_EVENT *events;
    
	init();

	panel_init();
	
	LCD_Init();

	spiosd_init(0x11E, 0x24, 0x99, 0x27);

	//do_calibration(0);		/* touch panel calibration */

	tw_widget_init();

	resource_init();

	osd_init();
    reset_nvp6114a();
	nvp6114a_outport_1mux_chseq_mode(PAL_MODE, NVP6124_VI_720P_2530);
	//DBG_PRINT("one_mux 720 pal\n");
	//VIPInit(VIP_720P_PAL);	
	CVDInit();
	auto_detection_ctrl(ENABLE);
	
	while(1) {
		check_event();
		//get_command();
		events = get_event();
		rc = tw_page_process(events);
		//put_nodify(events, rc);
	}
}


