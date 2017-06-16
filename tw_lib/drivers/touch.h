/**
 *  @file   touch.h
 *  @brief  head file for touch panel driver
 *  $Id: touch.h,v 1.4 2012/12/17 05:30:32 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef __TOUCH_H__
#define __TOUCH_H__

#define SAR0_INTERRUPT       0x13  //P6 0x13  
#define SAR0_INITIAL_SETTING 0X20  //p3 0x20
#define SAR0_THRESHOLD_LOW   0X21  //p3 0x21
#define SAR0_THRESHOLD_HIGH  0X22  //p3 0x22
#define SAR0_STATUS          0X2C  //p3 0x2c
#define SAR0_TRIG_STATUS     0X2D  //p3 0x2d
#define SAR0_TOUCH_CLOCK     0x2F  //p3 0x2f
#define SAR0_TPMEASURE_GAP   0x31  //p3 0x31
#define SAR0_PDDMEASURE_GAP  0x40  //p3 0x40
#define SAR0_MEASURE_GAP     0x41  //p3 0x41

#define SAR0_BOUN_THREHX     0x50  //P3_50
#define SAR0_BOUN_THREHY     0x51  //P3_51
#define SAR0_TIMES_XY        0x52  //P3_52
#define SAR0_BOUN_THREH0     0x55  //P3_55

#define SAR0_VALUE_LOWXY     0x3D  //P3_3D
#define SAR0_VALUE_HIGHX     0x3E  //P3_3E
#define SAR0_VALUE_HIGHY     0x3F  //P3_3F

#define TOUCH_BUFF_SIZE	18 


typedef struct touchpanel
{
	unsigned char status;
	unsigned short int x;
	unsigned short int y;
}touchd;

extern unsigned short tdata[5];

extern unsigned short td_rep_thres;

extern void touch_init(void);
extern void touch_isr(void);
extern void doTouch(unsigned char choice);
extern touchd touch_read(void);
extern void dotransfer(unsigned short int x_value,unsigned short int y_value,unsigned char status);
extern void drawtarget(unsigned char select);
extern void do_calibration(unsigned char select);
extern void touch_clear(void);
extern void TouchSet(unsigned short _left,unsigned short _right,unsigned short _up,unsigned short _down,unsigned char _calibration_finish);
extern void TouchSave(void);
extern touchd check_repeat(unsigned short threshold);
extern void touch_filter(unsigned char enable,unsigned char time,unsigned char speed);
extern unsigned short repeat_value;

typedef struct touchtemp
{
	unsigned short x;
	unsigned short y;
}tcoord;
typedef struct touchfilter
{
	unsigned char enable:1;//0 :disable 1: enable
	unsigned char type:2;//	0 :initial 1: pen down 2: other
	unsigned char speed:2;//0: no lint 1: 10ms 100times/s 2: 20ms 50times/s 3:30ms 33times/s
	unsigned char time:3;//	less than the value will be discard between first point and second point unit is 10ms
}tfilter;

extern void touch_covalue(unsigned short *left,unsigned short *right,unsigned short *up,unsigned short *down);
extern void touch_coset(unsigned short left,unsigned short right,unsigned short up,unsigned short down);

#endif	/* __TOUCH_H__ */
