/**
 *  @file   videodetect.h
 *  @brief  head file for video detect driver
 *  $Id: videodetect.h,v 1.5 2013/02/06 03:53:23 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.5 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef __VIDEODETECT_H__
#define __VIDEODETECT_H__

#define YPBPR_COLOR_CALIBRATION
#define ENABLE		1
#define DISABLE		0

enum {
    Src_Start = 0,
	Src_CVD,
	Src_YC,
	Src_YUV,
	Src_VGA,
	Src_VIP,
	Src_NONE,
	Src_End
};

enum {
	YUV_480I = 1,
	YUV_576I,	
	YUV_480P,
	YUV_576P,
	YUV_720P_50,
	YUV_720P_60,
	YUV_1080I_50,
	YUV_1080I_60,
	YUV_UNKNOW = 10,
};
enum {
	VGA_720_480=1,
	VGA_720_576,
	VGA_1920_1080,
	VGA_UNKNOW=10,
};
typedef enum //star add 0206 
{
	sigLoss = 0,
	NTSC_M,
	PAL_B,
	PAL_M,
	PAL_N,
	SECAM,
	NTSC_443,
	CVBS_UNKNOW =10,
}eSignalType;

typedef enum
{
	CVBS1 =1,
    CVBS2  ,
    CVBS3  ,
}eChannel;

enum {
	Signal_Lost = 0,
	Signal_Active,
	Signal_Set,
	Signal_init
};
enum {
	VIP_START=0,
	VIP_720_480I,
	VIP_720_576I,
	VIP_720p_NTSC,
	VIP_720P_PAL,
	VIP_1080P_NTSC,
	VIP_1080P_PAL,
	VIP_UNKNOWN,
};
//typedef struct SignalState{
typedef struct SignalState{	
    unsigned char SigActive;    /**/
	unsigned char SigSrc;       /**/
	unsigned char SigStd;       /**/
	unsigned char YUV_ADC_OFFSET_ENABLE;       /**/
	unsigned char DeteSignalTimes;  
	unsigned char DeteSignalType;
	unsigned char DeteSignalType1;
	//unsigned char KeepChannel;
	unsigned char DeteChannel;
	unsigned char YUV_CALIBRATION;
	unsigned char VGA_CALIBRATION;  // yuke 20121214 add
	unsigned char VGA_CALIBRATION_Gain_R;
	unsigned char VGA_CALIBRATION_Gain_G;
	unsigned char VGA_CALIBRATION_Gain_B;
	unsigned char VGA_CALIBRATION_Offset_R;
	unsigned char VGA_CALIBRATION_Offset_G;
	unsigned char VGA_CALIBRATION_Offset_B;
}SigState; 
	
extern SigState sigstate;
extern unsigned short code *VIDEODET_CVD_TAB;
extern unsigned short code *VIDEODET_YUV_TAB;
extern unsigned short code *VIDEODET_VGA_TAB;
extern void  GainSignal(void);
extern void  SignalProcess(void);
extern void SingalDetect(void);
extern void InitStateVal(void);
extern void auto_detection_init(unsigned short code *IML_CVBS_TAB,unsigned short code *IML_YUV_TAB,unsigned short code *IML_VGA_TAB);
extern void auto_detection_ctrl(unsigned char en);
extern void CVDInit(void);
extern void YUVInit(void);
extern void VGAInit(void);
extern void VIPInit(unsigned char std);
#ifdef YPBPR_COLOR_CALIBRATION
 extern bit YPbPr_ADC_Auto_offset(void);
 extern void YPbPr_ADC_Auto_offset_reset(void);
#endif
extern void VideoSet(unsigned char _vVSigStd,unsigned char _vASigStd,unsigned char _vDSigStd,unsigned char _Pb,unsigned char _Pr,unsigned char _YPbPrBalance);

//extern void VideoSet(unsigned char _vASigStd,unsigned char _vDSigStd,unsigned char _Pb,unsigned char _Pr,unsigned char _YPbPrBalance);
extern void VideoSave(void);
extern void vde_soft_reset();

/*ypbpr auto offset calibration*/
//p0
#define adc_red_offset          0x0a
#define adc_blue_offset         0x0c
#define boundary_control        0x26
#define adc_green_offset        0x0b
#define adc_red_gain            0x07
#define adc_green_gain          0x08
#define adc_blue_gain           0x09
//p1
#define color_range_probe       0xc0
#define color_probe_x_low       0xca
#define color_porbe_x_high      0xcb
#define color_probe_y_low       0xcc
#define color_porbe_y_high      0xcd
#define color_probe_pixel_count 0xce
//p3
#define auto_offset_setting        0X00
#define auto_offset_low_boundary  0x01
#define auto_offset_high_boundary 0x02
#define auto_offset_threshold     0x03


#define TwoSignalSystemMode
 
#endif
