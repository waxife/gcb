/**
 *  @file   c_root.c
 *  @brief  main program for demo code
 *  $Id: c_root.c,v 1.4 2012/12/17 05:30:48 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2012 Terawins 
 Inc. All rights reserved.
 * 
 *  @date   2012/06/04  ken 	New file.
 *
 */
#include <math.h>
#include <stdio.h>

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
#include "Tw_timer.h"
#include "timer.h"
#include "Iccontrol.h"


/* init.c */
extern void init(void);
extern void nvp6114a_outport_1mux_chseq_mode(unsigned char vformat,unsigned char mode);
extern unsigned char highlight_flag[5];
extern void OSDShowMenu(unsigned char item);//
extern SigState sigstate;
extern unsigned char MenuIndex;

#define H_FREQ 1
#define L_FREQ 3

#define	K_OFFSET			8

#define 	kyADD			1	
#define 	kyMENU  			2
#define 	kyDEC			3	


/*define SPI CLK and DATA*/
//SPI_DI    GPC5
//SPI_CLK   GPC6
GPIO GPC5 = {OUTPUT,OFF,OFF,HIGH,5,GPIOC};
GPIO GPC6 = {OUTPUT,OFF,OFF,HIGH,6,GPIOC};

/*Globle Variable*/
unsigned char aaa;
unsigned char bbb;
bit CD4017_EN_FLAG = 0;
bit CD4066_EN_FLAG = 0;
bit VIDEO_FLAG = 0;
bit TEL0_FLAG = 0;
bit TEL1_FLAG = 0;

unsigned char dur_cb_sc; /*duration for cable searching*/


unsigned char GetKey(void)
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
		value=kyDEC;  // ¡ª
	}
		

	return value;
	
}


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


/*Data Transimission Over Simulated SPI*/
void SPI_SendData(unsigned char i)
{  
	unsigned char n;
	for(n=0; n<8; n++)			
	{       
		//SPI_CLK=0;
		GPC6.gpio_out_val = LOW;
		setgpio(GPC6, 6);
		//_nop_(); _nop_();_nop_();_nop_();
		//Nop();
		//SPI_DI=i&0x80;
		if( (i&0x80)!=0 )
		{
			GPC5.gpio_out_val = HIGH;
			setgpio(GPC5, 5);
		}
		else
		{
			GPC5.gpio_out_val = LOW;
			setgpio(GPC5, 5);
		}	
		//_nop_(); _nop_();_nop_();_nop_();
		//Nop();
		//SPI_CLK=1;
		GPC6.gpio_out_val = HIGH;
		setgpio(GPC6, 6);
		i<<=1;
		//_nop_();_nop_();_nop_();_nop_();
		//Nop();
	}
}

/*Write Command To LCD*/
void SPI_WriteComm(unsigned char i)
{
	//SPI_CS=0;
	//  _nop_(); _nop_();_nop_();_nop_();
	//Nop();
	// SPI_CLK=0;
	GPC6.gpio_out_val = LOW;
	setgpio(GPC6, 6);
	//_nop_(); _nop_();_nop_();_nop_();
	//Nop();
	//SPI_DI=0;
	GPC5.gpio_out_val = LOW;
	setgpio(GPC5, 5);
	//_nop_(); _nop_();_nop_();_nop_();
	//Nop();
	//SPI_CLK=1;
	GPC6.gpio_out_val = HIGH;
	setgpio(GPC6, 6);
	SPI_SendData(i);
	//_nop_(); _nop_();_nop_();_nop_();
	//Nop();
	//  SPI_CS=1;
}

/*Write Data To LCD*/
void SPI_WriteData(unsigned char i)
{ 
	//SPI_CS=0;
	//_nop_(); _nop_();_nop_();_nop_();
	//Nop();
	//SPI_CLK=0;
	GPC6.gpio_out_val = LOW;
	setgpio(GPC6, 6);
	//_nop_(); _nop_();_nop_();_nop_();
	//Nop();
	//SPI_DI=1;
	GPC5.gpio_out_val = HIGH;
	setgpio(GPC5, 5);
	//_nop_(); _nop_();_nop_();_nop_();
	//Nop();
	//SPI_CLK=1;
	GPC6.gpio_out_val = HIGH;
	setgpio(GPC6, 6);
	SPI_SendData(i);
	//_nop_(); _nop_();_nop_();_nop_();
	//Nop();
	//SPI_CS=1;
} 


/*  LCD init  */
#if 0
void LCD_Init(void)
{
	//SPI_RES=1;
	//Delay(50);
	twdDelay(50);
	//SPI_RES=0;
	//Delay(800);
	twdDelay(800);
	//SPI_RES=1;
	//Delay(800);
	twdDelay(800);
	SPI_WriteComm(0x11);
	//Delay(120); //Delay 120ms
	twdDelay(700);
	//--------------------------------Display and color format setting----------------------------//
	SPI_WriteComm(0x36);
	SPI_WriteData(0x00);
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
	unsigned char tm_flag;
	unsigned char ln_status; /*To save the line number*/
	unsigned long tr1_val;
	unsigned short tr1_freq;
	unsigned long i;
	unsigned char temp;
	/*Set default status of GPIOC*/
	GPIO GPC0 = {OUTPUT,OFF,OFF,HIGH,0,GPIOC}; /*CD4017 clock*/
	GPIO GPC1 = {OUTPUT,OFF,OFF,HIGH,1,GPIOC}; /*CD4017 reset*/
	GPIO GPC2 = {OUTPUT,OFF,OFF,HIGH,2,GPIOC}; /*High four bits control*/
	GPIO GPC3 = {OUTPUT,OFF,OFF,HIGH,3,GPIOC}; /*Line0 control*/
	GPIO GPC4 = {OUTPUT,OFF,OFF,HIGH,4,GPIOC}; /*Line1 control*/
	GPIO GPC5 = {OUTPUT,OFF,OFF,HIGH,5,GPIOC}; /*LCD SPI data*/
	GPIO GPC6 = {OUTPUT,OFF,OFF,HIGH,6,GPIOC}; /*LCD SPI clock*/
	GPIO GPB6 = {OUTPUT,OFF,OFF,HIGH,6,GPIOB}; /*SW enable*/
	
	/*Set default value for variable*/
	ln_status = 0;
	tm_flag = 0;
	VIDEO_FLAG =0;
	
	init();

	#if 1 //No panel was defined
	panel_init();
	#endif
	
	LCD_Init();

	spiosd_init(0x11E, 0x24, 0x99, 0x27);

	//do_calibration(0);		/* touch panel calibration */

	tw_widget_init();

	resource_init();/*has not been validated*/

	#if 1
	osd_init();/*has not been validated*/
	#endif
	
    //reset_nvp6114a();
	//nvp6114a_outport_1mux_chseq_mode(PAL_MODE, NVP6124_VI_720P_2530);
	//DBG_PRINT("one_mux 720 pal\n");
	//VIPInit(VIP_720P_PAL);	
	CVDInit();
	ClosePanel(0x1D, 0xF0, 0x6C);
	//OpenPanel();
	auto_detection_ctrl(DISABLE);
	//auto_detection_ctrl(ENABLE);
	
	#if 1
	/*CD4017 reset process*/
	GPC1.gpio_out_val = LOW;
	setgpio(GPC1, 1);
	twdDelay(10);
	GPC1.gpio_out_val = HIGH;
	setgpio(GPC1, 1);
	DBG_PRINT("CD4017 reset done \r\n");
	
	GPB6.gpio_out_val = HIGH; /*CD4066 was switched on/off, High->off; LOW->on*/
	setgpio(GPB6, 6);
	DBG_PRINT("Line 9V was switched ON\r\n");
	
	GPC2.gpio_out_val = HIGH; /*Enable D1,2,3,4, D9, RING power supply*/
	setgpio(GPC2, 2);
	DBG_PRINT("Enable D1,2,3,4, D9, RING power supply\r\n");
	
	GPC3.gpio_out_val = HIGH; /*Set line0 to high*/
	setgpio(GPC3, 3);
	DBG_PRINT("Set line0 to high\r\n");
	
	GPC4.gpio_out_val = HIGH; /*Set line1 to high*/
	setgpio(GPC4, 4);
	DBG_PRINT("Set line1 to high\r\n");
	#endif
	
	#if 0 /*TW timer*/
	tw_timer_open(100);
	DBG_PRINT("TW_Timer Start, interrupt happen every 2 second\r\n");
	#endif
	
	#if 0
	tr1_freq = 0; 
	tr1_val = 1;
	timer1_init(tr1_freq);
	tr1_open(tr1_val);
	DBG_PRINT("TR1 run with setting = %d\r\n");
	#endif
	
	CD4017_EN_FLAG = 0;
	CD4066_EN_FLAG = 0;
	dur_cb_sc = H_FREQ; 
	
	temp =IC_ReadByte(TWIC_P3, 0x24);
	DBG_PRINT("temp = %bd\r\n", temp);
	
	DBG_PRINT("Process run\r\n");
	


	#if 0
	while(1)
	{
		temp = IC_ReadByte(TWIC_P0,0x91);
		DBG_PRINT("TWIC_P0 = %bd\r\n", temp);
		twdDelay(10000); 
	}
	#endif

	while(1)
	{
		if(CD4066_EN_FLAG)
		{
			/*CD4017 reset process*/
			GPC1.gpio_out_val = LOW;
			setgpio(GPC1, 1);
			twdDelay(10);
			GPC1.gpio_out_val = HIGH;
			setgpio(GPC1, 1);
			DBG_PRINT("CD4017 reset done \r\n");
			
			temp =IC_ReadByte(TWIC_P3, 0x24); /*required, to clean the key buffer.*/
			DBG_PRINT("temp1 = %bd\r\n", temp); /*after readbyte, the key value was updated to -1*/
			
			/*no sure below commanded is necessary*/
			twdDelay(1000); 
		
			temp =IC_ReadByte(TWIC_P3, 0x24);
			DBG_PRINT("temp2 = %bd\r\n", temp); 
			
			check_event();
			events = get_event();
			rc = tw_page_process(events);

			temp =IC_ReadByte(TWIC_P3, 0x24);
			DBG_PRINT("temp3 = %bd\r\n", temp);
			
			/**************************************/
			

			/*Main process for seeking the cable.*/			
			while(1)
			{
				for(tm_flag=0;tm_flag<dur_cb_sc;tm_flag++)
				{
					//GPC0.gpio_out_val = LOW;
					//setgpio(GPC0, 0);
					GPB6.gpio_out_val = LOW; /*CD4066 was switched on/off, High->off; LOW->on*/
					setgpio(GPB6, 6);
					if(i!=1)
					{i=1;i=2;}
				}
				for(tm_flag=0;tm_flag<dur_cb_sc;tm_flag++)
				{
					//GPC0.gpio_out_val = HIGH;
					//setgpio(GPC0, 0);	
					GPB6.gpio_out_val = HIGH; /*CD4066 was switched on/off, High->off; LOW->on*/
					setgpio(GPB6, 6);
					if(i!=1)
					{i=1;i=2;}
				}

				//DBG_PRINT("IC_ReadByte(TWIC_P3, 0x24) = %x \r\n", IC_ReadByte(TWIC_P3, 0x24));

#if 1
				if(IC_ReadByte(TWIC_P3, 0x24)!=-1) /*if get the key of right and left, processing stop.*/
				{
					CD4066_EN_FLAG = 0;
					temp =IC_ReadByte(TWIC_P3, 0x24);
					DBG_PRINT("temp4 = %bd\r\n", temp);
					DBG_PRINT("while break;\r\n");
					
					twdDelay(1000);
					temp =IC_ReadByte(TWIC_P3, 0x24);
					DBG_PRINT("temp5 = %bd\r\n", temp);
					
					highlight_flag[1] = 5;  /*5 -> yello; 3-> white*/
					OSDShowMenu(1); /*update main page*/
					
					break; /*break frequency output*/
				}
#endif
				
			}

		}
		
		if(CD4017_EN_FLAG)//TW_TIMER_OUT_FLAG, TIMER1_OUT_FLAG && CD4017_EN_FLAG
		{
			//tw_timer_close();
			//TW_TIMER_OUT_FLAG = 0;
			
			GPB6.gpio_out_val = HIGH; /*CD4066 was switched on/off, High->off; LOW->on*/
			setgpio(GPB6, 6);
			DBG_PRINT("Line 9V was switched off\r\n");

			twdDelay(1000); 
		
			temp =IC_ReadByte(TWIC_P3, 0x24);
			DBG_PRINT("temp2 = %bd\r\n", temp); 
			
			while(1)
			{
				temp =IC_ReadByte(TWIC_P3, 0x24);
				DBG_PRINT("temp normal = %bd\r\n", temp); 
				
				GPC0.gpio_out_val = LOW;
				setgpio(GPC0, 0);
								
				for(i=0;i<50;i++)
				{
					twdDelay(100); /*10ms delay*/
				
					if(IC_ReadByte(TWIC_P3, 0x24)!=-1) /*if get the key of right and left, processing stop.*/
					{
						CD4017_EN_FLAG = 0;
						//twdDelay(1000); 
						temp =IC_ReadByte(TWIC_P3, 0x24);
						DBG_PRINT("temp a = %bd\r\n", temp); 
						//break;
					}
				}
			
				GPC0.gpio_out_val = HIGH;
				setgpio(GPC0, 0);

				for(i=0;i<50;i++)
				{
					twdDelay(100); /*10ms delay*/
				
					if(IC_ReadByte(TWIC_P3, 0x24)!=-1) /*if get the key of right and left, processing stop.*/
					{
						CD4017_EN_FLAG = 0;
						//twdDelay(1000); 
						temp =IC_ReadByte(TWIC_P3, 0x24);
						DBG_PRINT("temp b = %bd\r\n", temp); 
						//break;
					}
				}
				
				if(CD4017_EN_FLAG==0)
				{
					DBG_PRINT("Jump out while = %bd\r\n"); 
					highlight_flag[2] = 5;  /*5 -> yello; 3-> white*/
					OSDShowMenu(1); /*update main page*/
					break;
				}
			}
		
		}

		if(MenuIndex==7)
		{
			if(TEL0_FLAG)
			{
				DBG_PRINT("Line inject test start\r\n"); 
				GPC4.gpio_out_val = HIGH;
				setgpio(GPC4, 4);
				GPC3.gpio_out_val = LOW;
				setgpio(GPC3, 3);
				TEL0_FLAG = 0;
			}
			if(TEL1_FLAG)
			{
				DBG_PRINT("Line loop test start\r\n"); 
				GPC4.gpio_out_val = LOW;
				setgpio(GPC4, 4);
				GPC3.gpio_out_val = LOW;
				setgpio(GPC3, 3);
				TEL1_FLAG = 0;
			}
		}

		check_event();
		//get_command();
		events = get_event();
		rc = tw_page_process(events);
		//put_nodify(events, rc);

	}
}



