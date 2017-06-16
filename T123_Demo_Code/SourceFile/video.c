#include <reg51.h>
#include "sys.h"
#include "tw_irq.h"
#include "serial.h"
#include "cq.h"
#include "sarkey.h"
//#include "img_ctrl.h"
//#include "xgpio.h"
#include "i51_i2c_master.h"
#include "i51_i2c_gpio.h"
#include "timer.h"

#include "nvp6124.h"
#include "video.h"
#include "nvp6124_reg.h"
#include "iccontrol.h"
#include "videodetect.h"

/*for 144MHz data,72M clock sampling*/
//#define SAMPLING_CLK_72M

#ifdef ONLY_720H
#define VDEC_108M
#endif

#ifndef VDEC_108M
#define VDEC_144M
#endif
#define NTSC   0
#define PAL    1
#define SET_ALL_CH          0xff

/*nvp6124 1080P 色彩推薦配置*/
#define BRI_CENTER_VAL_NTSC 0xF4
#define BRI_CENTER_VAL_PAL  0xF4
#define CON_CENTER_VAL_NTSC 0x90
#define CON_CENTER_VAL_PAL  0x90
#define SAT_CENTER_VAL_NTSC 0x80
#define SAT_CENTER_VAL_PAL  0x80                       
#define HUE_CENTER_VAL_NTSC 0x00
#define HUE_CENTER_VAL_PAL  0x00


/*nvp6124 720P 色彩推薦配置*/
#define BRI_CENTER_VAL_NTSC_720P 0x08
#define BRI_CENTER_VAL_PAL_720P  0x08
#define CON_CENTER_VAL_NTSC_720P 0x88
#define CON_CENTER_VAL_PAL_720P  0x88
#define SAT_CENTER_VAL_NTSC_720P 0x90
#define SAT_CENTER_VAL_PAL_720P  0x90                       
#define HUE_CENTER_VAL_NTSC_720P 0xFD
#define HUE_CENTER_VAL_PAL_720P  0x00
                           
/*nvp6124 960H 色彩推薦配置*/
#define BRI_CENTER_VAL_NTSC_960H 0xF8
#define BRI_CENTER_VAL_PAL_960H  0xF5
#define CON_CENTER_VAL_NTSC_960H 0x80
#define CON_CENTER_VAL_PAL_960H  0x79
#define SAT_CENTER_VAL_NTSC_960H 0xA0
#define SAT_CENTER_VAL_PAL_960H  0x80                       
#define HUE_CENTER_VAL_NTSC_960H 0x01
#define HUE_CENTER_VAL_PAL_960H  0x00

const unsigned char nvp6124_con_tbl[2]  = {CON_CENTER_VAL_NTSC, CON_CENTER_VAL_PAL};
const unsigned char nvp6124_hue_tbl[2]  = {HUE_CENTER_VAL_NTSC, HUE_CENTER_VAL_PAL};
const unsigned char nvp6124_sat_tbl[2]  = {SAT_CENTER_VAL_NTSC, SAT_CENTER_VAL_PAL};
const unsigned char nvp6124_bri_tbl[2]  = {BRI_CENTER_VAL_NTSC, BRI_CENTER_VAL_PAL};

const unsigned char nvp6124_con_tbl_720P[2]  = {CON_CENTER_VAL_NTSC_720P, CON_CENTER_VAL_PAL_720P};
const unsigned char nvp6124_hue_tbl_720P[2]  = {HUE_CENTER_VAL_NTSC_720P, HUE_CENTER_VAL_PAL_720P};
const unsigned char nvp6124_sat_tbl_720P[2]  = {SAT_CENTER_VAL_NTSC_720P, SAT_CENTER_VAL_PAL_720P};
const unsigned char nvp6124_bri_tbl_720P[2]  = {BRI_CENTER_VAL_NTSC_720P, BRI_CENTER_VAL_PAL_720P};


const unsigned char nvp6124_con_tbl_960H[2]  = {CON_CENTER_VAL_NTSC_960H, CON_CENTER_VAL_PAL_960H};
const unsigned char nvp6124_hue_tbl_960H[2]  = {HUE_CENTER_VAL_NTSC_960H, HUE_CENTER_VAL_PAL_960H};
const unsigned char nvp6124_sat_tbl_960H[2]  = {SAT_CENTER_VAL_NTSC_960H, SAT_CENTER_VAL_PAL_960H};
const unsigned char nvp6124_bri_tbl_960H[2]  = {BRI_CENTER_VAL_NTSC_960H, BRI_CENTER_VAL_PAL_960H};

const unsigned char nvp6124_motion_sens_tbl[8]= {0xe0,0xc8,0xa0,0x98,0x78,0x68,0x50,0x48};
unsigned char ch_mode_status[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char ch_mode_status_tmp[16]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

extern unsigned char nvp6124_cnt;
extern unsigned char chip_id[4];
extern unsigned char nvp6124_mode;
extern unsigned char channel;
//extern int g_soc_chiptype;
//extern void audio_init(unsigned char dec, unsigned char ch_num, unsigned char samplerate, unsigned char bits);


void nvp6124_common_init(void);
//void nvp6124_pal_common_init(void);
void nvp6124_write_table(unsigned char chip_addr, unsigned char addr, unsigned char *tbl_ptr, unsigned char tbl_cnt);
extern void reset_nvp6114a(void);
extern void nvp6114a_outport_1mux_chseq_mode(unsigned char vformat,unsigned char mode);

void msleep(unsigned short ms)
{
    ms = ms;
	//twdDelay(ms*100);
	//twdDelay(ms*10);
	//twdDelay(ms);
}

unsigned char gpio_i2c_read(unsigned char addr, unsigned char offset)
{
	unsigned char rc = 0;
	unsigned char value = 0;
	rc = io_iIC_rdReg(addr,offset,&value,1);
	if(rc == 0)
	{
		//printk("i2c read error\n");
	}
	return value;
}

unsigned char gpio_i2c_write(unsigned char slave, unsigned char addr, unsigned char datas)
{
	unsigned char rc = 0;
	unsigned char value = datas;
	rc = io_iIC_wrReg(slave,addr,&value,1);
	if(rc == 0)
	{
		//printk("i2c write error\n");
	}
	return value;
}

//extern unsigned int nvp6124_mode;
extern unsigned char nvp6124_slave_addr[4];
void NVP6124_AfeReset(void)
{
	gpio_i2c_write(nvp6124_slave_addr[0], 0xFF, 0x00);
	gpio_i2c_write(nvp6124_slave_addr[1], 0xFF, 0x00);
	gpio_i2c_write(nvp6124_slave_addr[0], 0x02, gpio_i2c_read(nvp6124_slave_addr[0], 0x02)|0x0F);
	gpio_i2c_write(nvp6124_slave_addr[1], 0x02, gpio_i2c_read(nvp6124_slave_addr[1], 0x02)|0x0F);
	gpio_i2c_write(nvp6124_slave_addr[0], 0x02, gpio_i2c_read(nvp6124_slave_addr[0], 0x02)&0xF0);
	gpio_i2c_write(nvp6124_slave_addr[1], 0x02, gpio_i2c_read(nvp6124_slave_addr[1], 0x02)&0xF0);
	printk("NVP6124_AfeReset done\n");
}

void nvp6124_datareverse(void)
{
/*
BANK1 0xD2[5:2],每個bit控制一個bt656的數據順序，1為反序，0為正序。
*/
	int i = 0;                               
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x01);
		//gpio_i2c_write(nvp6124_slave_addr[i], 0xD2, 0x3C); 
		gpio_i2c_write(nvp6124_slave_addr[i], 0xD2, 0x0ff); //0xFF
	}
	printk("nvp6124 data reversed\n");
	IC_WritByte(TWIC_P0,0xE8,0x10);
	IC_WritByte(TWIC_P0,0xE9,0x00);
	IC_WritByte(TWIC_P0,0xEA,0x10);
	IC_WritByte(TWIC_P0,0xEB,0xFF);
	IC_WritByte(TWIC_P0,0xE2,0x11);
}

void nvp6124_system_init(void)
{
    int i = 0;                               
	for(i=0;i<nvp6124_cnt;i++)
	{
			gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x01); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x82, 0x12); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x83, 0x2C); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x3e, 0x10); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x80, 0x60); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x80, 0x61); 
			twdDelay(1000); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x80, 0x40); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x81, 0x02); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x97, 0x00); 
			twdDelay(100); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x80, 0x60); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x81, 0x00); 
			twdDelay(100); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x97, 0x0F); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x38, 0x18); 
			gpio_i2c_write(nvp6124_slave_addr[i], 0x38, 0x08); 
			twdDelay(100); 
			printk("nvp6124b_system_init\n");
			twdDelay(1000);
			gpio_i2c_write(nvp6124_slave_addr[i], 0xCA, 0x22);//0xAE
	}
}

void software_reset(void)
{
	  int i = 0;                               
	for(i=0;i<nvp6124_cnt;i++)
	{
	    gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x01); 
	    gpio_i2c_write(nvp6124_slave_addr[i], 0x80, 0x40); 
	    gpio_i2c_write(nvp6124_slave_addr[i], 0x81, 0x02);
	    gpio_i2c_write(nvp6124_slave_addr[i], 0x80, 0x61);
	    gpio_i2c_write(nvp6124_slave_addr[i], 0x81, 0x00);
	    msleep(100); 
	    gpio_i2c_write(nvp6124_slave_addr[i], 0x80, 0x60); 
	    gpio_i2c_write(nvp6124_slave_addr[i], 0x81, 0x00);
	    msleep(100); 
	}
	printk("\n\r nvp6124 software reset!!!\n");
}           

void nvp6124_common_init(void)
{
	int i = 0;

	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x00);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B0_Buf,254);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x01);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B1_Buf,254);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x02);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B2_Buf,254);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x03);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B3_Buf,254);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x04);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B4_Buf,254);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x09);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_B9_Buf,254);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x0A);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_BA_Buf,254);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x0B);
        nvp6124_write_table(nvp6124_slave_addr[i],0x00,NVP6124_BB_Buf,254);
	}

	nvp6124_system_init();
}

#if 0
void mpp2clk(unsigned char clktype)
{
	int i=0;
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x01);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xD4, 0x0F);  //mpp1,2,3,4 port clock func enables
		gpio_i2c_write(nvp6124_slave_addr[i], 0xB4, 0x66);  //clock&delay setting
		gpio_i2c_write(nvp6124_slave_addr[i], 0xB5, 0x66);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xB6, 0x66);  
		gpio_i2c_write(nvp6124_slave_addr[i], 0xB7, 0x66);
	}
}
#endif

/*nvp6124b视频模式值转换*/
static unsigned int nvp6124b_vdsts2_nvp6124(unsigned int vdsts)
{
	unsigned int ret;
	switch(vdsts)
	{
		case 0x00:			//nvp6124b NTSC SD状态值
			ret = 0x01;		//nvp6124  NTSC SD状态值
		break;
		case 0x10:          //nvp6124b PAL SD状态值
			ret = 0x02;
		break;
		case 0x20:			//nvp6124b NT 720P/30状态值
			ret = 0x04;
		break;
		case 0x21:
			ret = 0x08;
		break;
		case 0x22:
			ret = 0x10;
		break;
		case 0x23:
			ret = 0x20;
		break;
		case 0x30:
			ret = 0x40;
		break;
		case 0x31:
			ret = 0x80;
		break;
		case 0xFF:
			ret = 0x00;
		break;
		default:
			ret = vdsts;
		break;
	}
	return ret;
}

#define CNT 5
unsigned int video_fmt_debounce(unsigned char ch)
{
	unsigned int idx=CNT-1;
	unsigned int tmp, buf[CNT]={0,0,0,0,0};
	int i, j,cnt[CNT]={0,0,0,0,0};
		
	for(i=0; i<CNT; i++)
	{

		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
		tmp = gpio_i2c_read(nvp6124_slave_addr[ch/4], 0xF0);
		buf[i] = nvp6124b_vdsts2_nvp6124(tmp);
		if(buf[i] == 0)
		{
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
			buf[i] = gpio_i2c_read(nvp6124_slave_addr[ch/4], 0xD0+(ch%4));
		}
	}
	for(i=0; i<CNT; i++)
    {
        for(j=0; j<CNT; j++)
        {
            if(buf[i] == buf[j])
            {
            	cnt[i]++;
            }
        }
        if(i>0 && cnt[i-1]>=cnt[i])
	        idx = i-1;      
    }
	printk("video_fmt_debounce ch[%d] buf[%d] = %x\n", (int)ch, (int)idx, (int)buf[idx]);
	return buf[idx];
}

/*視頻輸入制式檢測函數*/
void video_fmt_det(nvp6124_input_videofmt *pvideofmt)
{
    static int count = 0;
	int i=0;
	unsigned char tmp;
	//static nvp6124_input_videofmt videofmt;
	
	for(i=0; i<nvp6124_cnt*4-2; i++)
	{
		gpio_i2c_write(nvp6124_slave_addr[i/4], 0xFF, 0x05 + i%4);
		tmp =  gpio_i2c_read(nvp6124_slave_addr[i/4], 0xF0);
		pvideofmt->getvideofmt[i] = nvp6124b_vdsts2_nvp6124(tmp);
		if(pvideofmt->getvideofmt[i] == 0x00)
		{
			gpio_i2c_write(nvp6124_slave_addr[i/4], 0xFF, 0x00);
			pvideofmt->getvideofmt[i] = gpio_i2c_read(nvp6124_slave_addr[i/4], 0xD0+i%4);
		}
	#if 0
		if(videofmt.getvideofmt[i] != pvideofmt->getvideofmt[i])
		{
		    count = 0;
			//pvideofmt->getvideofmt[i] = video_fmt_debounce(i);
			videofmt.getvideofmt[i] = pvideofmt->getvideofmt[i];
			return;
		}
		else
		{
		    if(count++ < 10)
				return;
		}
	#endif
		if(count++ >= 200)
		{
			count=0;
		    //printk("videofmt:0x%02x----i:0x%02x\n",pvideofmt->getvideofmt[i], i);
		}
		if((0x01==pvideofmt->getvideofmt[i])&&((nvp6124_mode != NTSC_MODE)||(nvp6124_difinition != NVP6124_VI_720H)||(channel!=i)))
		{
			printk("signal change!\n");
			ClosePanel(0x10, 0xf0, 0x80);
			reset_nvp6114a();	
			channel = i;
	        nvp6114a_outport_1mux_chseq_mode(NTSC_MODE, NVP6124_VI_720H);
			VIPInit(VIP_720_480I);
			OpenPanel();
		}
		else if((0x02==pvideofmt->getvideofmt[i])&&((nvp6124_mode != PAL_MODE)||(nvp6124_difinition != NVP6124_VI_720H)||(channel!=i)))
		{
		    printk("signal change!\n");
			ClosePanel(0x10, 0xf0, 0x80);
			reset_nvp6114a();	
			channel = i;
	        nvp6114a_outport_1mux_chseq_mode(PAL_MODE, NVP6124_VI_720H);
			VIPInit(VIP_720_576I);
			OpenPanel();
		}
		else if((0x04==pvideofmt->getvideofmt[i])&&((nvp6124_mode != NTSC_MODE)||(nvp6124_difinition != NVP6124_VI_720P_2530)||(channel!=i)))
		{
		    printk("signal change!\n");
			ClosePanel(0x10, 0xf0, 0x80);
			reset_nvp6114a();	
			channel = i;
	        nvp6114a_outport_1mux_chseq_mode(NTSC_MODE, NVP6124_VI_720P_2530);
			VIPInit(VIP_720p_NTSC);
			OpenPanel();
		}
		else if((0x08==pvideofmt->getvideofmt[i])&&((nvp6124_mode != PAL_MODE)||(nvp6124_difinition != NVP6124_VI_720P_2530)||(channel!=i)))
		{
		    printk("signal change!\n");
			ClosePanel(0x10, 0xf0, 0x80);
			reset_nvp6114a();	
			channel = i;
	        nvp6114a_outport_1mux_chseq_mode(PAL_MODE, NVP6124_VI_720P_2530);
			VIPInit(VIP_720P_PAL);			
			OpenPanel();
		}
		else if((0x40==pvideofmt->getvideofmt[i])&&((nvp6124_mode != NTSC_MODE)||(nvp6124_difinition != NVP6124_VI_1080P_2530)||(channel!=i)))
		{
		    printk("signal change!\n");
			ClosePanel(0x10, 0xf0, 0x80);
			reset_nvp6114a();	
			channel = i;
	        nvp6114a_outport_1mux_chseq_mode(NTSC_MODE, NVP6124_VI_1080P_2530);
			VIPInit(VIP_1080P_NTSC);
			OpenPanel();
		}
		else if((0x80==pvideofmt->getvideofmt[i])&&((nvp6124_mode != PAL_MODE)||(nvp6124_difinition != NVP6124_VI_1080P_2530)||(channel!=i)))
		{
		    printk("signal change!\n");
			ClosePanel(0x10, 0xf0, 0x80);
			reset_nvp6114a();
			channel = i;
	        nvp6114a_outport_1mux_chseq_mode(PAL_MODE, NVP6124_VI_1080P_2530);
			VIPInit(VIP_1080P_PAL);
			
			OpenPanel();
		}
	}
}

unsigned int nvp6124_getvideoloss(void)
{
	unsigned int vloss=0, i;
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x00);
		vloss|=(gpio_i2c_read(nvp6124_slave_addr[i], 0xB8)&0x0F)<<(4*i);
	}
	return vloss;
}

//#define _EQ_ADJ_COLOR_
extern unsigned char vloss;
unsigned char setcnt[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
unsigned char stage_update=0;

const unsigned char ANALOG_EQ_1080P[8]  = {0x13,0x03,0x53,0x73,0x73,0x73,0x73,0x73};
const unsigned char DIGITAL_EQ_1080P[8] = {0x00,0x00,0x00,0x00,0x8B,0x8F,0x8F,0x8F};
#ifdef _EQ_ADJ_COLOR_
const unsigned char BRI_EQ_1080P[8]    = {0xF4,0xF4,0xF4,0xF4,0xF8,0xF8,0xF8,0xF8};
const unsigned char CON_EQ_1080P[8]    = {0x90,0x90,0x90,0x90,0x90,0x90,0x80,0x80};
const unsigned char SAT_EQ_1080P[8]    = {0x80,0x80,0x80,0x78,0x78,0x78,0x78,0x78};
const unsigned char BRI_EQ_720P[9]    = {0xF4,0xF4,0xF4,0xF4,0xF8,0xF8,0xF8,0xF8,0xF8};
const unsigned char CON_EQ_720P[9]    = {0x90,0x90,0x90,0x90,0x88,0x88,0x84,0x90,0x90};
const unsigned char SAT_EQ_720P[9]    = {0x84,0x84,0x84,0x80,0x80,0x80,0x80,0x84,0x84};
#endif
const unsigned char SHARP_EQ_1080P[8]  = {0x90,0x90,0x99,0x99,0x99,0x99,0x99,0x90};
const unsigned char PEAK_EQ_1080P[8]   = {0x00,0x10,0x00,0x00,0x00,0x00,0x50,0x00};
const unsigned char CTI_EQ_1080P[8]    = {0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A};
const unsigned char C_LOCK_EQ_1080P[8] = {0x92,0x92,0x92,0x92,0x92,0x92,0xA2,0xA2};
const unsigned char UGAIN_EQ_1080P[8]  = {0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00};
const unsigned char VGAIN_EQ_1080P[8]  = {0x00,0x00,0x00,0x00,0x10,0x10,0x20,0x00};


const unsigned char SHARP_EQ_720P[9]   =  {0x90,0x90,0x99,0x99,0x99,0x99,0x99,0x90,0x90};
const unsigned char PEAK_EQ_720P[9]    =  {0x00,0x20,0x10,0x10,0x00,0x00,0x40,0x20,0x20};
const unsigned char CTI_EQ_720P[9]     =  {0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A,0x0A};
const unsigned char C_LOCK_EQ_720P[9]  =  {0x92,0x92,0x92,0x92,0x92,0x92,0xA2,0x92,0xA2};
const unsigned char UGAIN_EQ_720P[9]   =  {0x30,0x30,0x30,0x30,0x30,0x30,0x40,0x30,0x30};
const unsigned char VGAIN_EQ_720P[9]   =  {0x30,0x30,0x30,0x30,0x30,0x30,0x40,0x30,0x30};
const unsigned char ANALOG_EQ_720P[9]  =  {0x13,0x03,0x53,0x73,0x73,0x73,0x73,0x03,0x13};
const unsigned char DIGITAL_EQ_720P[9] =  {0x00,0x00,0x00,0x00,0x88,0x8B,0x8F,0x00,0x00}; 

unsigned char eq_stage[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
#ifdef _EQ_ADJ_COLOR_
void nvp6124_brightness_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4],(0x0c+(ch%4)),BRI_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4],(0x0c+(ch%4)),BRI_EQ_1080P[stage]);
}

void nvp6124_contrast_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),CON_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),CON_EQ_1080P[stage]);
}

void nvp6124_saturation_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x3C+(ch%4)),SAT_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x3C+(ch%4)),SAT_EQ_1080P[stage]);
}
#endif
void nvp6124_cl_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x21+4*(ch%4)),C_LOCK_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x21+4*(ch%4)),C_LOCK_EQ_1080P[stage]);
}


void nvp6124_sharpness_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x14+(ch%4)),SHARP_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x14+(ch%4)),SHARP_EQ_1080P[stage]);
}

void nvp6124_peaking_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x18+(ch%4)),PEAK_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x18+(ch%4)),PEAK_EQ_1080P[stage]);
}

void nvp6124_ugain_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x44+(ch%4)),UGAIN_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x44+(ch%4)),UGAIN_EQ_1080P[stage]);
}

void nvp6124_vgain_eq(unsigned int ch,  unsigned int stage)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
    if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x48+(ch%4)),VGAIN_EQ_720P[stage]);
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x48+(ch%4)),VGAIN_EQ_1080P[stage]);
}

unsigned long val_bak, val_bak_b;
void nvp6124_set_equalizer(void)
{
//	unsigned char tmpbuf;
	unsigned char eqindex=0, ch;
//	unsigned char tmpval;
	unsigned char stage_update;
	unsigned long val, val1;
	unsigned long buf[16];
	unsigned char c_eq_stage, y_eq_stage;

	for(ch=0;ch<nvp6124_cnt*4;ch++)
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
		buf[0] = vloss; 
		buf[1] = gpio_i2c_read(nvp6124_slave_addr[ch/4],0xEC);
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x05 + ch%4);
		buf[2] = gpio_i2c_read(nvp6124_slave_addr[ch/4],0xE2)&0x07;
		buf[3] = gpio_i2c_read(nvp6124_slave_addr[ch/4],0xE3);
		buf[4] = gpio_i2c_read(nvp6124_slave_addr[ch/4],0xEA)&0x07;
		buf[5] = gpio_i2c_read(nvp6124_slave_addr[ch/4],0xEB);
		
		if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		{
			val = buf[2];
			val <<= 8;
			val |= buf[3];
			val &= 0x7FF;

			if((((buf[0]>>(ch%4))&0x01) == 0x00) &&  (((buf[1]>>(ch%4))&0x01) == 0x01))
			{
				setcnt[ch]++;

				if(setcnt[ch] == 4)	stage_update = 1;
				else			stage_update = 0;
			}
			else if((((buf[0]>>(ch%4))&0x01) == 0x01) &&  (((buf[1]>>(ch%4))&0x01) == 0x00))
			{
				stage_update = 1;
				setcnt[ch] = 0;
			}
			else
			{
				stage_update = 0;
				setcnt[ch] = 0;
			}

			if(stage_update)
			{
				val_bak = val;
			}

			if	   (val_bak >= 0x00  && val_bak < 0x52  )   c_eq_stage = 1;
			else if(val_bak >= 0x52  && val_bak < 0x89  )   c_eq_stage = 2;
			else if(val_bak >= 0x89  && val_bak < 0x113 )   c_eq_stage = 3;
			else if(val_bak >= 0x113 && val_bak < 0x25F )   c_eq_stage = 4;
			else if(val_bak >= 0x25F && val_bak < 0x700 )   c_eq_stage = 5;
			else if(val_bak >= 0x700 && val_bak < 0x7FF )   c_eq_stage = 6;
			else 
			{
				if((((buf[0]>>(ch%4))&0x01) == 0x00) &&  (((buf[1]>>(ch%4))&0x01) == 0x01))
					c_eq_stage = 7;
				else
					c_eq_stage = 0;
			}


			val1 = buf[4];
			val1 <<= 8;
			val1 |= buf[5];

			if(stage_update)
			{
				val_bak_b = val1;
			}

			if     (val_bak_b == 0x000)                         y_eq_stage = 0;
			else if(val_bak_b >  0x000 && val_bak_b < 0x0E7)    y_eq_stage = 1;
			else if(val_bak_b >= 0x0E7 && val_bak_b < 0x11A)    y_eq_stage = 2;
			else if(val_bak_b >= 0x11A && val_bak_b < 0x151)    y_eq_stage = 3;
			else if(val_bak_b >= 0x151 && val_bak_b < 0x181)    y_eq_stage = 4;
			else if(val_bak_b >= 0x181 && val_bak_b < 0x19B)    y_eq_stage = 5;
			else if(val_bak_b >= 0x19B && val_bak_b < 0x200)    y_eq_stage = 6;
			else												y_eq_stage = 7;


			if(c_eq_stage >0 )
				eq_stage[ch] = y_eq_stage;
			else
				eq_stage[ch] = c_eq_stage;

			printk("1080P stage_update = %d, eq_stage[%d]=%d,y_eq_stage=%d,c_eq_stage=%d,val_bak=%x,val_bak_b=%x\n", stage_update, ch, eq_stage[ch], y_eq_stage, c_eq_stage,val_bak,val_bak_b);
			if(stage_update)
			{
				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF,0x05+ch%4);
				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_1080P[eq_stage[ch]]);
				
				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF,((ch%4)<2)?0x0A:0x0B);         
				gpio_i2c_write(nvp6124_slave_addr[ch/4], (ch%2==0)?0x3B:0xBB, DIGITAL_EQ_1080P[eq_stage[ch]]);
				#ifdef _EQ_ADJ_COLOR_
				nvp6124_brightness_eq(ch, eq_stage[ch]);
				nvp6124_contrast_eq(ch, eq_stage[ch]);
				nvp6124_saturation_eq(ch, eq_stage[ch]);
				#endif
				nvp6124_cl_eq(ch, eq_stage[ch]);
				nvp6124_sharpness_eq(ch, eq_stage[ch]);
				nvp6124_peaking_eq(ch, eq_stage[ch]);
				nvp6124_ugain_eq(ch, eq_stage[ch]);
				nvp6124_vgain_eq(ch, eq_stage[ch]);
			}
		}
		else if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
		{
			val = buf[2];
			val <<= 8;
			val |= buf[3];


			if((((buf[0]>>(ch%4))&0x01) == 0x00) &&  (((buf[1]>>(ch%4))&0x01) == 0x01))
			{
				setcnt[ch]++;

				if(setcnt[ch] == 4)	stage_update = 1;
				else			stage_update = 0;
			}
			else if((((buf[0]>>(ch%4))&0x01) == 0x01) &&  (((buf[1]>>(ch%4))&0x01) == 0x00))
			{
				stage_update = 1;
				setcnt[ch] = 0;
			}
			else
			{
				stage_update = 0;
				setcnt[ch] = 0;
			}
			if(stage_update )
			{
				val_bak = val;
			}

			if	   (val_bak >= 0x00  && val_bak < 0x55 )   c_eq_stage = 1;
			else if(val_bak >= 0x55  && val_bak < 0x82 )   c_eq_stage = 2;
			else if(val_bak >= 0x82  && val_bak < 0xD8 )   c_eq_stage = 3;
			else if(val_bak >= 0xD8  && val_bak < 0x18F)   c_eq_stage = 4;
			else if(val_bak >= 0x18F && val_bak < 0x700)   c_eq_stage = 5;
			else if(val_bak >= 0x700 && val_bak < 0x7FF)   c_eq_stage = 6;
			else 
			{
				if((((buf[0]>>(ch%4))&0x01) == 0x00) &&  (((buf[1]>>(ch%4))&0x01) == 0x01))
					c_eq_stage = 7;
				else
					c_eq_stage = 0;
			}

			val1 = buf[4];
			val1 <<= 8;
			val1 |= buf[5];

			if(stage_update )
			{
				val_bak_b = val1;
			}

			if     (val_bak_b == 0x000)                         y_eq_stage = 0;
			else if(val_bak_b >  0x000 && val_bak_b < 0xF0 )    y_eq_stage = 1;
			else if(val_bak_b >= 0x0F0 && val_bak_b < 0x125)    y_eq_stage = 2;
			else if(val_bak_b >= 0x125 && val_bak_b < 0x14C)    y_eq_stage = 3;
			else if(val_bak_b >= 0x14C && val_bak_b < 0x16F)    y_eq_stage = 4;
			else if(val_bak_b >= 0x16F && val_bak_b < 0x185)    y_eq_stage = 5;
			else											    y_eq_stage = 6;

			if(  	((c_eq_stage > 0 && c_eq_stage < 3) && val_bak_b >= 0x160)  
				|| 	((c_eq_stage == 3) && val_bak_b >= 0x168) 
				|| 	((c_eq_stage == 4) && val_bak_b >= 0x195) )
			{
			    y_eq_stage = 7;
				c_eq_stage = 7;
			}	
			else if(	((c_eq_stage == 5) && val_bak_b >= 0x1A0) ||
			    		(val_bak == 0x7FF  && val_bak_b >= 0x1B0)   )  
			{
			    y_eq_stage = 8;
				c_eq_stage = 8;
			}
			
					
			if(c_eq_stage>0)
				eq_stage[ch] = y_eq_stage;
			else
				eq_stage[ch] = c_eq_stage;

			printk("720P stage_update = %d, eq_stage[%d]=%d,y_eq_stage=%d,c_eq_stage=%d,val_bak=%x,val_bak_b=%x\n", stage_update, ch, eq_stage[ch], y_eq_stage, c_eq_stage,val_bak,val_bak_b);
			if(stage_update)
			{
				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF,0x05+ch%4);
				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58, ANALOG_EQ_720P[eq_stage[ch]]);
				
				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF,((ch%4)<2)?0x0A:0x0B);         
				gpio_i2c_write(nvp6124_slave_addr[ch/4], (ch%2==0)?0x3B:0xBB, DIGITAL_EQ_720P[eq_stage[ch]]);
				#ifdef _EQ_ADJ_COLOR_
				nvp6124_brightness_eq(ch, eq_stage[ch]);
				nvp6124_contrast_eq(ch, eq_stage[ch]);
				nvp6124_saturation_eq(ch, eq_stage[ch]);
				#endif
				nvp6124_cl_eq(ch, eq_stage[ch]);
				nvp6124_sharpness_eq(ch, eq_stage[ch]);
				nvp6124_peaking_eq(ch, eq_stage[ch]);
				nvp6124_ugain_eq(ch, eq_stage[ch]);
				nvp6124_vgain_eq(ch, eq_stage[ch]);
			}
		}
	}
}

/*
nvp6124 has 4 BT656 output ports.
nvp6114a only has 2, so ch_seq[2]&ch_seq[3] are invalid in nvp6114a.
*/
unsigned char ch_seq[4]={2,1,0,3};
void nvp6124_outport_1mux_chseq(void)
{
	int i = 0;                               
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x01);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC0, ch_seq[0]<<4|ch_seq[0]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC1, ch_seq[0]<<4|ch_seq[0]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC2, ch_seq[1]<<4|ch_seq[1]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC3, ch_seq[1]<<4|ch_seq[1]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC4, ch_seq[2]<<4|ch_seq[2]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC5, ch_seq[2]<<4|ch_seq[2]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC6, ch_seq[3]<<4|ch_seq[3]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC7, ch_seq[3]<<4|ch_seq[3]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC8, 0x00);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC9, 0x00);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xCA, 0xFF);
	}
	printk("nvp6124_outport_1mux_chseq\n");
}

/*
vformat:0->NTSC, 1->PAL
portx_mode: 
高4bit選擇port接口[7:4]->0~3:port0~3; 
低4bit選擇vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530, NVP6124_VI_1080P_2530)
*/
void nvp6124_outport_2mux(unsigned char vformat, unsigned char port1_mode, unsigned char port2_mode )
{
	int ch, i, tmp=0;
	unsigned char p1_num,p2_num,port1_vimode,port2_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	p2_num = port2_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	port2_vimode = port2_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		if(ch%4 < 2)
			vmode.chmode[ch] = port1_vimode;
		else
			vmode.chmode[ch] = port2_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write( nvp6124_slave_addr[i], 0xFF, 0x01);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC0+p1_num*2, 0x10);    
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC1+p1_num*2, 0x10);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC0+p2_num*2, 0x32);    
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC1+p2_num*2, 0x32);
		tmp = (port2_vimode==NVP6124_VI_1080P_2530?1:2)<<(p2_num*4)|((port1_vimode==NVP6124_VI_1080P_2530?1:2)<<(p1_num*4));
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC8, tmp&0xFF);         
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC9, (tmp>>8)&0xFF);    

		tmp=((1<<p1_num)|(1<<p2_num));
		gpio_i2c_write( nvp6124_slave_addr[i], 0xCA, (tmp<<4|tmp));    //打開端口[3:0]和時鐘[7:4],和硬件相關。
		
		if(port1_vimode == NVP6124_VI_SD)
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCC+p1_num, 0x46);    //時鐘頻率設置
		else
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCC+p1_num, 0x66);
		if(port2_vimode == NVP6124_VI_SD)
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCC+p2_num, 0x46);    //時鐘頻率設置
		else
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCC+p2_num, 0x66);
  	}
	printk("nvp6124_outport_2mux setting\n");
}

/*
vformat:0->NTSC, 1->PAL
port1_mode: 
高4bit選擇port接口[7:4]->0~3:port0~3; 
低4bit選擇vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530)
*/
void nvp6124_outport_4mux(unsigned char vformat, unsigned char port1_mode )
{
	int ch, i, tmp=0;
	unsigned char p1_num,port1_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		vmode.chmode[ch] = port1_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write( nvp6124_slave_addr[i], 0xFF, 0x00);
		gpio_i2c_write( nvp6124_slave_addr[i], 0x55, 0x10);
		gpio_i2c_write( nvp6124_slave_addr[i], 0x56, 0x32);  //reset channel id
		gpio_i2c_write( nvp6124_slave_addr[i], 0xFF, 0x01);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC0+p1_num*2, 0x10);    
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC1+p1_num*2, 0x32);
		tmp = ((NVP6124_VI_720P_2530?3:8)<<(p1_num*4));
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC8, tmp&0xFF);         
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC9, (tmp>>8)&0xFF);    

		tmp=(1<<p1_num);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xCA, (tmp<<4|tmp));    //打開端口[3:0]和時鐘[7:4],和硬件相關。代
		
		gpio_i2c_write( nvp6124_slave_addr[i], 0xCC+p1_num, 0x66);    //時鐘頻率設置
  	}
	printk("nvp6124_outport_4mux setting\n");
}

void nvp6114a_outport_1mux_chseq(void)
{
	int i = 0;    
	ch_seq[0] = 1;   // port A outputs channel0's video data
	ch_seq[1] = 0;   // port B outputs channel1's video data
	ch_seq[2] = 0xFF;
	ch_seq[3] = 0xFF;
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write(nvp6124_slave_addr[i], 0xFF, 0x01);
		//gpio_i2c_write(nvp6124_slave_addr[i], 0xC0, ch_seq[0]<<4|ch_seq[0]);
		//gpio_i2c_write(nvp6124_slave_addr[i], 0xC1, ch_seq[0]<<4|ch_seq[0]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC2, ch_seq[0]<<4|ch_seq[0]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC3, ch_seq[0]<<4|ch_seq[0]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC4, ch_seq[1]<<4|ch_seq[1]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC5, ch_seq[1]<<4|ch_seq[1]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC6, ch_seq[1]<<4|ch_seq[1]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC7, ch_seq[1]<<4|ch_seq[1]);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC8, 0x00);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xC9, 0x00);
		gpio_i2c_write(nvp6124_slave_addr[i], 0xCA, 0xFF);
	}
	printk("nvp6114a_outport_1mux_chseq\n");
}

/*
vformat:0->NTSC, 1->PAL
portx_mode: 
高4bit選擇port接口[7:4]->0~1:port0~1; 
低4bit選擇vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530, NVP6124_VI_1080P_2530)
*/
void nvp6114a_outport_2mux(unsigned char vformat, unsigned char port1_mode, unsigned char port2_mode )
{
	int ch, i;
	unsigned char tmp=0;
	unsigned char p1_num,p2_num,port1_vimode,port2_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	p2_num = port2_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	port2_vimode = port2_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		if(ch%4 < 2)
			vmode.chmode[ch] = port1_vimode;
		else
			vmode.chmode[ch] = port2_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write( nvp6124_slave_addr[i], 0xFF, 0x01);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC2+p1_num*2, 0x10);    
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC3+p1_num*2, 0x10);
		//gpio_i2c_write( nvp6124_slave_addr[i], 0xC2+p2_num*2, 0x32);    
		//gpio_i2c_write( nvp6124_slave_addr[i], 0xC3+p2_num*2, 0x32);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC2+p2_num*2, 0x10);    
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC3+p2_num*2, 0x10);
				
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC6, gpio_i2c_read( nvp6124_slave_addr[i], 0xC4));
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC7, gpio_i2c_read( nvp6124_slave_addr[i], 0xC5));
		tmp = ((port1_vimode==NVP6124_VI_1080P_2530?1:2)<<4);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC8, tmp);         
		tmp = ((port2_vimode==NVP6124_VI_1080P_2530?1:2)<<4)|((port2_vimode==NVP6124_VI_1080P_2530?1:2));
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC9, tmp);    

		//tmp=((1<<p1_num)|(1<<p2_num));
		gpio_i2c_write( nvp6124_slave_addr[i], 0xCA, 0xFF);    //打開端口[3:0]和時鐘[7:4],和硬件相關。
		
		if(port1_vimode == NVP6124_VI_SD)
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCD+p1_num*2, 0x46);    //時鐘頻率設置
		else
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCD+p1_num*2, 0x66);
		if(port2_vimode == NVP6124_VI_SD)
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCD+p2_num*2, 0x46);    //時鐘頻率設置
		else
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCD+p2_num*2, 0x66);
  	}
	printk("nvp6114A_outport_2mux setting\n");
}


/*
vformat:0->NTSC, 1->PAL
port1_mode: 
高4bit選擇port接口[7:4]->0~1:port0~1; 
低4bit選擇vi模式[3:0]-> (NVP6124_VI_SD,NVP6124_VI_720P_2530)
*/
void nvp6114a_outport_4mux(unsigned char vformat, unsigned char port1_mode)
{
	int ch, i;
	unsigned char tmp=0;
	unsigned char p1_num,port1_vimode;
	nvp6124_video_mode vmode;

	p1_num = port1_mode>>0x04;
	port1_vimode = port1_mode&0x0F;
	for(ch=0;ch<nvp6124_cnt*4;ch++)
  	{
  		vmode.vformat[0] = vformat;
		vmode.chmode[ch] = port1_vimode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<nvp6124_cnt;i++)
	{
		gpio_i2c_write( nvp6124_slave_addr[i], 0xFF, 0x00);
		gpio_i2c_write( nvp6124_slave_addr[i], 0x55, 0x10);
		gpio_i2c_write( nvp6124_slave_addr[i], 0x56, 0x32);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xFF, 0x01);
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC2+p1_num*2, 0x10);    
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC3+p1_num*2, 0x32);
		
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC6, gpio_i2c_read( nvp6124_slave_addr[i], 0xC4));
		gpio_i2c_write( nvp6124_slave_addr[i], 0xC7, gpio_i2c_read( nvp6124_slave_addr[i], 0xC5));
		tmp = ((port1_vimode==NVP6124_VI_720P_2530?3:8)<<4)|((port1_vimode==NVP6124_VI_720P_2530?3:8));
		if(p1_num == 0)
		{
			gpio_i2c_write( nvp6124_slave_addr[i], 0xC8, tmp);
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCA, 0x22);
		}
		else
		{
			gpio_i2c_write( nvp6124_slave_addr[i], 0xC9, tmp); 
			gpio_i2c_write( nvp6124_slave_addr[i], 0xCA, 0x8C);
		}		
		gpio_i2c_write( nvp6124_slave_addr[i], 0xCD+p1_num*2, 0x66);    //時鐘頻率設置
  	}
	printk("nvp6114A_outport_4mux setting\n");
}

void nvp6124_set_fpc(unsigned char ch, unsigned char vformat)
{
	unsigned char tmp;
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	tmp = gpio_i2c_read(nvp6124_slave_addr[ch/4], 0x54);
	if(vformat==PAL)
		tmp &= ~(0x10<<(ch%4));
	else	
		tmp &= ~(0x10<<(ch%4)); //tmp |= (0x10<<(ch%4));
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x54, tmp);
}

void nvp6124b_syncchange(unsigned char ch, nvp6124_video_mode *pvmode)
{
	unsigned char vformat = pvmode->vformat[0];

	if(vformat == NTSC)
	{
		if(pvmode->chmode[ch] == NVP6124_VI_1080P_2530)
		{
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x00);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x23+(ch%4)*4,0x83);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58+(ch%4),0x4E);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (ch%4)+0x8E,0x09);

			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x05+(ch%4));
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x24,0x1A);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x47,0xEE);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50,0xC6);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xBB,0x04);
		}
		else if(pvmode->chmode[ch]  == NVP6124_VI_720P_5060)
		{
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x00);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x23+(ch%4)*4,0x83);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58+(ch%4),0xa8);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (ch%4)+0x8E,0x09);

			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x05+(ch%4));
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x24,0x2A); // 
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x47,0xEE);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50,0xC6);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xBB,0x04);
		}
		else
		{
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x00);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x23+(ch%4)*4,0x83);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x05+(ch%4));
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x47,0x04);
		}
	}
	else
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x00);
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x23+(ch%4)*4,0x83);
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xff,0x05+(ch%4));
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x47,0x04);
	}
}

void init_acp(unsigned char ch)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
	//Decoder TX Setting
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xBC, 0xF7);
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xBD, 0xF7);
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA8, 0xA5);
		//ACP_RX_D0 = 0x50;
	}

	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x3A+((ch%2)*0x80), 0x01);
	twdDelay(100);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x3A+((ch%2)*0x80), 0x00);
	twdDelay(1000);
}

void acp_each_setting(unsigned char ch)
{
	init_acp(ch);
	
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));
	if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x00+((ch%2)*0x80), 0x0E);
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x03+((ch%2)*0x80), nvp6124_mode==PAL? 0x0D:0x0E);
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x00+((ch%2)*0x80), 0x0E);
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x03+((ch%2)*0x80), 0x0E);
	}
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x0D+((ch%2)*0x80), 0x14);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x0D+((ch%2)*0x80)+1, 0x00);  
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x05+((ch%2)*0x80), 0x07);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x0A+((ch%2)*0x80), 0x03);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x0B+((ch%2)*0x80), 0x10);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x2F+((ch%2)*0x80), 0x00);

	//Decoder RX Setting
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x05+(ch%4));
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x30, 0x00);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x31, 0x01);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x32, 0x64);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x7C, 0x11);
	if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x7D, 0x80);
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x7D, 0x80);
	}
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x03+((ch%4)/2));

	if(ch_mode_status[ch] == NVP6124_VI_720P_2530)
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x62+((ch%2)*0x80), nvp6124_mode==PAL? 0x05:0x06);	// Camera TX DATA Check
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x68+((ch%2)*0x80), 0x40);	// RX size
	}
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
	{
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x62+((ch%2)*0x80), 0x06);	// Camera TX DATA Check
		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x68+((ch%2)*0x80), 0x70);	// RX size
	}

	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x60+((ch%2)*0x80), 0x55);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x63+((ch%2)*0x80), 0x01);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x66+((ch%2)*0x80), 0x80);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x67+((ch%2)*0x80), 0x01);
	//printk("ACP DATA READ TEST!!!!CH%d  RESOL = %s\n\r",ch, ch_mode_status[ch]==NVP6124_VI_720P_2530?"720P":"1080P");
}

void nvp6124_each_mode_setting(nvp6124_video_mode *pvmode )
{  
	unsigned char tmp;
	unsigned char ch, chmode[16];
	unsigned char pn_value_sd_nt_comet[4] = {0x4D,0x0E,0x88,0x6C};
	unsigned char pn_value_720p_30[4] = 	{0xEE,0x00,0xE5,0x4E};
	unsigned char pn_value_720p_60[4] = 	{0x2C,0xF9,0xC5,0x52};
	unsigned char pn_value_fhd_nt[4] = 		{0x2C,0xF0,0xCA,0x52};
	unsigned char pn_value_sd_pal_comet[4] = {0x75,0x35,0xB4,0x6C};
	unsigned char pn_value_720p_25[4] = 	{0x46,0x08,0x10,0x4F};
	unsigned char pn_value_720p_50[4] = 	{0x2C,0xE7,0xCF,0x52};
	unsigned char pn_value_fhd_pal[4] = 	{0xC8,0x7D,0xC3,0x52};
	unsigned char vformat = pvmode->vformat[0];
	
  	for(ch=0;ch<(nvp6124_cnt*4);ch++)
  	{
		chmode[ch] = pvmode->chmode[ch];
  	}
	printk("\n\nchmode[0] %bu chmode[1] %bu chmode[2] %bu chmode[3] %bu \n\n", chmode[0],chmode[1],chmode[2],chmode[3]);
	
	for(ch=0;ch<(nvp6124_cnt*4);ch++)
	{
		//if((chmode[ch] != ch_mode_status[ch]) && (chmode[ch] < NVP6124_VI_BUTT)) 
		if(chmode[ch] < NVP6124_VI_BUTT) 
		{
			switch(chmode[ch])
			{
				case NVP6124_VI_SD:
				case NVP6124_VI_720H:
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x08+ch%4, vformat==PAL?0xDD:0xA0);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x0c+ch%4, nvp6124_bri_tbl_960H[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x10+ch%4, nvp6124_con_tbl_960H[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x14+ch%4, vformat==PAL?0x80:0x80);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x18+ch%4, vformat==PAL?0x18:0x18);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), vformat==PAL?0x12:0x92);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), 0x43);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x30+ch%4, vformat==PAL?0x12:0x11);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x3c+ch%4, nvp6124_sat_tbl_960H[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x40+ch%4, nvp6124_hue_tbl_960H[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x44+ch%4, vformat==PAL?0x00:0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x48+ch%4, vformat==PAL?0x00:0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x4c+ch%4, vformat==PAL?0x04:0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x50+ch%4, vformat==PAL?0x04:0x00);
					if(chmode[ch]==NVP6124_VI_SD)
						gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58+ch%4, vformat==PAL?0x60:0xC0);
					else
					    gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58+ch%4, vformat==PAL?0x70:0x70);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x5c+ch%4, vformat==PAL?0x1e:0x1e);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x64+ch%4, vformat==PAL?0x0d:0x08);
	 				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x81+ch%4, vformat==PAL?0x00:0x00);	
			 		gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x85+ch%4, vformat==PAL?0x11:0x11);
					if(chmode[ch]==NVP6124_VI_720H)
					{
						printk("ch %d setted to 720H %s\n", (int)ch, vformat==PAL?"PAL":"NTSC");
			 			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x89+ch%4, vformat==PAL?0x11:0x11);
			 			gpio_i2c_write(nvp6124_slave_addr[ch/4], ch%4+0x8E, vformat==PAL?0x00:0x00);
					}
					else
					{
						printk("ch %d setted to 960H %s\n", (int)ch, vformat==PAL?"PAL":"NTSC");
			 			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x89+ch%4, vformat==PAL?0x10:0x00);
			 			gpio_i2c_write(nvp6124_slave_addr[ch/4], ch%4+0x8E, vformat==PAL?0x08:0x07);
					}
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x93+ch%4, 0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x98+ch%4, vformat==PAL?0x07:0x04);
	 				gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xa0+ch%4, vformat==PAL?0x15:0x15);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xa4+ch%4, vformat==PAL?0x05:0x07);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x88+ch%4, vformat==PAL?0x06:0x06);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x8c+ch%4, vformat==PAL?0xa6:0xa6);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xd7, gpio_i2c_read(nvp6124_slave_addr[ch/4], 0xd7)&(~(1<<(ch%4))));
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x02);  //motion
					tmp = gpio_i2c_read( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x00<<((ch%2)*4)));
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table( nvp6124_slave_addr[ch/4], 0x00, NVP6124_B5678_SD_Buf, 254 );
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x06,0x40);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x0F,vformat==PAL?0x13:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x1B,0x08);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x20,0x88);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x1E,vformat==PAL?0x00:0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x2C,vformat==PAL?0x08:0x0C);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x35,vformat==PAL?0x17:0x15);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x62,0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA1,vformat==PAL?0x10:0x30);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA2,vformat==PAL?0x0E:0x0C);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA3,vformat==PAL?0x70:0x50);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA8,vformat==PAL?0x40:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAC,vformat==PAL?0x10:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAD,vformat==PAL?0x08:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAE,vformat==PAL?0x04:0x14);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xC0,vformat==PAL?0x0D:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x25,vformat==PAL?0xCA:0xDA);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x40+(ch%4), 0x60);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x44, gpio_i2c_read(nvp6124_slave_addr[ch/4], 0x44)&(~(1<<(ch%4))));
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[0]:pn_value_sd_nt_comet[0]);	//ch%41 960H	
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[1]:pn_value_sd_nt_comet[1]);		
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[2]:pn_value_sd_nt_comet[2]);		
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_sd_pal_comet[3]:pn_value_sd_nt_comet[3]);
					printk("ch %bu setted to SD %s\n", (int)ch, vformat==PAL?"PAL":"NTSC");
				break;	
				case NVP6124_VI_720P_2530:
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x08+ch%4,vformat==PAL?0x60:0x60);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x0c+ch%4,nvp6124_bri_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x10+ch%4,nvp6124_con_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x14+ch%4,vformat==PAL?0x90:0x90);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x18+ch%4,vformat==PAL?0x30:0x30);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), 0x92);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x22+4*(ch%4), 0x0A);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), 0x43); //half/ful frame
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x30+ch%4,0x15);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x3c+ch%4,nvp6124_sat_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x40+ch%4,nvp6124_hue_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x44+ch%4,vformat==PAL?0x30:0x30);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x48+ch%4,vformat==PAL?0x30:0x30);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x4c+ch%4,vformat==PAL?0x04:0x04);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x50+ch%4,vformat==PAL?0x04:0x04);
					nvp6124_set_fpc(ch, vformat);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x58+ch%4,vformat==PAL?0x80:0x90);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x5c+ch%4,vformat==PAL?0x9e:0x9e);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x64+ch%4,vformat==PAL?0xb1:0xb2);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x81+ch%4,vformat==PAL?0x07:0x06);	
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x85+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x89+ch%4,vformat==PAL?0x10:0x10);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], ch%4+0x8E,0x0d);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x93+ch%4,0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x98+ch%4,vformat==PAL?0x07:0x04);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xa0+ch%4,0x15);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xa4+ch%4,vformat==PAL?0x00:0x01);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF,0x01);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x88+ch%4, vformat==PAL?0x0A:0x0A);	
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x8c+ch%4, vformat==PAL?0x0A:0x0A);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x9E, vformat==PAL?0x55:0x55);	
					//gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xcc+ch_seq[ch%4],vformat==PAL?0x46:0x46);
					
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xd7,gpio_i2c_read( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x02);  //motion
					tmp = gpio_i2c_read( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x05<<((ch%2)*4)));
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table( nvp6124_slave_addr[ch/4], 0x00, NVP6124_B5678_Buf,254 );
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x01,0x0D); 
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x06,0x40); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x1E,vformat==PAL?0x00:0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x35,vformat==PAL?0x17:0x15);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x7A,vformat==PAL?0x00:0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x7B,vformat==PAL?0x00:0x81);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA1,vformat==PAL?0x10:0x30);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA2,vformat==PAL?0x0E:0x0C);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA3,vformat==PAL?0x70:0x50);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA8,vformat==PAL?0x40:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAC,vformat==PAL?0x10:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAD,vformat==PAL?0x08:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAE,vformat==PAL?0x04:0x14);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x25,0xDB);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x2B,0x78); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x59,0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58,0x13);
					
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xC0,0x16);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xC1, 0x13);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xC8, 0x04);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xD8,0x0C);    
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xD9,0x0E); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDA,0x12);    
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDB,0x14); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDC,0x1C);    
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDD,0x2C);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDE,0x34);
					
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x40+(ch%4),0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x44, 0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_720p_25[0]:pn_value_720p_30[0]);	//ch%41 960H	
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_720p_25[1]:pn_value_720p_30[1]);		
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_720p_25[2]:pn_value_720p_30[2]);		
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_720p_25[3]:pn_value_720p_30[3]);	
					printk("ch %bu setted to 720P %s\n", ch, vformat==PAL?"PAL":"NTSC");
				break;
				case NVP6124_VI_720P_5060:
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x08+ch%4,vformat==PAL?0x60:0x60);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x0c+ch%4,nvp6124_bri_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x10+ch%4,nvp6124_con_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x14+ch%4,vformat==PAL?0x90:0x90);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x18+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), 0x92);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x22+4*(ch%4), 0x0A);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), vformat==PAL?0x43:0x41);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x30+ch%4,vformat==PAL?0x12:0x12);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x3c+ch%4,nvp6124_sat_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x40+ch%4,nvp6124_hue_tbl_720P[vformat%2]);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x44+ch%4,vformat==PAL?0x30:0x30);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x48+ch%4,vformat==PAL?0x30:0x30);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x4c+ch%4,vformat==PAL?0x04:0x04);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x50+ch%4,vformat==PAL?0x04:0x04);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x58+ch%4,vformat==PAL?0xc0:0xb0);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x5c+ch%4,vformat==PAL?0x9e:0x9e);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x64+ch%4,vformat==PAL?0xb1:0xb2);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x81+ch%4,vformat==PAL?0x05:0x04);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x85+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x89+ch%4,vformat==PAL?0x10:0x10);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], ch%4+0x8E,vformat==PAL?0x0b:0x09);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x93+ch%4,0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x98+ch%4,vformat==PAL?0x07:0x04);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xa0+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xa4+ch%4,vformat==PAL?0x00:0x01);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x01);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x88+ch%4,vformat==PAL?0x4d:0x4d);	
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x8c+ch%4,vformat==PAL?0x84:0x84);
					//gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xcc+ch_seq[ch%4],vformat==PAL?0x66:0x66);
					if(chip_id[0] == NVP6124_R0_ID)
						gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xcc+ch_seq[ch%4], 0x66);
					else if(chip_id[0] == NVP6114A_R0_ID && ch_seq[ch]!=0xFF)
						gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xcd+(ch_seq[ch%4]%2)*2, 0x66);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xd7, gpio_i2c_read( nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x02);  //motion
					tmp = gpio_i2c_read( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x05<<((ch%2)*4)));
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table( nvp6124_slave_addr[ch/4], 0x00, NVP6124_B5678_Buf, 254);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x01,0x0C);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x06,0x40);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x2B,0x78); 
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x59,0x01);  
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x24,vformat==PAL?0x2A:0x1A);  //sync changed
					//gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x47,vformat==PAL?0x04:0xEE); 
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x50,vformat==PAL?0x84:0x86);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xBB,vformat==PAL?0x00:0xE4);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0xFF,0x09);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x40+(ch%4), 0x00);
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x44, gpio_i2c_read( nvp6124_slave_addr[ch/4], 0x44)|(1<<(ch%4)));
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_720p_50[0]:pn_value_720p_60[0]);	//ch%41 960H	
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_720p_50[1]:pn_value_720p_60[1]);		
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_720p_50[2]:pn_value_720p_60[2]);		
					gpio_i2c_write( nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_720p_50[3]:pn_value_720p_60[3]);
					printk("ch %bu setted to 720P@RT %s\n", ch, vformat==PAL?"PAL":"NTSC");
				break;		
				case NVP6124_VI_1080P_2530:
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x08+ch%4,vformat==PAL?0x60:0x60);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x0c+ch%4,nvp6124_bri_tbl[vformat%2]);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x10+ch%4,nvp6124_con_tbl[vformat%2]);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x14+ch%4,vformat==PAL?0x90:0x90);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x18+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x21+4*(ch%4), 0x92);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x22+4*(ch%4), 0x0B);
			
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x23+4*(ch%4), 0x43);//0x83
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x30+ch%4,0x15);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x3c+ch%4,nvp6124_sat_tbl[vformat%2]);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x40+ch%4,nvp6124_hue_tbl[vformat%2]);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x44+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x48+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x4c+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50+ch%4,vformat==PAL?0x00:0x00);
					//nvp6124_set_fpc(ch, vformat);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x54,vformat==PAL?0xF0:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58+ch%4,vformat==PAL?0x78:0x57);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x5c+ch%4,vformat==PAL?0x9e:0x9e);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x64+ch%4,vformat==PAL?0xbf:0x8d);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x81+ch%4,vformat==PAL?0x03:0x02);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x85+ch%4,vformat==PAL?0x00:0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x89+ch%4,vformat==PAL?0x10:0x10);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], ch%4+0x8E,vformat==PAL?0x0a:0x09);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x93+ch%4,0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x98+ch%4,vformat==PAL?0x07:0x04);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xa0+ch%4,0x15);	
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xa4+ch%4,vformat==PAL?0x00:0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x01);
					
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x88+ch%4, vformat==PAL?0x00:0x00);//NVP6124A NYH	
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x8c+ch%4, vformat==PAL?0x42:0x42);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xd7, gpio_i2c_read(nvp6124_slave_addr[ch/4], 0xd7)|(1<<(ch%4)));
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x02);  
					tmp = gpio_i2c_read(nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x16+(ch%4)/2, (tmp&(ch%2==0?0xF0:0x0F))|(0x05<<((ch%2)*4)));
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x05+ch%4);
					nvp6124_write_table(nvp6124_slave_addr[ch/4], 0x00, NVP6124_B5678_Buf,254 );
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x01,0x0C);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x06,0x40);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x1E,vformat==PAL?0x00:0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x35,vformat==PAL?0x17:0x15);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x7A,vformat==PAL?0x00:0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x7B,vformat==PAL?0x00:0x81);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA1,vformat==PAL?0x10:0x30);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA2,vformat==PAL?0x0E:0x0C);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA3,vformat==PAL?0x70:0x50);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xA8,vformat==PAL?0x40:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAC,vformat==PAL?0x10:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAD,vformat==PAL?0x08:0x20);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xAE,vformat==PAL?0x04:0x14);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x2A,0x72);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x2B,0xA8); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x58,0x13);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x59,0x01);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xC0, 0x16); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xC1, 0x13);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xC8, 0x04); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x47, 0xEE); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50, 0xC4); 					
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xD8,0x10);    
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xD9,0x1F); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDA,0x2B);    
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDB,0x7F); 
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDC,0xFF);    
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDD,0xFF);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xDE,0xFF);
					
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x24,vformat==PAL?0x2A:0x1A);  
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50,vformat==PAL?0x84:0x86);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xBB,vformat==PAL?0x00:0xE4);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x40+(ch%4), 0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x44, 0x00);
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x50+4*(ch%4),vformat==PAL?pn_value_fhd_pal[0]:pn_value_fhd_nt[0]);		
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x51+4*(ch%4),vformat==PAL?pn_value_fhd_pal[1]:pn_value_fhd_nt[1]);		
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x52+4*(ch%4),vformat==PAL?pn_value_fhd_pal[2]:pn_value_fhd_nt[2]);		
					gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x53+4*(ch%4),vformat==PAL?pn_value_fhd_pal[3]:pn_value_fhd_nt[3]);	
					printk("ch %bu setted to 1080P %s\n", ch, vformat==PAL?"PAL":"NTSC");
				break;
				default:
					printk("ch%d wrong mode detected!!!\n", ch);
					break;
			}
			
			ch_mode_status[ch] = chmode[ch];
			nvp6124b_syncchange(ch, pvmode);
			//TODO
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x09);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x64, 0x18);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x65, 0xC2);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x66, 0x01);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x67, 0x1E);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x68, 0x02);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x69, 0x64);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x6A, 0x60);
			gpio_i2c_write(nvp6124_slave_addr[ch/4], 0x6B, 0x3C);
			acp_each_setting(ch);
		}	
	}
}



void nvp6124_video_set_contrast(unsigned int ch, unsigned int value, unsigned int v_format)
{	
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(value >= 100)
	{
		if(ch_mode_status[ch] == NVP6124_VI_SD)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_960H[v_format]+value-100));
		else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl[v_format]+value-100));
		else 
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_720P[v_format]+value-100));
	}
	else
	{
		if(ch_mode_status[ch] == NVP6124_VI_SD)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_960H[v_format]+(0xff-(98-value))));
		else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl[v_format]+(0xff-(98-value))));
		else
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x10+(ch%4)),(nvp6124_con_tbl_720P[v_format]+(0xff-(98-value))));
	}
}

void nvp6124_video_set_brightness(unsigned int ch, unsigned int value, unsigned int v_format)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(value >= 100)
	{
		if(ch_mode_status[ch] == NVP6124_VI_SD)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_960H[v_format]+value-100));
		else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl[v_format]+value-100));
		else
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_720P[v_format]+value-100));
	}	
	else
	{
		if(ch_mode_status[ch] == NVP6124_VI_SD)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_960H[v_format]+(0xff-(98-value))));
		else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl[v_format]+(0xff-(98-value))));
		else
			gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x0c+(ch%4)),(nvp6124_bri_tbl_720P[v_format]+(0xff-(98-value))));
	}	
}

void nvp6124_video_set_saturation(unsigned int ch, unsigned int value, unsigned int v_format)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(ch_mode_status[ch] == NVP6124_VI_SD)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x3c+(ch%4)),(nvp6124_sat_tbl_960H[v_format]+value-100));
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x3c+(ch%4)),(nvp6124_sat_tbl[v_format]+value-100));
	else
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x3c+(ch%4)),(nvp6124_sat_tbl_720P[v_format]+value-100));
}

void nvp6124_video_set_hue(unsigned int ch, unsigned int value, unsigned int v_format)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	if(ch_mode_status[ch] == NVP6124_VI_SD)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x40+(ch%4)), (nvp6124_hue_tbl_960H[v_format]+value-100));
	else if(ch_mode_status[ch] == NVP6124_VI_1080P_2530)
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x40+(ch%4)), (nvp6124_hue_tbl[v_format]+value-100));
	else
		gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x40+(ch%4)), (nvp6124_hue_tbl_720P[v_format]+value-100));
}

void nvp6124_video_set_sharpness(unsigned int ch, unsigned int value)
{
	gpio_i2c_write(nvp6124_slave_addr[ch/4], 0xFF, 0x00);
	gpio_i2c_write(nvp6124_slave_addr[ch/4], (0x14+(ch%4)), (0x90+value-100));
}

void nvp6124_write_table(unsigned char chip_addr, unsigned char addr, unsigned char *tbl_ptr, unsigned char tbl_cnt)
{
	unsigned char i = 0;
	
	for(i = 0; i < tbl_cnt; i ++)
	{
		gpio_i2c_write(chip_addr, (addr + i), *(tbl_ptr + i));
	}
}

