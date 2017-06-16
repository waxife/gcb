/**
 *  @file   videodetect.c
 *  @brief  Video Detect Driver
 *  $Id: videodetect.c,v 1.13 2013/08/27 02:15:40 onejoe Exp $
 *  $Author: onejoe $
 *  $Revision: 1.13 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include <reg51.h>
#include <stdlib.h>

#include "sys.h"
#include "iccontrol.h"
#include "reg_tw.h"
#include "panel.h"
#include "videodetect.h" 
#include "cq.h"
#include "video.h"
#ifdef CCIR656_DETECT
#include "ccir656detect.h"
#endif

#ifdef VGA_DETECT
    #include "VGAdetect.h"
#else
    extern void Sync_Mode_Detect(void);
#endif

#define VD_Status_P2 			0x3a
#define VSYNC_PeriodH_P0		0x5b
#define HSYNC_PeriodH_P0        0x59
#define VSYNC_PeriodL_P0		0x5a

#define BULE_BACKGROUND	  	  0x1D, 0xF0, 0x6C
#define BLACK_BACKGROUND  0x10, 0xf0, 0x80
#define BACKGROUND  	BLACK_BACKGROUND

unsigned char Signalset_times = 0;
SigState sigstate;
unsigned short code *VIDEODET_CVD_TAB;
unsigned short code *VIDEODET_YUV_TAB;
unsigned short code *VIDEODET_VGA_TAB;
unsigned short code *VIDEODET_VIP_TAB;//数字信号
static unsigned char ASigStd;
static unsigned char DSigStd;
static unsigned char VSigStd;
static unsigned char Pb;
static unsigned char Pr;
static unsigned char YPbPrBalance;
static unsigned char AUTO_DETECT=1;

/*
 * 摘要 :static unsigned short GetVTotal(unsigned char mode)
 *
 * 描述 :取得vsync total lines
 *
 * 参数 :unsigned char mode =>　mode =0 直接从　5a 5b计算, mode = 1 根据 vsync total xclk * 32 /hsync xclk计算
 *
 * 回传值 : 回传所计算的　1 vsync total lines
 *
 * 注意 : 无
*/
static unsigned short GetVTotal(unsigned char mode)
{
	unsigned short wTemp=1000;
	unsigned short wpreTemp=0;
	unsigned short VSyncCount=0;
	unsigned long dwVSyncCount=0; 
	unsigned short HSyncCount=0;
	int i;

    if(mode == 0)
    {
	    wTemp=0;
	    for(i=0; i<10; i++)
	    {	 
		    twdDelay(50);
			wTemp = IC_ReadByte(TWIC_P0,0x5B);
		    wTemp<<=8;
	        wTemp|= IC_ReadByte(TWIC_P0,0x5A);
		    if((wTemp<=wpreTemp+1)&&(wTemp>=wpreTemp-1))
		    {
			    VSyncCount++;
			    if(VSyncCount>=5)
				    break;
		    }
		    else
		    {
			    wpreTemp = wTemp;
			    VSyncCount = 0;
		    }
	    }
	    return wpreTemp;
    }
	else
	{
		wTemp = IC_ReadByte(TWIC_P0,0x50);
	    IC_WritByte(TWIC_P0, 0x50, wTemp | 0x50);//Enable VSYNC counter & Hsync period
	    wTemp = 50;
	    while(wTemp--)
	    {
		    twdDelay(30);
		     //V sync counter is done exit
		    if(IC_ReadByte(TWIC_P0,0x50)&0x20)  //V sync counter is done exit
			break;
	    }
		if (wTemp == 0)
			return 0;
	
		wTemp = IC_ReadByte(TWIC_P0,0x50);
	    wTemp &= 0xCF;			//Disable Auto Position
	    IC_WritByte(TWIC_P0, 0x50, wTemp);
		dwVSyncCount =  IC_ReadByte(TWIC_P0,0x53);
	    dwVSyncCount <<= 8;
		dwVSyncCount |= IC_ReadByte(TWIC_P0,0x52);
	    dwVSyncCount <<= 8;
		dwVSyncCount |= IC_ReadByte(TWIC_P0,0x51);
	    wTemp=0;
	
	    for(i=0; i<10; i++)
	    {	
			twdDelay(50);
			HSyncCount = IC_ReadByte(TWIC_P0,0x59);
		    HSyncCount <<= 8;
			HSyncCount += IC_ReadByte(TWIC_P0,0x58);
		    if((wTemp&0xFF00)==((HSyncCount)&0xFF00))
			    break;
		    else
			    wTemp = HSyncCount;
	    }
	    wTemp = dwVSyncCount * 32  / (HSyncCount);
	    //dbg(("YPbPr Active signal std is 0x%bx!", (unsigned short)wTemp) );
	    return wTemp;
	}
} 

/*
 * 摘要 :unsigned char DataInRage(unsigned short sData, unsigned short tData, unsigned char rData)
 *
 * 描述 :确认是否在范围内,一般用来计算vsync hsync跟标准讯号比较,是否在误差内
 *
 * 参数 :unsigned short sData (比较的参数一) 计算所得
 *       unsigned short tData (比较的参数二) 标准数字
 *       unsigned char rData  (所允许的误差值)
 *
 * 回传值 :　1: 在范围内　0:在范围外
 *
 * 注意 : 无
*/
unsigned char DataInRage(unsigned short sData, unsigned short tData, unsigned char rData)
{
	if((sData<(tData+rData))&&(sData>(tData-rData)))
		return 1;
	else
		return 0;
}

/*
 * 摘要 :void  GainSignal(void)
 *
 * 描述 :影像讯号处理程序
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void  GainSignal(void)
{	
    unsigned short regTEMP;
	unsigned char temp_freerun=0;
	unsigned char temp_mode=0;
	unsigned char temp=0;
	unsigned char temp_status=0;
	unsigned char temp_syncchange=0;
	unsigned char tmpSync;

	//DBG_PRINT("\r  GAIN SIGNAL\n\r");
    temp_freerun = IC_ReadByte(TWIC_P0,0x50)&0x04;
	if (sigstate.SigSrc == Src_NONE)
		return;
	if (sigstate.SigSrc == Src_CVD)
	{
	
	    /*use freerun(P2_50[2] ) and P2_3A[0] to check signal loss */
		if((IC_ReadByte(TWIC_P2,VD_Status_P2)&0x07 !=0x06)) 		
		{
			sigstate.SigActive = Signal_Lost;	
			sigstate.DeteSignalType =sigLoss;
			sigstate.DeteSignalTimes =0;
	
			//DBG_PRINT("\rvd status=%bx   temp_freerun=%bx\n\r",IC_ReadByte(TWIC_P2,VD_Status_P2)&0x07,temp_freerun);
			/*Blabk screen or open panel => close blue panel */
			//DBG_PRINT("\r temp_freerun close panel BLACK 1!!!\n\r");
		    if((IC_ReadByte(TWIC_P0,0x91)&0x80) )
	        {
				 sigstate.SigStd=CVBS_UNKNOW;
	    	     /*blue screen*/
				 ClosePanel(BLACK_BACKGROUND);
				 //DBG_PRINT("\r  close panel BLUE  !!!\n\r");
			}
			return;
	    }		
		

#ifdef TwoSignalSystemMode
		/* CVD HAVE SINGAL */
		regTEMP = IC_ReadByte(TWIC_P0, VSYNC_PeriodH_P0);
		regTEMP<<=8;
		regTEMP |= IC_ReadByte(TWIC_P0, VSYNC_PeriodL_P0);
		if(regTEMP > 0x180)
		regTEMP /=2;
		
		if(DataInRage(regTEMP, 0x0107, 7))	//ntsc	    
		    temp =NTSC_M;
		else if(DataInRage(regTEMP, 0x0139, 7)) //pal		
			temp =PAL_B; 
	
	    /*check standard stable*/
	    if((sigstate.DeteSignalTimes ==0) || (sigstate.DeteSignalType1 == temp))
	    {		
	    	if (temp == PAL_B)
				twdDelay(10);	
		    sigstate.DeteSignalType1 = temp;
			sigstate.DeteSignalTimes++;
	
			if(sigstate.DeteSignalTimes>12 )
			{
				sigstate.DeteSignalType = temp;
	            sigstate.DeteSignalTimes=0;
#if 1
				if(sigstate.DeteSignalType!=sigstate.SigStd)
				{
				DBG_PRINT ("\r State.DeteSignalType = 0x%bx\n", sigstate.DeteSignalType);
				DBG_PRINT ("\r OK State.DeteSignalTimes = 0x%bx\n", sigstate.DeteSignalTimes);
				}
#endif
			}	
	    }
		else
		{
		    sigstate.DeteSignalTimes=0;
		}
		
	    if(sigstate.DeteSignalType != sigstate.SigStd)
	    {
	        #if 0
	        temp_status = IC_ReadByte(TWIC_P0, 0x50);
			DBG_PRINT ("\r P0_50 = 0x%bx\n", temp_status);
	        temp_status = IC_ReadByte(TWIC_P2, 0x41);
			DBG_PRINT ("\r P2_41 = 0x%bx\n", temp_status);
			temp_status = IC_ReadByte(TWIC_P0, 0x5A);
			DBG_PRINT ("\r P0_5A = 0x%bx\n", temp_status);
	        temp_status = IC_ReadByte(TWIC_P0, 0x5B);
			DBG_PRINT ("\r P0_5B = 0x%bx\n", temp_status);
			#endif
	
			if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))
	        {
			    /*black screen*/
				ClosePanel(BLACK_BACKGROUND);
				//DBG_PRINT("\r standard change close panel BLACK 3!!!\n\r");
			}
	    }
#else
		regTEMP = IC_ReadByte(TWIC_P2,0x41)&0x07;

		switch(regTEMP)
	    {
			case 0x00:
				     temp =NTSC_M;
					 break;
					 
			case 0x01:
					 temp =PAL_B;
					 break;
	
			case 0x02:
					 temp =PAL_M;
					 break;
	
			case 0x03:
					 temp =PAL_N;
					 break;
	
			case 0x04:
					 temp =SECAM;
					 break;

			case 0x05:
					 temp =NTSC_443;
					 break;
					 
			default: 
					temp =sigLoss;
					break;
		}	
		
		/*check standard stable*/
	    if((sigstate.DeteSignalTimes ==0) || (sigstate.DeteSignalType1 == temp))
	    {		
	    	if (temp == PAL_B)
				twdDelay(100);	
		    sigstate.DeteSignalType1 = temp;
			sigstate.DeteSignalTimes++;
			twdDelay(100);
			if(sigstate.DeteSignalTimes>12 )
			{
				sigstate.DeteSignalType = temp;
	            sigstate.DeteSignalTimes=0;
#if 1
				if(sigstate.DeteSignalType!=sigstate.SigStd)
				{
				DBG_PRINT ("\r State.DeteSignalType = 0x%bx\n", sigstate.DeteSignalType);
				DBG_PRINT ("\r OK State.DeteSignalTimes = 0x%bx\n", sigstate.DeteSignalTimes);
				}
#endif
			}	
	    }
		else
		{
		    sigstate.DeteSignalTimes=0;
		}
		
	        
#endif //ManySignalSystemMode
		if(temp != sigstate.SigStd)
				{
	        #if 0
					temp_status = IC_ReadByte(TWIC_P0, 0x50);
					DBG_PRINT ("\r P0_50 = 0x%bx\n", temp_status);
					temp_status = IC_ReadByte(TWIC_P2, 0x41);
					DBG_PRINT ("\r P2_41 = 0x%bx\n", temp_status);
					temp_status = IC_ReadByte(TWIC_P0, 0x5A);
					DBG_PRINT ("\r P0_5A = 0x%bx\n", temp_status);
					temp_status = IC_ReadByte(TWIC_P0, 0x5B);
					DBG_PRINT ("\r P0_5B = 0x%bx\n", temp_status);
			#endif
			
					if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))
					{
						/*black screen*/
						ClosePanel(BLACK_BACKGROUND);
						//DBG_PRINT("\r standard change close panel BLACK 3!!!\n\r");
					}
				}

		return;
	}	

	else if (sigstate.SigSrc == Src_YUV)
	{
		temp=IC_ReadByte(TWIC_P6,0x12)&0x0F;
		IC_WritByte(TWIC_P6,0x12,0x3F);
		/* NO SINGAL */
		if( (temp&0x03)==0x03 ||  (temp&0x22)==0x02 )
    	{
    	    if(sigstate.SigActive != Signal_Lost)
    	    {
    	        /*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	        	{
					 sigstate.SigStd=YUV_UNKNOW;
			    	/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
		    	}
    	    }
    		sigstate.SigActive = Signal_Lost; 
    	}
		/* HAVE SINGLE */
		else
		{
			
    	    if(sigstate.SigActive == Signal_Set)
    	    {
    	        tmpSync = IC_ReadByte(TWIC_P6,0x12)&0x0F;
    			IC_WritByte(TWIC_P6,0x12,0x3F); 
    			tmpSync = tmpSync&0x0C;
    
    			/*Sync change*/
    			if(tmpSync == 0x0C)
    			{
    			    /*Close Panel*/ 
					if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))
	        		{
			    		/*black screen*/
						ClosePanel(BLACK_BACKGROUND);
						//DBG_PRINT("\r temp_freerun close panel BLACK 1!!!\n\r");
		    		}
    			    
    			}
				else if((tmpSync&0x03)==0x03)
				{
					if(sigstate.SigActive != Signal_Lost)
    	    		{
    	        		/*Close Panel*/ 
						if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	        			{
			    			/*blue screen*/
							ClosePanel(BLACK_BACKGROUND);
							//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
		    			}
    	   			 }
    				sigstate.SigActive = Signal_Lost; 
				}
    			else
    			{
    			    /*Open Panel*/ 
    				if((IC_ReadByte(TWIC_P0,0x91)&0x80))
    			    {   
    			        if((sigstate.SigStd!=CVBS_UNKNOW)  && (sigstate.SigStd!=YUV_UNKNOW))
    			        {
    	                    OpenPanel();	 
    			        }
    					
    			    }
    			}
    	    }
    
    		if(sigstate.SigActive == Signal_Lost)
    		{
#ifdef YPBPR_COLOR_CALIBRATION						
				if (YPbPrBalance == 0)
				{
					YPbPr_ADC_Auto_offset_reset();
					YPbPr_ADC_Auto_offset();
					twdDelay(2000);
					Pb=IC_ReadByte(TWIC_P0, adc_blue_offset);
					Pr=IC_ReadByte(TWIC_P0, adc_red_offset);
					YPbPrBalance=1;
					sigstate.YUV_CALIBRATION = YPbPrBalance;
				//	VideoSave();
				}	
#endif
    		    sigstate.SigActive = Signal_Active;
    			/*Close Panel*/
    			if(!(IC_ReadByte(TWIC_P0,0x91)&0x80) || (IC_ReadByte(TWIC_P0,0x9D) == 0x10))
	            {
	            	/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
			    }
    			
    		}
    	}
			
	}
#ifdef VGA_DETECT	
   	else if (sigstate.SigSrc == Src_VGA)
	{
	//DBG_PRINT("\r 1.sigstate.SigActive = 0x%d\n\r",(int)sigstate.SigActive);
		//twdDelay(500);
		
		temp=IC_ReadByte(TWIC_P6,0x12)&0x0F;
		IC_WritByte(TWIC_P6,0x12,0x3F);
		/* NO SINGAL */
		if( (temp&0x03)==0x03 ||  (temp&0x22)==0x02 )
    	{
    		Sync_Mode_Detect();
    	    if(sigstate.SigActive != Signal_Lost)
    	    {
    	        /*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	        	{
					 sigstate.SigStd=VGA_UNKNOW;
			    	/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
		    	}
    	    }
    		sigstate.SigActive = Signal_Lost; 
    	}
		/* HAVE SINGLE */
		else
		{	
    	    if(sigstate.SigActive == Signal_Set)
    	    {
    	    	//twdDelay(500);
#if 0				
    	        tmpSync = IC_ReadByte(TWIC_P6,0x12)&0x0F;
				//DBG_PRINT("\r -->tmpSync = 0x%x\n\r",(int)tmpSync);
    			IC_WritByte(TWIC_P6,0x12,0x3F); 
    			tmpSync = tmpSync&0x0C;
#endif    
    			/*Sync change*/
    			if((temp && 0x08) || (temp && 0x04))
    			{
    			    /*Close Panel*/ 
					if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))
	        		{
			    		/*black screen*/
						ClosePanel(BLACK_BACKGROUND);
						//DBG_PRINT("\r temp_freerun close panel BLACK 1!!!\n\r");
		    		}
    			    sigstate.SigActive = Signal_Lost; 
    			}
#if 0				
				else if((tmpSync&0x03)==0x03)
				{
					if(sigstate.SigActive != Signal_Lost)
    	    		{
    	        		/*Close Panel*/ 
						if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	        			{
			    			/*blue screen*/
							ClosePanel(0x1D, 0xF0, 0x6C);
							//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
		    			}
    	   			 }
    				sigstate.SigActive = Signal_Lost; 
				}
#endif
    			else
    			{
    			    /*Open Panel*/ 
    				if((IC_ReadByte(TWIC_P0,0x91)&0x80))
    			    {   
    			        if((sigstate.SigStd!=CVBS_UNKNOW)  && (sigstate.SigStd!=YUV_UNKNOW))
    			        {
    	                    OpenPanel();	 
    			        }
    					
    			    }
    			}
    	    }
    		if(sigstate.SigActive == Signal_Lost)
    		{
    		    sigstate.SigActive = Signal_Active;
				DBG_PRINT("1.SET Signal_Active\n");
    			/*Close Panel*/
    			if(!(IC_ReadByte(TWIC_P0,0x91)&0x80) || (IC_ReadByte(TWIC_P0,0x9D) == 0x10))
	            {
	            	/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
			    }
    			
    		}
    	}
			
	}
#endif
}
/*
 * 摘要 :static unsigned char YPbPr_modedetect(unsigned short Hsync_period, unsigned short Vsync_period)	
 *
 * 描述 :ypbpr讯号匹配
 *
 * 参数 :unsigned short Hsync_period : hsync total dots
 *　	 unsigned short Vsync_period : vysnc total lines
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
static unsigned char YPbPr_modedetect(unsigned short Hsync_period, unsigned short Vsync_period)														
{
    unsigned char ypbprmode=0;

	if(DataInRage(Hsync_period,0xd688,0x20)&&DataInRage(Vsync_period,0x110,0x10))	
		ypbprmode=YUV_480I;
    else if(DataInRage(Hsync_period,0x6B42,0x20)&&DataInRage(Vsync_period,0x20d,0x10))
	    ypbprmode=YUV_480P;
	else if(DataInRage(Hsync_period,0xd808,0x20)&&DataInRage(Vsync_period,0x140,0x10))
		ypbprmode=YUV_576I;
	else if(DataInRage(Hsync_period,0x6c00,0x20)&&DataInRage(Vsync_period,0x271,0x10))
		ypbprmode=YUV_576P;
	else if(DataInRage(Hsync_period,0x5a01,0x20)&&DataInRage(Vsync_period,0x2f3,0x10))
		ypbprmode=YUV_720P_50;	
	else if(DataInRage(Hsync_period,0x4b14,0x20)&&DataInRage(Vsync_period,0x2f3,0x10))
		ypbprmode=YUV_720P_60;
	else if(DataInRage(Hsync_period,0x7802,0x20)&&DataInRage(Vsync_period,0x241,0x10))
		ypbprmode=YUV_1080I_50;
	else if(DataInRage(Hsync_period,0x6418,0x20)&&DataInRage(Vsync_period,0x241,0x10))	
		ypbprmode=YUV_1080I_60;
	else
		ypbprmode= YUV_UNKNOW;

	return ypbprmode;
}
#ifdef VGA_DETECT
/*
 * 摘要 :static unsigned char VGA_modedetect(unsigned short Hsync_period, unsigned short Vsync_period)	
 *
 * 描述 :vga讯号匹配
 *
 * 参数 :unsigned short Hsync_period : hsync total dots
 *　	 unsigned short Vsync_period : vysnc total lines
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
static unsigned char VGA_modedetect(unsigned short Hsync_period, unsigned short Vsync_period)														
{
    unsigned char vgamode=0;
       if(DataInRage(Hsync_period,0x6b3d,0x20)&&DataInRage(Vsync_period,0x20c,0x10))	
		vgamode=VGA_720_480;
	else if(DataInRage(Hsync_period,0x6bfe,0x20)&&DataInRage(Vsync_period,0x270,0x10))	
		vgamode=VGA_720_576;
	else if(DataInRage(Hsync_period,0x2908,0x20)&&DataInRage(Vsync_period,0x464,0x10))	
	{	vgamode=VGA_1920_1080;
		dbg(10, (">>>>> Main Menu page VGA\n\r"));}	
	else
		vgamode= VGA_UNKNOW;

	return vgamode;

}
#endif
/*
 * 摘要 :void SignalProcess()	
 *
 * 描述 :影像讯号处理程序
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void SignalProcess()
{
	unsigned short tmpData[2];
	unsigned char tmpStd=0; 
	unsigned char temp=0;

	if(sigstate.SigSrc == Src_NONE)
		return;
	//DBG_PRINT ("\rSIGNAL PROCESS\n");
	if (sigstate.SigSrc == Src_CVD)
	{

		if((sigstate.SigStd ==sigstate.DeteSignalType))	
		{
			return; 
		}
		sigstate.SigStd =sigstate.DeteSignalType;
		ASigStd = sigstate.SigStd;
		//registry_write((unsigned long)&UserData, sizeof(UserData));
		//VideoSave();
		if (sigstate.SigStd != sigLoss)
		{
			if(sigstate.SigStd != CVBS_UNKNOW)
			{
				//std=VIDEODET_CVD_TAB[sigstate.SigStd];
				WriteRegsTable((unsigned char*)VIDEODET_CVD_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
				WriteRegsTable((unsigned char*)VIDEODET_CVD_TAB[sigstate.SigStd+6], CQ_TRIGGER_VSYNC);
				//DBG_PRINT("2.SIGNAL ACT=%bd	SRC=%bd	STD=%bd	type=%bd\r\n",sigstate.SigActive,sigstate.SigSrc,sigstate.SigStd,sigstate.DeteSignalType);
				DBG_PRINT("\r\nWrite CVBS TABLE__OK!!!\n\r");	
//				vde_soft_reset();
//				OpenPanel();
//				twdDelay(3000);
			}
		}
		else
		{
		#if 1
			DBG_PRINT("\rWrite not Signal!!!\n\r");	
			if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))
	        {
				 sigstate.SigStd=CVBS_UNKNOW;
				ClosePanel(BLACK_BACKGROUND);
				//DBG_PRINT("\r  close panel BLUE 2 !!!\n\r");
			}
		#endif
		}
		
	}
	
	else if (sigstate.SigSrc == Src_YUV)
	{
		if(sigstate.SigActive == Signal_Active)
    	{
			 /* READ VSYNC PERIOD */
    		tmpData[1] = GetVTotal(0);
    		
    		twdDelay(800);
    		/* READ HSYNC PERIOD */
    		tmpData[0] = IC_ReadByte(TWIC_P0,0x59);
    		tmpData[0] <<=8;
    		tmpData[0] |= IC_ReadByte(TWIC_P0,0x58);
                
    		tmpStd=YPbPr_modedetect(tmpData[0], tmpData[1]);
    		DBG_PRINT("YUV Active signal Htotal 0x%x  Vtotal 0x%x std is 0x%bx \n", (unsigned short)tmpData[0], (unsigned short)tmpData[1], (unsigned char)tmpStd);

		  if( tmpStd != 0)
    		{
    		    if( (sigstate.SigStd != tmpStd) || (sigstate.SigActive != Signal_Set))
    		    {
    		        /*Close Panel*/
    			    if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))	
                    {  
                        /*black screen*/
						ClosePanel(BLACK_BACKGROUND);
						//DBG_PRINT("\r temp_freerun close panel BLACK 1!!!\n\r");
            	    }
    				
    		        if(sigstate.SigSrc == Src_YUV)
    		        {
    		            sigstate.SigStd = tmpStd;
    					if(sigstate.SigStd != YUV_UNKNOW)
    					{
    						WriteRegsTable((unsigned char*)VIDEODET_YUV_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
							WriteRegsTable((unsigned char*)VIDEODET_YUV_TAB[sigstate.SigStd+8], CQ_TRIGGER_VSYNC);		
							DBG_PRINT ("\r YUV load table Active signal std is 0x%bx\n",tmpStd);
    						sigstate.SigActive = Signal_Set;
							DSigStd = sigstate.SigStd;
							//registry_write((unsigned long)&UserData, sizeof(UserData));
					//		VideoSave();
							OpenPanel();
							twdDelay(3000);	
    					}
						else
						{
							sigstate.SigActive = Signal_Lost;
							ClosePanel(BLACK_BACKGROUND);
							return;
						}
    					
    		        }
    		    }
    		}
		  
		}
		else if(sigstate.SigActive == Signal_Set)
        {
        	temp=IC_ReadByte(TWIC_P6,0x12)&0x0F;
			IC_WritByte(TWIC_P6,0x12,0x3F);
			/* SINGAL CHANGE */
			if( (temp&0x08)==0x08 || (temp&0x04)==0x04)
    		{
    			/*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	    	    {
				/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
			    }
    			sigstate.SigActive = Signal_Lost; 
    		}
			/* NO SINGAL */
			if( (temp&0x03)==0x03 ||  (temp&0x22)==0x02 )
    		{
    		    /*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	    	    {
				   	/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
			    }
    			sigstate.SigActive = Signal_Lost; 
    		}
#if 0
			//twdDelay(800);
			/* READ HSYNC PERIOD */
			tmpData[0] = IC_ReadByte(TWIC_P0,0x59);
			tmpData[0] <<=8;
			tmpData[0] |= IC_ReadByte(TWIC_P0,0x58);
				
			/* READ VSYNC PERIOD */
			tmpData[1] =(Signalset_times==5)? GetVTotal(1):GetVTotal(0);
			tmpStd=YPbPr_modedetect(tmpData[0], tmpData[1]);	
			if( Signalset_times == 5 )	
				Signalset_times=0;
			else
				Signalset_times++; 

			if(sigstate.SigStd != tmpStd)
  			{
      			sigstate.SigActive = Signal_Active;
   				DBG_PRINT("2. SET Signal_Active\n");
  				/*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))
	        		{
			    		/*black screen*/
						ClosePanel(0x10, 0x80, 0x80);
						//DBG_PRINT("\r temp_freerun close panel BLACK 1!!!\n\r");
		    		}
  			}									
#endif
		}
	}
#ifdef VGA_DETECT	
	else if (sigstate.SigSrc == Src_VGA)
    {
    	//DBG_PRINT("\r sigstate.SigActive = 0x%d\n\r",(int)sigstate.SigActive);
		if(sigstate.SigActive == Signal_Active)
    	{        
#ifdef AUTO_CAL_OUT_TIMING
			if (SrcVGAProcess(&tmpStd, 0))
			{
				sigstate.SigActive = Signal_Set;
				sigstate.SigStd = tmpStd;
				OpenPanel();
			}
#else
			/* READ VSYNC PERIOD */
		   tmpData[1] = GetVTotal(0);
					   
		   twdDelay(800);
		   /* READ HSYNC PERIOD */
		   tmpData[0] = IC_ReadByte(TWIC_P0,0x59);
		   tmpData[0] <<=8;
		   tmpData[0] |= IC_ReadByte(TWIC_P0,0x58);

			tmpStd=VGA_modedetect(tmpData[0], tmpData[1]);
			/* */ 		
			if( tmpStd != 0)
    		{
    		    if( (sigstate.SigStd != tmpStd) || (sigstate.SigActive != Signal_Set))
    		    {
    		        /*Close Panel*/
    			    if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))	
                    {  
                       
				ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
				//DBG_PRINT("\r temp_freerun close panel BLACK 1!!!\n\r");
            	    }	
   		        if(sigstate.SigSrc == Src_VGA)
    		        {
    		            sigstate.SigStd = tmpStd;
    					if(sigstate.SigStd != VGA_UNKNOW)
    					{
							//ClosePanel(0x10, 0x80, 0x80);
							//ClosePanel(0x1d, 0xf0, 0x80);
    						   WriteRegsTable((unsigned char*)VIDEODET_VGA_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
							WriteRegsTable((unsigned char*)VIDEODET_VGA_TAB[sigstate.SigStd+3], CQ_TRIGGER_VSYNC);		
							DBG_PRINT ("\r VGA load table Active signal std is 0x%bx\n",tmpStd);
    						      sigstate.SigActive = Signal_Set;
							VSigStd = sigstate.SigStd;
							#if 0
							RGBAutoADCGainOffset();
							#endif 
							//registry_write((unsigned long)&UserData, sizeof(UserData));
							//VideoSave();
                                                twdDelay(50);
                                               OpenPanel();
							DBG_PRINT("\r Write_VGATable !!!\n\r");
							 //  twdDelay(50);
							
							
    					}
						else
						{
							sigstate.SigActive = Signal_Lost;
							ClosePanel(BLACK_BACKGROUND);
							return;
						}					
    		        }
    		    }
    		}	
#endif
		 }
	 	else if(sigstate.SigActive == Signal_Set)
        {
        	temp=IC_ReadByte(TWIC_P6,0x12)&0x0F;
			IC_WritByte(TWIC_P6,0x12,0x3F);
			/* SINGAL CHANGE */
			if( (temp&0x08)==0x08 || (temp&0x04)==0x04)
    		{
    			/*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	    	    {
				 	/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
			    }
    			sigstate.SigActive = Signal_Lost; 
    		}
			/* NO SINGAL */
			if( (temp&0x03)==0x03 ||  (temp&0x22)==0x02 )
    		{
    			/*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80)||(IC_ReadByte(TWIC_P0, FRAME_CRED_CFG_REG) == 0x10))
	    	    {
				   	/*blue screen*/
					ClosePanel(BLACK_BACKGROUND);
					//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
			    }
    			sigstate.SigActive = Signal_Lost; 
    		}	
#ifndef AUTO_CAL_OUT_TIMING
			//twdDelay(800);
			/* READ HSYNC PERIOD */
			tmpData[0] = IC_ReadByte(TWIC_P0,0x59);
			tmpData[0] <<=8;
			tmpData[0] |= IC_ReadByte(TWIC_P0,0x58);
				
			/* READ VSYNC PERIOD */
			tmpData[1] =(Signalset_times==5)? GetVTotal(1):GetVTotal(0);
			tmpStd=VGA_modedetect(tmpData[0], tmpData[1]);	

			
			if( Signalset_times == 5 )	
				Signalset_times=0;
			else
				Signalset_times++; 
			if(sigstate.SigStd != tmpStd)
  			{
      			sigstate.SigActive = Signal_Active;
  				/*Close Panel*/ 
				if(!(IC_ReadByte(TWIC_P0,0x91)&0x80))
	        		{
			    		/*black screen*/
						ClosePanel(BLACK_BACKGROUND);
		    		}
  			}
#endif
		} 
	}	
#endif	
}	

/*
 * 摘要 :void SignalProcess()	
 *
 * 描述 :影像讯号处理自动侦测
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void SingalDetect(void)
{	
	if(AUTO_DETECT)
	{
        GainSignal();
        SignalProcess();
	}
}
/*
 * 摘要 :void InitStateVal(void)
 *
 * 描述 :初始化sigstate　structure
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void InitStateVal(void)
{
	sigstate.SigActive = Signal_Lost;	
	sigstate.DeteSignalType =CVBS_UNKNOW;
	sigstate.DeteSignalType1 =CVBS_UNKNOW;
    sigstate.DeteSignalTimes =0;
	sigstate.YUV_CALIBRATION =0;
	
	sigstate.VGA_CALIBRATION =0;
	sigstate.VGA_CALIBRATION_Gain_R=0x80;
	sigstate.VGA_CALIBRATION_Gain_G=0x80;
	sigstate.VGA_CALIBRATION_Gain_B=0x80;
	sigstate.VGA_CALIBRATION_Offset_R=0x80;
	sigstate.VGA_CALIBRATION_Offset_G=0x80;
	sigstate.VGA_CALIBRATION_Offset_B=0x80;
}
/*
 * 摘要 :void YUVInit(void)
 *
 * 描述 :YUV初始化
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void YUVInit(void)
{
	dbg(0, ("DSigStd = 0x%bx\n\r",DSigStd));
	dbg(0, ("ASigStd = 0x%bx\n\r",ASigStd));
	sigstate.SigSrc = Src_YUV;
	sigstate.SigStd=DSigStd;
	
	WriteRegsTable((unsigned char*)VIDEODET_YUV_TAB[0], CQ_TRIGGER_VSYNC);
	if ((sigstate.YUV_CALIBRATION == 0) && (YPbPrBalance == 1))
	{
		IC_WritByte(TWIC_P0, adc_blue_offset,Pb);
		IC_WritByte(TWIC_P0, adc_red_offset,Pr);
		sigstate.YUV_CALIBRATION = 1;
	}
	if(sigstate.SigStd != YUV_UNKNOW)
	{
		WriteRegsTable((unsigned char*)VIDEODET_YUV_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
		WriteRegsTable((unsigned char*)VIDEODET_YUV_TAB[sigstate.SigStd+8], CQ_TRIGGER_VSYNC); 	
		sigstate.SigActive = Signal_Set;
		//OpenPanel();
		twdDelay(3000); 
	}
	else
	{
		if((IC_ReadByte(TWIC_P0,0x91)&0x80) )
		{
			/*blue screen*/
			ClosePanel(BLACK_BACKGROUND);
			//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
		}
	}
	IC_WritByte(TWIC_P0,0x3a,0x03);
	IC_WritByte(TWIC_P0,0x3b,0x03);					
	IC_WritByte(TWIC_P6,0x12,0x3F); 

}

/*
 * 摘要 :void  VGA_Auto_xiaozhen_init(void)
 *
 * 描述 :VGA AUTO初始化
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/

void  VGA_Auto_xiaozhen_init(void)
{
     
   	IC_WritByte(TWIC_P0, 0x07, sigstate.VGA_CALIBRATION_Gain_R);
	IC_WritByte(TWIC_P0, 0x08, sigstate.VGA_CALIBRATION_Gain_G);
	IC_WritByte(TWIC_P0, 0x09, sigstate.VGA_CALIBRATION_Gain_B);
	IC_WritByte(TWIC_P0, 0x0a, sigstate.VGA_CALIBRATION_Offset_R);
	IC_WritByte(TWIC_P0, 0x0b, sigstate.VGA_CALIBRATION_Offset_G);
	IC_WritByte(TWIC_P0, 0x0c, sigstate.VGA_CALIBRATION_Offset_B);
	
}
/*
 * 摘要 :void VGAInit(void)
 *
 * 描述 :VGA 初始化
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/

void VGAInit(void)
{
	dbg(0, ("DSigStd = 0x%bx\n\r",DSigStd));
	dbg(0, ("ASigStd = 0x%bx\n\r",ASigStd));
	sigstate.SigSrc = Src_VGA;
	sigstate.SigStd=VSigStd;//DSigStd;// 1
	//sigstate.SigActive = Signal_Set;


       /*VGA ×O?ˉD￡Oy?COa*/
#if 0
	   if (sigstate.VGA_CALIBRATION ==0)
      	{
      	      VGA_Auto_xiaozhen_init();
		DBG_PRINT("\r  VGAinit_CALIBATION!\n\r");  
      	
      	}
#endif  
	 

	
	
	#if 1 

	WriteRegsTable((unsigned char*)VIDEODET_VGA_TAB[0], CQ_TRIGGER_VSYNC);
	//if ((sigstate.DeteSignalType !=sigLoss) || (sigstate.SigStd !=VGA_UNKNOW))
	if(sigstate.SigStd != VGA_UNKNOW)
	{
		//WriteRegsTable((unsigned char*)VIDEODET_VGA_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
		WriteRegsTable((unsigned char*)VIDEODET_VGA_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
		WriteRegsTable((unsigned char*)VIDEODET_VGA_TAB[sigstate.SigStd+1], CQ_TRIGGER_VSYNC); 	
		sigstate.SigActive = Signal_Set;
	
		twdDelay(100);  //3000
	     DBG_PRINT("\r  sigstate.SigStd != VGA_UNKNOW  !!!\n\r");
	}
	else
	{
		if((IC_ReadByte(TWIC_P0,0x91)&0x80) )
		{
			/*blue screen*/
			//ClosePanel(0x1D, 0xF0, 0x6C);	
				//ClosePanel(0x10, 0x80, 0x80);
					sigstate.SigActive = Signal_Set;
			DBG_PRINT("\r  VGAinit_close panel BLack  !!!\n\r");
		}
	}
	#endif
	IC_WritByte(TWIC_P0,0x3a,0x03);
	IC_WritByte(TWIC_P0,0x3b,0x03);
	IC_WritByte(TWIC_P6,0x12,0x3F); 
	//Sync_Mode_Detect();
} 
/*
 * 摘要 :void CVDInit(void)
 *
 * 描述 :CVD 初始化
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void CVDInit(void)
{
	sigstate.SigSrc = Src_CVD;
	sigstate.SigStd=ASigStd;
	dbg(2, ("data=%x %x\n",VIDEODET_CVD_TAB[0],VIDEODET_CVD_TAB[1]));
	DBG_PRINT("VIDEODET_CVD_TAB[0] = %bd,VIDEODET_CVD_TAB[1]= %bd \n\r", VIDEODET_CVD_TAB[0],VIDEODET_CVD_TAB[1]);
	WriteRegsTable((unsigned char*)VIDEODET_CVD_TAB[0], CQ_TRIGGER_VSYNC);
	twdDelay(300);	
	
#ifdef	TwoSignalSystemMode				
	if (sigstate.SigStd == CVBS_UNKNOW)
		sigstate.SigStd = PAL_B;
#endif
	if ((sigstate.DeteSignalType !=sigLoss) || (sigstate.SigStd !=CVBS_UNKNOW))
	{
		WriteRegsTable((unsigned char*)VIDEODET_CVD_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
		WriteRegsTable((unsigned char*)VIDEODET_CVD_TAB[sigstate.SigStd+6], CQ_TRIGGER_VSYNC); 			
		twdDelay(3000);
		DBG_PRINT("\r  aaaaaaaaaaaa  !!!\n\r");
	}	
	else
	{
		DBG_PRINT("\r  bbbbbbbbbbbb  !!!\n\r");
		if((IC_ReadByte(TWIC_P0,0x91)&0x80) )
		{
			/*blue screen*/
			ClosePanel(BLACK_BACKGROUND);
			DBG_PRINT("\r  close panel BLUE  !!!\n\r");
			
		}
		WriteRegsTable((unsigned char*)VIDEODET_CVD_TAB[sigstate.SigStd], CQ_TRIGGER_VSYNC);
		WriteRegsTable((unsigned char*)VIDEODET_CVD_TAB[sigstate.SigStd+6], CQ_TRIGGER_VSYNC); 			
		twdDelay(3000);
	}

}

/*
 * 摘要 :void VIPInit(unsigned char std)
 *
 * 描述 :数字信号初始化
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void VIPInit(unsigned char std)
{
	sigstate.SigSrc = Src_VIP;//数字信号
	sigstate.SigStd = std;//默认为ccir656
	ClosePanel(BACKGROUND);
	IC_WritByte(TWIC_P0, 0xC2, 0x12);
	IC_WritByte(TWIC_P1, 0x3A, 0x00);
	WriteRegsTable((unsigned char*)VIDEODET_VIP_TAB[0], CQ_TRIGGER_SW);
	if(sigstate.SigStd != VIP_UNKNOWN)
	{
		WriteRegsTable((unsigned char*)VIDEODET_VIP_TAB[sigstate.SigStd], CQ_TRIGGER_SW);
		WriteRegsTable((unsigned char*)VIDEODET_VIP_TAB[sigstate.SigStd+6], CQ_TRIGGER_SW); 	
		
		sigstate.SigActive = Signal_Active;
		//OpenPanel();
		//twdDelay(2000); 
	}
	else
	{
		if((IC_ReadByte(TWIC_P0,0x91)&0x80) )
		{
			/*blue screen*/
			ClosePanel(BACKGROUND);
			//DBG_PRINT("\r  close panel BLUE  !!!\n\r");
		}
	}
	//if(nvp6124_difinition == NVP6124_VI_1080P_2530)
		//WriteRegsTable(ccir656_1080p_patch, CQ_TRIGGER_SW);
	//IC_WritByte(TWIC_P1,0x3B,0x44);
	IC_WritByte(TWIC_P0,0x3a,0x03);
	IC_WritByte(TWIC_P0,0x3b,0x03);					
	IC_WritByte(TWIC_P6,0x12,0x3F); 	
    IC_WritByte(TWIC_P0, 0xC2, 0x00);
	OpenPanel();
	auto_detection_ctrl(DISABLE);
}

/* auto detection init */
/*
 * 摘要 :void auto_detection_init(unsigned short code *IML_CVBS_TAB,unsigned short code *IML_YUV_TAB,unsigned short code *IML_VGA_TAB)
 *
 * 描述 :AUTO DETECTION 初始化,并载入预设的TABLE
 *
 * 参数 :unsigned short code *IML_CVBS_TAB  =>cvbs预设tablre
 *       unsigned short code *IML_YUV_TAB   =>yuv预设table
 *       unsigned short code *IML_VGA_TAB   =>vga预设table
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
#if 1
void auto_detection_init(unsigned short code *IML_CVBS_TAB,unsigned short code *IML_YUV_TAB,unsigned short code *IML_VGA_TAB)
{
	unsigned char i=0;
	VIDEODET_YUV_TAB= IML_YUV_TAB;
	VIDEODET_CVD_TAB= IML_CVBS_TAB;
	VIDEODET_VGA_TAB= IML_VGA_TAB;
	VIDEODET_VIP_TAB=&IML0_VIP_TAB;//数字信号输入
	dbg(2, ("YUV=%d CVD=%d\n VGA=%d\n",(int)sizeof(IML_YUV_TAB[0]),(int)sizeof(IML_CVBS_TAB[0]),(int)sizeof(IML_VGA_TAB[0])));
}
#endif
/*
 * 摘要 :void auto_detection_ctrl(unsigned char en)
 *
 * 描述 :是否允许AUTO DETECTION
 *
 * 参数 :unsigned char en　=> 1= 使能　0=关闭
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void auto_detection_ctrl(unsigned char en)
{
	if (en)
	{
		AUTO_DETECT=1;
	}
	else
	{
		AUTO_DETECT=0;
		sigstate.SigSrc = Src_NONE;
	}
}
/*
 * 摘要 :bit YPbPr_ADC_Auto_offset(void)
 *
 * 描述 :YPBPR ADC AUTO OFFSET 自动校正ADC
 *
 * 参数 :无
 *
 * 回传值 :　1=完成　0=失败
 *
 * 注意 : 无
*/
#ifdef YPBPR_COLOR_CALIBRATION
bit YPbPr_ADC_Auto_offset(void)
{
	bit rc=0;
	IC_WritByte(TWIC_P0, adc_red_offset, 0x60);//adc red offset
	IC_WritByte(TWIC_P0, adc_blue_offset, 0X60);//adc blue offset
	
	IC_WritByte(TWIC_P0,boundary_control,IC_ReadByte(TWIC_P0,boundary_control)|0X80);//boundary detect enable
	
	IC_WritByte(TWIC_P1, color_range_probe, 0XE1);//check range enable
	IC_WritByte(TWIC_P1, color_probe_x_low, 0X0D);//probe x low byte 
	IC_WritByte(TWIC_P1, color_porbe_x_high, 0X00);//probe x high byte
	
	IC_WritByte(TWIC_P1, color_probe_y_low, 0X78);//probe y low byte
	IC_WritByte(TWIC_P1, color_porbe_y_high, 0X00);//probe y high byte

	IC_WritByte(TWIC_P1, color_probe_pixel_count, 0X10);//probe pixel count number

	IC_WritByte(TWIC_P3, auto_offset_low_boundary,  0X60);//setting low boundary
	IC_WritByte(TWIC_P3, auto_offset_high_boundary, 0XFF);//setting high boundary
	IC_WritByte(TWIC_P3, auto_offset_threshold, 0X10);//setting threshold
	
	IC_WritByte(TWIC_P3, auto_offset_setting, 0X08);//polarity setting
	IC_WritByte(TWIC_P3, auto_offset_setting, 0X09);//enable auto offset
	if(IC_ReadByte(TWIC_P3,auto_offset_setting)&0x80)//check the status
		rc=1;//done
	else
		rc=0;//fail
	return rc;
}
/*
 * 摘要 :bit YPbPr_ADC_Auto_offset(void)
 *
 * 描述 :YPBPR ADC AUTO RESET 自动校正ADC重置
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void YPbPr_ADC_Auto_offset_reset(void)
{
	if(IC_ReadByte(TWIC_P3,auto_offset_setting)&0x80);//check status if done 
	{
		IC_WritByte(TWIC_P3,auto_offset_setting,IC_ReadByte(TWIC_P3,auto_offset_setting)&~0x01);//first clear bit 1
		IC_WritByte(TWIC_P3,auto_offset_setting,0X82);//second setting reset bit enable
		IC_WritByte(TWIC_P3,auto_offset_setting,0X00);//second clear reset bit
	}
}
#endif

#if 0
void VideoSet(unsigned char _vASigStd,unsigned char _vDSigStd,unsigned char _Pb,unsigned char _Pr,unsigned char _YPbPrBalance)
{
	ASigStd = _vASigStd;
	DSigStd = _vDSigStd;
	VSigStd = _vVSigStd;
	Pb = _Pb;
	Pr = _Pr;
	YPbPrBalance = _YPbPrBalance;
}
#else
/*
 * 摘要 :void VideoSet(unsigned char _vVSigStd,unsigned char _vASigStd,unsigned char _vDSigStd,unsigned char _Pb,unsigned char _Pr,unsigned char _YPbPrBalance)
 *
 * 描述 :影像参数设置
 *
 * 参数 :unsigned char _vVSigStd　  =>VGA讯号源　如640X480
		unsigned char _vASigStd     =>CVBS讯号源　如NTSC PAL
		unsigned char _vDSigStd     =>YPBPR序号源　如480P
		unsigned char _Pb           =>adc_blue_offset
		unsigned char _Pr           =>adc_red_offset
		unsigned char _YPbPrBalance =>YPbPrBalance 是否AUTO OFFSET设定完毕　0:尚未　1:以校正
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void VideoSet(unsigned char _vVSigStd,unsigned char _vASigStd,unsigned char _vDSigStd,unsigned char _Pb,unsigned char _Pr,unsigned char _YPbPrBalance)
{
	ASigStd = _vASigStd;
	DSigStd = _vDSigStd;
	VSigStd = _vVSigStd;
	Pb = _Pb;
	Pr = _Pr;
	YPbPrBalance = _YPbPrBalance;
}
#endif
#if 0
void VideoSave(void)
{	
	RUserData VideoData;
	
	VideoData.vASigStd=ASigStd;
	VideoData.vDSigStd=DSigStd;
	VideoData.Pb=Pb;
	VideoData.Pr=Pr;
	VideoData.YPbPrBalance=YPbPrBalance;//finish tag
	//registry_write((unsigned long)&VideoData, sizeof(VideoData));
}
#endif
/*
 * 摘要 :void vde_soft_reset()
 *
 * 描述 :VDE SOFTWARE RESRT VDE重置
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void vde_soft_reset()
{
	IC_WritByte(TWIC_P2,0X3F,0X01);
	twdDelay(200);
	IC_WritByte(TWIC_P2,0X3F,0X00);
}
