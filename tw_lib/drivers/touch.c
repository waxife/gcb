/**
 *  @file   touch.c
 *  @brief  touch panel drivers
 *  $Id: touch.c,v 1.17 2013/09/04 06:07:32 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.17 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "reg_tw.h"
#include "iccontrol.h"
#include "spirw.h"
#include "tw_irq.h"
#include "touch.h"
#include "spiosd.h"
#include "osd2api.h"
#include "tw_timer.h"

#define pen_down 	0x00;
#define pen_move 	0x01;
#define pen_release 0x02;
#define pen_noack   0x03;

#define _dfu_offset 16384L 

//#define sub(x,y) ((x)>(y)?((x)-(y)):((y)-(x)))
#define diff 	20	 
#define c_x 	100	// calibration x axis
#define c_y 	60// calibration y axis

static unsigned short txqueue[TOUCH_BUFF_SIZE]={0};
static unsigned short tyqueue[TOUCH_BUFF_SIZE]={0};
static unsigned char tfront=TOUCH_BUFF_SIZE-1;
static unsigned char trear=TOUCH_BUFF_SIZE-1;
static unsigned char tstatus[TOUCH_BUFF_SIZE]={0};

static unsigned short c_w=0x00;//panel width
static unsigned short c_h=0x00;//panel high
static unsigned short tp_left=0x00;
static unsigned short tp_right=0x00;
static unsigned short tp_up=0x00;
static unsigned short tp_down=0x00;
static bit calibration_finish=0;
static bit calibration_timeout=0;
unsigned short tdata[5];

unsigned short repeat_value=0x00;
unsigned short td_rep_thres= 200;

static unsigned short repeat_count=0x00;

unsigned char old_time=0x00;  // if 0= pen down and 1 =pen release else if 0= pen down and 1 = pen motion
tfilter t_fset={0,0,0,0};	 
tcoord  t_coord={0,0};	  

//add by yuke 
static unsigned char tp_mode = 0x00;
#define Calibration_MAX     512

unsigned char code HANSTER_TOUCH_INIT_TBL[]={
0X20,0X6F,
0X21,0X0F,
0X22,0X01,
0X2D,0X03,
0X2F,0X64,
0X31,0XFF,
0X40,0XFF,
0X41,0X4E,
0X50,0X09,
0X51,0X09,
0X52,0X05,
0X55,0X44,
0XFF,0X00,
};
/*
 * 摘要 :void WriteTouchTable(unsigned char code* RegsTable)
 *
 * 描述 :初始 touch panel 参数设定
 *
 * 参数 :unsigned char code* RegsTable  (touch panel 参数table)
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void WriteTouchTable(unsigned char code* RegsTable)
{
	unsigned short Index = 0x00;
	while(1)
	{
		if(RegsTable[Index] == 0xFF)
				break;
		IC_WritByte(TWIC_P3,RegsTable[Index],RegsTable[Index+1]);
		Index += 2;
	}
}
/*
 * 摘要 :unsigned short sub(unsigned short x,unsigned short y)
 *
 * 描述 :两数相减取绝对值
 *
 * 参数 :unsigned short x(第一个数字)
 *		 unsigned short y(第二个数字)
 *
 * 回传值 :相减结果
 *
 * 注意 : 无
*/
unsigned short sub(unsigned short x,unsigned short y)
{
	if(x>y)
		return x-y;
	else
		return y-x;
}

extern unsigned short code LAYOUT_TP_LEFT;
extern unsigned short code LAYOUT_TP_RIGHT;
extern unsigned short code LAYOUT_TP_UP;
extern unsigned short code LAYOUT_TP_DOWN;
extern unsigned char code LAYOUT_TP_FINISH;
/*
 * 摘要 :drawtarget(unsigned char select)
 *
 * 描述 :画校正图形
 *
 * 参数 :unsigned char select: select=0  预设值 select =1 表示强制重新校正
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void drawtarget(unsigned char select)
{
	unsigned char x=0;
	unsigned char y=0;
	unsigned char data temp, O_LSB, O_MSB=0;
	unsigned short save[5]={0,0,0,0,0};	
    int rc = 0;

#ifndef ENABLE_EX_MCU
	if (((LAYOUT_TP_FINISH&0x0F)==1) && (select==0))
	{
		tp_left= LAYOUT_TP_LEFT;
		tp_right= LAYOUT_TP_RIGHT;
		tp_up= LAYOUT_TP_UP;
		tp_down= LAYOUT_TP_DOWN;
		calibration_finish=1;
		tp_mode = LAYOUT_TP_FINISH>>4;
		dbg(0, (" tp_left = 0x%X \r\n", tp_left));
		dbg(0, (" tp_right = 0x%X \r\n", tp_right));
		dbg(0, (" tp_up = 0x%X \r\n", tp_up));
		dbg(0, (" tp_down = 0x%X \r\n", tp_down));	
		dbg(0, (" tp_mode = 0x%X \r\n", (int)tp_mode));
		return;
	} else {
		calibration_finish= 0;
	}
#else
	select= select;
#endif

	osd_disable();

	/* Load Main Color LUT */
	osd_cfg_wr(0x08, 0x00);	// OSD LUT address start point
	IC_WritByte(TWIC_P0, 0xA8, 0x09);

	IC_WritByte(TWIC_P0, 0xA9, 0xFF);	// A
	IC_WritByte(TWIC_P0, 0xA9, 0x00);	// B
	IC_WritByte(TWIC_P0, 0xA9, 0x00);	// G
	IC_WritByte(TWIC_P0, 0xA9, 0x00);	// R

	IC_WritByte(TWIC_P0, 0xA9, 0x00);	// A
	IC_WritByte(TWIC_P0, 0xA9, 0x00);	// B
	IC_WritByte(TWIC_P0, 0xA9, 0xFF);	// G
	IC_WritByte(TWIC_P0, 0xA9, 0x00);	// R

	/* Set BMP Color LUT Base Address */
	osd_cfg_wr(0x2C, 0x00);

	/* Set BMP Width */
	osd_cfg_wr(0x24, 0x0A);
	osd_cfg_wr(0x25, 0x00);
	/* Set BMP Height */
	osd_cfg_wr(0x26, 0x28);
	osd_cfg_wr(0x27, 0x00);
	/* Set BMP Horizintal Start Position */
	osd_cfg_wr(0x28, 0x15);
	osd_cfg_wr(0x29, 0x00);
	/* Set BMP Vertical Start Position */
	osd_cfg_wr(0x2A, 0x00);
	osd_cfg_wr(0x2B, 0x00);
	/* Set OSD RAM Base Address */
	osd_set_ram_addr(0x0000);
	/* Load BMP Index to OSDRAM */
	for(y=0; y<40; y++) {
		for(x=0; x<40; x++) {
			if(x==19||y==19)
				temp = 0x01;
			else
				temp = 0x00;
			
			switch((x%4)) {
				case 0:
					O_MSB=(temp<<4);
					break;
				case 1:
					O_MSB|=temp;
					break;
				case 2:
					O_LSB=(temp<<4);
					break;
				case 3:
					O_LSB|=temp;
					OSD_SET_RAM_DATA_BYTE(O_LSB);
					OSD_SET_RAM_DATA_BYTE(O_MSB);
					break;
			}
		}
	}
	/* Set BMP Data Address in OSDRAM */
	osd_cfg_wr(0x21, 0x00);
	osd_cfg_wr(0x22, 0x00);

	/* Set BMP Configuration */
	osd_cfg_wr(0x0B, 0x00);
	osd_cfg_wr(0x0E, 0x07);
	osd_cfg_wr(0x20, 0xC0);
		
	//osd_enable();

	//dbg(2, ("Calibration OSD Draw Final\r\n"));
}
/*
 * 摘要 :void touch_queue(unsigned short avgx,unsigned short avgy,unsigned char status)
 *
 * 描述 :使用queue buffer 储存touch panel资料
 *
 * 参数 :unsigned short avgx  (x座标)
 *       unsigned short avgy  (y座标)
 *       unsigned char status (状态)
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void touch_queue(unsigned short avgx,unsigned short avgy,unsigned char status)
{
	if((avgx!=0||avgy!=0)&&(status>=0)&&(status<=2))
	{
		trear=(trear+1)%TOUCH_BUFF_SIZE;
		if(trear==tfront)
		{
			#if 0
			if(trear==0)
			{
				trear=TOUCH_BUFF_SIZE-1;//max value
			}
			else
			{
				trear=trear-1;
			}
			#endif
			trear=TOUCH_BUFF_SIZE-1;
			trear=TOUCH_BUFF_SIZE-1;
			trear=(trear+1)%TOUCH_BUFF_SIZE;
			txqueue[trear]=avgx;
			tyqueue[trear]=avgy;
			tstatus[trear]=status;
		}
		else
		{
			txqueue[trear]=avgx;
			tyqueue[trear]=avgy;
			tstatus[trear]=status;
		}
	}
}
void touch_auto_switch(unsigned long *avgx, unsigned long *avgy, unsigned char tp_mode)
{
    unsigned short temp;
	
    switch(tp_mode)
	{			
	    case 0x01:
			*avgx=*avgx;
			*avgy=*avgy;	
			//DBG_PRINT(" doTouch_choice1\n");  
			break;
		case 0x02:
		    *avgx=Calibration_MAX-*avgx;
		    //DBG_PRINT(" doTouch_choice2\n");
		    break; 
	    case 0x03:
			*avgy=Calibration_MAX-*avgy;
			//DBG_PRINT(" doTouch_choice3\n"); 
			break;
		case 0x04:
			*avgx=Calibration_MAX-*avgx;
			*avgy=Calibration_MAX-*avgy;	
			//DBG_PRINT(" doTouch_choice4\n"); 
			break;
		case 0x05:
			temp=*avgx;
			*avgx=*avgy;
			*avgy=temp;
			//DBG_PRINT(" doTouch_choice5\n"); 
			break;
		case 0x06:
			temp=*avgx;
			*avgx=*avgy;
			*avgy=temp;
            *avgy=Calibration_MAX-*avgy;
			//DBG_PRINT(" doTouch_choice6\n"); 
			break;	
        case 0x07:
			temp=*avgx;
			*avgx=*avgy;
			*avgy=temp;
            *avgx=Calibration_MAX-*avgx;
			*avgy=Calibration_MAX-*avgy;						
			//DBG_PRINT(" doTouch_choice7\n"); 
			break;
		case 0x08:
			temp=*avgx;
			*avgx=*avgy;
			*avgy=temp;
            *avgx=Calibration_MAX-*avgx;						
			//DBG_PRINT(" doTouch_choice8\n"); 
			break;	
		default:
			break;
    }
}
/*
 * 摘要 :void doTouch(unsigned char choice)
 *
 * 描述 :执行 touch panel 取点动作
 *
 * 参数 :unsigned char choice => choice =0 (校正完毕后使用) choice = 1 (初始校正用) 
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void doTouch(unsigned char choice)
{
		unsigned long avgx=0;
		unsigned long avgy=0;
		unsigned char status;
		
        unsigned long temp=0;
	    #define Calibration_MAX     512
		
		if((IC_ReadByte_INT(TWIC_P3,SAR0_STATUS)&0x10)==0X10&&(IC_ReadByte_INT(TWIC_P3,SAR0_TRIG_STATUS)&0x40)!=0x40)//pendown
		{
				TP_TIME=0x00;
				status=pen_down;
				
		}
		else if(IC_ReadByte_INT(TWIC_P3,SAR0_STATUS)&0x20)//release
		{
				status=pen_release;
		}
		else if((IC_ReadByte_INT(TWIC_P3,SAR0_STATUS)&0x10)==0X10&&(IC_ReadByte_INT(TWIC_P3,SAR0_TRIG_STATUS)&0x40)==0x40)//move
		{
				status=pen_move;
		}
			
		avgx=IC_ReadByte_INT(TWIC_P3,SAR0_VALUE_LOWXY)&0x0f;//read avgx [7:4] 0xb04065d0
		avgx=(IC_ReadByte_INT(TWIC_P3,SAR0_VALUE_HIGHX)<<4)|avgx;//read avgx [11:4] 0xb04065e0
		avgy=IC_ReadByte_INT(TWIC_P3,SAR0_VALUE_LOWXY)>>4;//read avgy [3:0] 0xb04065d0
		avgy=(IC_ReadByte_INT(TWIC_P3,SAR0_VALUE_HIGHY)<<4)|avgy;//read avgy avgx[11:4] 0xb04065f0
		avgx=avgx>>3;
		avgy=avgy>>3;
		switch (choice)
		{
			case 0:
				touch_queue(avgx,avgy,status);
				break;			
			case 1:
            touch_auto_switch(&avgx, &avgy, tp_mode);
			if((avgx!=0) || (avgy!=0))//(((avgx>=tp_right)&&(avgx<=tp_left))&&((avgy>=tp_down)&&(avgy<=tp_up)))
			{
				if(t_fset.enable)
				{
					avgx=c_w-(((avgx-tp_right)*c_w)/(tp_left-tp_right));
					avgy=c_h-((avgy-tp_down)*c_h)/(tp_up-tp_down);
					if(status==0)//pen down
					{
						t_coord.x=avgx;
						t_coord.y=avgy;
						t_fset.type=0x01;
					}
					else if(status==2) //pen release
					{
						if(t_fset.type==0x01&&TP_TIME>=(t_fset.time))
						{
							touch_queue(t_coord.x,t_coord.y,0);	
						}
						else if(t_fset.type==0x01)
						{
							t_fset.type=2;
							return;
						}	
						touch_queue(avgx,avgy,status);
						t_fset.type=2;
					}
					else if(status==1)// pen motion
					{
						#if 0
						if(t_fset.type==0x01)
						{
							touch_queue(t_coord.x,t_coord.y,0);	
						}
						touch_queue(avgx,avgy,status);
						t_fset.type=2;
						#endif//#else
						if(t_fset.type==0x01&&TP_TIME<t_fset.time)
						{
							return;
						}
						else if(t_fset.type==0x01)
						{
							touch_queue(t_coord.x,t_coord.y,0);
						}
						t_fset.type=2;
						switch(t_fset.speed)
						{
							case 0:
								touch_queue(avgx,avgy,status);
								break;
							case 1:
								if(old_time!=TP_TIME)
								{
									touch_queue(avgx,avgy,status);
									old_time=TP_TIME;
								}
								break;
							case 2:
								if(TP_TIME%2==0)
								{
									touch_queue(avgx,avgy,status);	
								}
								break;
							case 3:
								if(TP_TIME%3==0)
								{
									touch_queue(avgx,avgy,status);	
								}
								break;
						}
						//#endif
					}
				}
				else
				{                                								  				      
					avgx=c_w-(((avgx-tp_right)*c_w)/(tp_left-tp_right));
					avgy=c_h-((avgy-tp_down)*c_h)/(tp_up-tp_down);

					
					if(status!=2)
					{
						t_coord.x=avgx;
						t_coord.y=avgy;
					}
					else if(status==2)
					{
						avgx=t_coord.x;
						avgy=t_coord.y;	
					}
					touch_queue(avgx,avgy,status);
				}
			}
			else
			{
				return;
			}
			break;
		}	
}
/*
 * 摘要 :bit calibration (unsigned short int *x,unsigned short int *y)
 *
 * 描述 :根据传过来的点 执行校正判断
 *
 * 参数 :unsigned char choice => choice =0 (校正完毕后使用) choice = 1 (初始校正用) 
 *
 * 回传值 : 1:成功 0:失败
 *
 * 注意 : 无
*/
bit calibration (unsigned short int *x,unsigned short int *y)
{
	unsigned short int xa,xb,ya,yc;
	unsigned char tempx=0x00;
	unsigned char tempy=0x00;
    //unsigned short temp1=0x00;
			 
	xa=c_x;
	xb=c_w-c_x;
	ya=c_h-c_y;
	yc=c_y;
	tempx= sub(sub(x[0],x[1]),sub(x[2],x[3]));
	tempy= sub(sub(y[0],y[1]),sub(y[2],y[3]));
	//DBG_PRINT("tempx=%d tempy=%d",(int)tempx,(int)tempy);
	if(tempx<=0x05&&tempy<=0x05)
	{
	    if(sub(x[0],x[1])>200)
	    {
	        if((x[0]>x[1])&&(y[0]>y[2]))
	        {
			    tp_mode=0x01;
			    //DBG_PRINT("bit calibration1\n");
		    }
		    else if((x[0]<x[1])&&(y[0]>y[2]))
		    {
	            x[0]=512-x[0];
			    x[1]=512-x[1];
			    x[2]=512-x[2];
			    x[3]=512-x[3];
			    tp_mode=0x02;
			    //DBG_PRINT("bit calibration2\n");
		    }
		    else if((x[0]>x[1])&&(y[0]<y[2]))
	        { 
	            y[0]=512-y[0];
			    y[1]=512-y[1];
			    y[2]=512-y[2];
			    y[3]=512-y[3];
			    tp_mode=0x03;
			    //DBG_PRINT("bit calibration3\n");
		    }
		    else if((x[0]<x[1])&&(y[0]<y[2]))
		    {
	            x[0]=512-x[0];
			    x[1]=512-x[1];
			    x[2]=512-x[2];
			    x[3]=512-x[3];
	            y[0]=512-y[0];
			    y[1]=512-y[1];
			    y[2]=512-y[2];
			    y[3]=512-y[3];	    
			    tp_mode=0x04;
			    //DBG_PRINT("bit calibration4\n");
		    }
		}
        else if(sub(y[0],y[1])>200)
        {	 
            unsigned short temp1=0x00;			 
            if((y[0]>y[1])&&(x[0]>x[2]))
            { 
            #if 1
                temp1=x[0];  /* x0,y0交换位置  */
		        x[0]=y[0];
		        y[0]=temp1;
		        temp1=x[1]; /* x1,y1交换位置  */
		        x[1]=y[1];
		        y[1]=temp1;
		        temp1=x[2]; /* x2,y2交换位置  */
		        x[2]=y[2];
		        y[2]=temp1;
		        temp1=x[3];
		        x[3]=y[3];
		        y[3]=temp1;   /* x3,y3相互交换位置*/
           #endif
		        tp_mode=0x05;
	        //DBG_PRINT("bit calibration5\n");
	        }
            else  if((y[0]>y[1])&&(x[0]<x[2]))
            {
            #if 1
                temp1=x[0];  /* x0,y0交换位置  */
		        x[0]=y[0];
		        y[0]=temp1;
		        temp1=x[1]; /* x1,y1交换位置  */
		        x[1]=y[1];
		        y[1]=temp1;
		        temp1=x[2]; /* x2,y2交换位置  */
		        x[2]=y[2];
		        y[2]=temp1;
		        temp1=x[3];
		        x[3]=y[3];
		        y[3]=temp1;   /* x3,y3相互交换位置*/
            #endif
		        y[0]=512-y[0];  /*y0,y1,y2,y3取补 */
		        y[1]=512-y[1];
		        y[2]=512-y[2];
		        y[3]=512-y[3];
                tp_mode=0x06;
	        //DBG_PRINT("bit calibration6\n");
	        }			 
	        else  if((y[0]<y[1])&&(x[0]<x[2]))
            {	
            #if 1
                temp1=x[0];  /* x0,y0交换位置  */
		        x[0]=y[0];
		        y[0]=temp1;
		        temp1=x[1];   /* x1,y1交换位置  */
		        x[1]=y[1];
		        y[1]=temp1;
		        temp1=x[2]; /* x2,y2交换位置  */
		        x[2]=y[2];
		        y[2]=temp1;
		        temp1=x[3];
		        x[3]=y[3];
		        y[3]=temp1;    /* x3,y3相互交换位置*/
            #endif
		        x[0]=512-x[0];  /*x0,x1,x2,x3取补 */
		        x[1]=512-x[1];
		        x[2]=512-x[2];
		        x[3]=512-x[3];

		        y[0]=512-y[0];  /*y0,y1,y2,y3取补 */
		        y[1]=512-y[1];
		        y[2]=512-y[2];
		        y[3]=512-y[3];
                tp_mode=0x07;
	            //DBG_PRINT("bit calibration7\n");		
	        }
	        else  if((y[0]<y[1])&&(x[0]>x[2]))
            {	
            #if 1
                temp1=x[0];   /* x0,y0交换位置  */
		        x[0]=y[0];
		        y[0]=temp1;

		        temp1=x[1];   /* x1,y1交换位置  */
		        x[1]=y[1];
		        y[1]=temp1;

		        temp1=x[2];  /* x2,y2交换位置  */
		        x[2]=y[2];
		        y[2]=temp1;

		        temp1=x[3];
		        x[3]=y[3];
		        y[3]=temp1;   /* x3,y3相互交换位置*/
            #endif
		        x[0]=512-x[0];  /*x0,x1,x2,x3取补 */
		        x[1]=512-x[1];
		        x[2]=512-x[2];
		        x[3]=512-x[3];
		        tp_mode=0x08;
	            //DBG_PRINT("bit calibration8\n");
	        }    
	    }
		x[0]=(x[0]+x[2])/2;
		x[1]=(x[1]+x[3])/2;
		y[0]=(y[0]+y[1])/2;
		y[2]=(y[2]+y[3])/2;
		tp_left=x[0]+((x[0]-x[1])*c_x)/(xb-xa);
		tp_right=x[1]-((x[0]-x[1])*c_x)/(xb-xa);		
		tp_up=y[0]+((y[0]-y[2])*c_y)/(ya-yc);
		tp_down=y[2]-((y[0]-y[2])*c_y)/(ya-yc);
		dbg(0, (" tp_mode = 0x%X \r\n", (int)tp_mode));
		return 1;
	}
	else
	{
		return 0;
	}
}
/*
 * 摘要 :void do_calibration(unsigned char select)
 *
 * 描述 :进行校正
 *
 * 参数 :unsigned char select => select =0 表示若校正完毕 不会在校正一次 select =1 若已经校正完毕 可以强制再校正一次
 *
 * 回传值 : 
 *
 * 注意 : 无
*/
void do_calibration(unsigned char select)
{
	bit rc=0;
	unsigned short x[4]={0};
	unsigned short y[4]={0};
	unsigned char count=0x00;
	unsigned char i=0x00;
	unsigned short x_l,x_r,y_u,y_d;
	static unsigned short re_x;
	static unsigned short re_y;
	touchd co_value={0,0,0};

	c_w=IC_ReadByte(TWIC_P0,0xb5)<<8|IC_ReadByte(TWIC_P0,0xb4);//get panel width
	c_h=IC_ReadByte(TWIC_P0,0xb7)<<8|IC_ReadByte(TWIC_P0,0xb6);//get panel heigh

	drawtarget(select);
	
	x_l=c_x;	//200
	x_r=c_w-c_x;	 //600
	y_u=c_y;		//120
	y_d=c_h-c_y;	//360
	//DBG_PRINT("x_l=%d x_r=%d y_u=%d y_d=%d c_w=%d c_h=%d",(int)x_l,(int)x_r,(int)y_u,(int)y_d,(int)c_w,(int)c_h);
	if(calibration_finish==1)
		return;
	
	tw_timer_open(2000);

#ifdef OSD_SELECT_SPIOSD
	sosd_disable();
	_sosd_pLUT_addr((unsigned long)TOUCH_BASE_ADDRESS);
	_sosd_img_addr((unsigned long)TOUCH_BASE_DATA);
	_sosd_img_window(0x29,0x29);
	_sosd_img_ls(0x29);
	_sosd_img_loca(x_l-diff,y_u-diff);//first point  180 100
	_sosd_load_plut();
	sosd_enable(LOAD_LUT);
#else
	/* Set BMP H/V Start Position */
	_osd_bmp_pos((x_l-diff)+0x15, (y_u-diff));
	osd_enable();
#endif
	
	while(count<=4)
	{
		co_value=touch_read();
		if(TW_TIMER_OUT_FLAG) {
			tw_timer_close();
#ifdef OSD_SELECT_SPIOSD
			sosd_disable();
#else
			osd_disable();
			osd_bmp_disable();
#endif
			return;
		}
		if(((co_value.x!=0) || (co_value.y!=0)) && (co_value.status==0))
		{
			count++;
			switch(count)
			{
				case 1:
					x[0]=co_value.x;
					y[0]=co_value.y;
					re_x=co_value.x;
					re_y=co_value.y;
#ifdef OSD_SELECT_SPIOSD
					sosd_set_img_location(x_r-diff,y_u-diff);//for second points 
#else					
					osd_bmp_star_position((x_r-diff)+0x15,y_u-diff);//for second points 
#endif
					//DBG_PRINT("first point x=%d y=%d\r\n",(int)x[0],(int)y[0]);
					break;
				case 2:
					if((sub(co_value.x,re_x)>=50)||(sub(co_value.y,re_y)>=50))
					{
						x[1]=co_value.x;
						y[1]=co_value.y;
						re_x=co_value.x;
						re_y=co_value.y;
#ifdef OSD_SELECT_SPIOSD
						sosd_set_img_location(x_l-diff,y_d-diff);//for third points
#else
						osd_bmp_star_position((x_l-diff)+0x15,y_d-diff);//for third points
#endif
						//DBG_PRINT("second point x=%d y=%d\r\n",(int)x[1],(int)y[1]);
					}
					else
					{
						count=1;
						//DBG_PRINT("back=%d\r\n",count);
					}
					break;
				case 3:
					if((sub(co_value.x,re_x)>=50)||(sub(co_value.y,re_y)>=50))
					{
						x[2]=co_value.x;
						y[2]=co_value.y;
						re_x=co_value.x;
						re_y=co_value.y;
#ifdef OSD_SELECT_SPIOSD
						sosd_set_img_location(x_r-diff,y_d-diff);//for fourth points
#else
						osd_bmp_star_position((x_r-diff)+0x15,y_d-diff);//for fourth points
#endif
						//DBG_PRINT("third point x=%d y=%d\r\n",(int)x[2],(int)y[2]);
					}
					else
					{
						count=2;
					//	DBG_PRINT("back=%d\r\n",count);
					}
					break;
				case 4:
					if((sub(co_value.x,re_x)>=50)||(sub(co_value.y,re_y)>=50))
					{
						x[3]=co_value.x;
						y[3]=co_value.y;
						//DBG_PRINT("fourth point x=%d y=%d\r\n",(int)x[3],(int)y[3]);
						rc=calibration(x,y);
						if(rc!=0)
						{
							//DBG_PRINT("successful\r\n");
							//DBG_PRINT("tp_left=%d tp_right=%d tp_up=%d tp_down=%d\r\n",(int)tp_left,(int)tp_right,(int)tp_up,(int)tp_down);
							count=5;
							//IC_WritByte(TWIC_P3, 0x9E,0x00);
							tw_timer_close();
#ifdef OSD_SELECT_SPIOSD
							sosd_disable();
#else
							osd_disable();
							osd_bmp_disable();
#endif
							//spi_dma_write(TOUCH_BASE_SAVE,(unsigned short)save,sizeof(save));
							calibration_finish=1;
#ifndef ENABLE_EX_MCU
							if(0x00 == select)
								TouchSave();
#endif
						}
						else
						{
							//DBG_PRINT("try again\r\n");
							count=0;
#ifdef OSD_SELECT_SPIOSD							
							sosd_set_img_location(x_l-diff,y_u-diff);//first point
#else
							osd_bmp_star_position((x_l-diff)+0x15,(y_u-diff));//first point
#endif
						}
						re_x=co_value.x;
						re_y=co_value.y;
					}
					else
					{
						count=3;
						//DBG_PRINT("back=%d\r\n",count);
					}
					break;
			}
		}
	}
	tw_timer_close();

#ifdef OSD_SELECT_SPIOSD
	sosd_disable();
#else
	osd_disable();
	osd_bmp_disable();
#endif
}
/*
 * 摘要 :touchd touch_read(void)
 *
 * 描述 :进行touch panel 资料读取
 *
 * 参数 :unsigned char select => select =0 表示若校正完毕 不会在校正一次 select =1 若已经校正完毕 可以强制再校正一次
 *
 * 回传值 : touch=> 回传 touch 座标 状态 结构体
 *
 * 注意 : 无
*/
touchd touch_read(void)
{
	touchd result={0,0,0};//Initial 
	if(tfront==trear)
	{
		result.x=0;
		result.y=0;
		result.status=pen_noack;
		return result;
	}
	else
	{
		tfront=(tfront+1)%TOUCH_BUFF_SIZE;
		result.x=txqueue[tfront];
		result.y=tyqueue[tfront];
		result.status=tstatus[tfront];
		return result;
	}
}
/*
 * 摘要 :touch_clear(void)
 *
 * 描述 :清除touch buffer 资料
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void touch_clear(void)
{
	tfront=TOUCH_BUFF_SIZE-1;
	trear=TOUCH_BUFF_SIZE-1;
}
/*
 * 摘要 :touch_isr(void)
 *
 * 描述 :touch panel 中断处理程序
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void touch_isr(void)
{
	if(calibration_finish==0)
		doTouch(0);
	else
		doTouch(1);
	repeat_count=0x00;
	clear_tw_irq(IRQ_SAR0_TOGGLE);
}
/*
 * 摘要 :touch_init(void)
 *
 * 描述 :touch panel 初使化设定
 *
 * 参数 :无
 *
 * 回传值 : 无
 *
 * 注意 : 无
*/
void touch_init(void)
{
#if 0
	IC_WritByte(TWIC_P3,SAR0_THRESHOLD_LOW,0x0f);//sar 0 threshold 0xb0406410
	
	IC_WritByte(TWIC_P3,SAR0_THRESHOLD_HIGH,0x01);//sar 0 threshold 0xb0406420
	//IC_WritByte(TWIC_P3,0x00,0xb0406480);//sar 0 select 00 p6_48 0xb0406480
	IC_WritByte(TWIC_P6,SAR0_INTERRUPT,0xf1);//clear sar interrupt and unmask sar 0 interrupt 0xb0400e70
	IC_WritByte(TWIC_P3,SAR0_INITIAL_SETTING,0x2f);//power up sar 0 0xb0406400//default 0xcd	
	IC_WritByte(TWIC_P3,SAR0_TRIG_STATUS,0x03);//enable touch panel pendown trig and move trig 0xb04064d0
	IC_WritByte(TWIC_P3,SAR0_TOUCH_CLOCK,0x24);//touch panel clock //default 0xc4
	IC_WritByte(TWIC_P3,SAR0_BOUN_THREHX,0x09);//bounce threshold x//default 0x0a
	IC_WritByte(TWIC_P3,SAR0_BOUN_THREHY,0x09);//bounce threshold y//default 0x0a 
	 
	IC_WritByte(TWIC_P3,SAR0_TIMES_XY,0x05);//x y value times count//default 0x05
	IC_WritByte(TWIC_P3,SAR0_BOUN_THREH0,0xf2);//bounce for pendown penrelease
	
	//setup measure gap parameter
	IC_WritByte(TWIC_P3,SAR0_TPMEASURE_GAP,0xff);//tp_measure_gap 0xb0406510
	IC_WritByte(TWIC_P3,SAR0_PDDMEASURE_GAP,0xff);//pdd_measure_gap 0xb0406600
	IC_WritByte(TWIC_P3,SAR0_MEASURE_GAP,0xff);//measure_gap 0xb0406610

	enable_tw_irq(IRQ_SAR0_TOGGLE);

	touch_clear();
#endif

#if 0
	IC_WritByte(TWIC_P3,SAR0_THRESHOLD_LOW,0x0f);//sar 0 threshold 0xb0406410
	
	IC_WritByte(TWIC_P3,SAR0_THRESHOLD_HIGH,0x01);//sar 0 threshold 0xb0406420
	IC_WritByte(TWIC_P6,SAR0_INTERRUPT,0xf1);//clear sar interrupt and unmask sar 0 interrupt 0xb0400e70
	IC_WritByte(TWIC_P3,SAR0_INITIAL_SETTING,0x4F);//power up sar 0 0xb0406400//default 0xcd	
	IC_WritByte(TWIC_P3,SAR0_TRIG_STATUS,0x03);//enable touch panel pendown trig and move trig 0xb04064d0
	IC_WritByte(TWIC_P3,SAR0_TOUCH_CLOCK,0x24);//touch panel clock //default 0xc4
	IC_WritByte(TWIC_P3,SAR0_BOUN_THREHX,0x09);//bounce threshold x//default 0x0a
	IC_WritByte(TWIC_P3,SAR0_BOUN_THREHY,0x09);//bounce threshold y//default 0x0a 
	 
	IC_WritByte(TWIC_P3,SAR0_TIMES_XY,0x0A);//x y value times count//default 0x05
	IC_WritByte(TWIC_P3,SAR0_BOUN_THREH0,0xF3);//bounce for pendown penrelease
	
	//setup measure gap parameter
	IC_WritByte(TWIC_P3,SAR0_TPMEASURE_GAP,0xff);//tp_measure_gap 0xb0406510
	IC_WritByte(TWIC_P3,SAR0_PDDMEASURE_GAP,0xff);//pdd_measure_gap 0xb0406600
	IC_WritByte(TWIC_P3,SAR0_MEASURE_GAP,0xff);//measure_gap 0xb0406610
#endif

	WriteTouchTable(HANSTER_TOUCH_INIT_TBL);

	enable_tw_irq(IRQ_SAR0_TOGGLE);

	touch_clear();
	
}

#if 0
void TouchSet(unsigned short _left,unsigned short _right,unsigned short _up,unsigned short _down,unsigned char _calibration_finish)
{
	tp_left = _left;
	tp_right = _right;
	tp_up = _up;
	tp_down = _down;
	calibration_finish = _calibration_finish;
}
#endif
/*
 * 摘要 :touchd check_repeat(unsigned short threshold)
 *
 * 描述 :确认touch panel是否为repeat状态 并且发出event通知
 *
 * 参数 :unsigned short threshold =>多少repeat次数后 算为一次repeat
 *
 * 回传值 : touch 结构体
 *
 * 注意 : 无
*/
touchd check_repeat(unsigned short threshold)
{
	touchd result={0,0,0};//Initial 
	if((IC_ReadByte(TWIC_P3,SAR0_TRIG_STATUS)&0x80>0)&&(IC_ReadByte(TWIC_P3,SAR0_STATUS)!=0x04))
	{
		repeat_count++;
		if(repeat_value==0xFFFF)
			repeat_value=0x00;
		else
			repeat_value++;
		if((t_fset.type==0x01)&&(TP_TIME>t_fset.time))//satisfy 
		{
			t_fset.type=2;//meet the condition
			//touch_queue(t_coord.x,t_coord.y,0);
			trear=(trear+1)%TOUCH_BUFF_SIZE;
			txqueue[trear]=t_coord.x;
			tyqueue[trear]=t_coord.y;
			tstatus[trear]=0;
		}
		else if(repeat_count>=threshold)
		{
			repeat_count=0x00;
			result.x=txqueue[tfront];
			result.y=tyqueue[tfront];;
		}
	}
	return result;
}
/*
 * 摘要 :void TouchSave(void)
 *
 * 描述 :touch panel 校正存档
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
#ifndef ENABLE_EX_MCU
void TouchSave(void)
{
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
	spi_wrsr(0x00,QUAD_ENABLE_WIN);
#elif (defined(GD_128M_QUAD_NOR)||defined(WQ_25Q_128_QUAD_NOR))
	spi_wrsr(0x00,WS_REG0_GD128M);
#else
	spi_wrsr(QUAD_ENABLE,0x00);
#endif
	spi_dma_write(LAYOUT_TP_LEFT_ADDR     +_dfu_offset, (unsigned short)(&tp_left), 2);
	spi_dma_write(LAYOUT_TP_RIGHT_ADDR    +_dfu_offset, (unsigned short)(&tp_right), 2);
	spi_dma_write(LAYOUT_TP_UP_ADDR       +_dfu_offset, (unsigned short)(&tp_up), 2);
	spi_dma_write(LAYOUT_TP_DOWN_ADDR     +_dfu_offset, (unsigned short)(&tp_down), 2);
	spi_write_byte(LAYOUT_TP_FINISH_ADDR  +_dfu_offset, (tp_mode<<4)|0x01);
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
	spi_wrsr(BLOCK_PROTECT,QUAD_ENABLE_WIN);
#elif (defined(GD_128M_QUAD_NOR)||defined(WQ_25Q_128_QUAD_NOR))
	spi_wrsr(BLOCK_PROTECT,RS_REG0_GD);
#else
	spi_wrsr(QUAD_ENABLE|BLOCK_PROTECT,0x00);
#endif

	dbg(3, (" tp_left = 0x%X \r\n", tp_left));
	dbg(3, (" tp_right = 0x%X \r\n", tp_right));
	dbg(3, (" tp_up = 0x%X \r\n", tp_up));
	dbg(3, (" tp_down = 0x%X \r\n", tp_down));
	dbg(0, (" layout_tp_finish = 0x%X \r\n", (int)LAYOUT_TP_FINISH));
	dbg(0, ("tp_mode=0x%X \r\n",(int)tp_mode));
}
#endif
/*
 * 摘要 :void touch_filter(unsigned char enable,unsigned char time,unsigned char speed)
 *
 * 描述 :设定touch panel filter 取样次数
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void touch_filter(unsigned char enable,unsigned char time,unsigned char speed)
{
	t_fset.enable=enable;
	t_fset.time=time;
	t_fset.speed=speed;
	tw_timer_open(100);		
}
/*
 * 摘要 :void touch_covalue(unsigned short *left,unsigned short *right,unsigned short *up,unsigned short *down)
 *
 * 描述 :读取touch panel 校正后的参数
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void touch_covalue(unsigned short *left,unsigned short *right,unsigned short *up,unsigned short *down)
{
	*left=tp_left;
	*right=tp_right;
	*up=tp_up;
	*down=tp_down;
}
/*
 * 摘要 :touch_coset(unsigned short left,unsigned short right,unsigned short up,unsigned short down)
 *
 * 描述 :储存touch panel 校正后的参数
 *
 * 参数 :unsigned short left  (左边参数)
 *       unsigned short right (右边参数)
 *       unsigned short up    (上边参数)
 *       unsigned short down  (左边参数)
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void touch_coset(unsigned short left,unsigned short right,unsigned short up,unsigned short down)
{
	tp_left=left;
	tp_right=right;
	tp_up=up;
	tp_down=down;
	calibration_finish=1;
}