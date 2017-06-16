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
#include "common.h"

unsigned char chip_id[4];
unsigned char nvp6124_mode = 1;  //0:ntsc, 1: pal
unsigned char nvp6124_difinition = NVP6124_VI_720P_2530;
unsigned char nvp6124_mode_bak = 0xFF;
unsigned char nvp6124_cnt = 0;//source variable
unsigned char channel = 0;//current channel no.
const unsigned char nvp6124_slave_addr[4] = {0x60, 0x62, 0x64, 0x66};
extern unsigned char ch_mode_status[16];
//extern void nvp6124_datareverse(void);

unsigned char check_id(unsigned char dec)
{
	unsigned char ret;
	gpio_i2c_write(dec, 0xFF, 0x00);
	ret = gpio_i2c_read(dec, 0xf4);
	return ret;
}

//extern unsigned char g_coax_ch;
unsigned char vloss=0xFFFF;


void nvp6124_module_init(unsigned char mode)
{
	int ret = 0, i = 0;
//    unsigned char i2c_addr;
	nvp6124_video_mode vmode;
	

	nvp6124_mode = mode;

	printk("NVP6124&NVP6114A Test Driver 2015.3.16\n");

	for(i=0;i<4;i++)
	{
		chip_id[i] = check_id(nvp6124_slave_addr[i]);
		if( (chip_id[i] != NVP6124_R0_ID ) && (chip_id[i] != NVP6114A_R0_ID ) )
		{
			printk("nvp6124/6114a Device ID Error... %x\n",(unsigned short)chip_id[i]);
		}
		else
		{
			printk("nvp6124/6114a Device (0x%x) ID OK... %x\n", (unsigned short)nvp6124_slave_addr[i], (unsigned short)chip_id[i]);
			nvp6124_cnt++;
		}
	}

	software_reset();
	

	printk("NVP6124 Count = %x\n", (unsigned short)nvp6124_cnt);
	
	//if(nvp6124_mode%2 == 0)
		nvp6124_common_init();
	//else
		//nvp6124_pal_common_init();
	for(i=0;i<nvp6124_cnt*4;i++)
	{
		vmode.vformat[0] = nvp6124_mode%2;
		vmode.chmode[i] = NVP6124_VI_1080P_2530;  //默认设置为1080P模式.
	}
	if(chip_id[0] == NVP6124_R0_ID )
		nvp6124_each_mode_setting(&vmode);
	else if(chip_id[0] == NVP6114A_R0_ID )
		nvp6114a_outport_2mux(nvp6124_mode%2, 0x00|NVP6124_VI_720P_2530, 0x10|NVP6124_VI_720P_2530);
	nvp6124_datareverse();
}
#if 0
void nvp6114a_outport_1mux_chseq_mode(unsigned char vformat,unsigned char mode)
{
	int i = 0; 
	int ch = 0;
	nvp6124_video_mode vmode;
	unsigned char ch_seq[4]={2,1,0,3};
	ch_seq[0] = 1;   // port A outputs channel0's video data
	ch_seq[1] = 0;   // port B outputs channel1's video data
	ch_seq[2] = 0xFF;
	ch_seq[3] = 0xFF;
	if(vformat%2 == 0)
		nvp6124_ntsc_common_init();
	else
		nvp6124_pal_common_init();
	for(ch=0;ch < 4;ch++)
  	{
  		vmode.vformat[i] = vformat;
		vmode.chmode[ch] = mode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<1;i++)
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
	printk("nvp6114a_outport_1mux_chseq_mode\n");
}
#endif
/*
chip:chip select[0,1,2,3];
portsel: port select[0,1];
portmode: port mode select[1mux,2mux,4mux]
chid:  channel id, 1mux[0,1,2,3], 2mux[0,1], 4mux[0]
NOTE: 
portsel == 0,对应芯片硬件VDO1,寄存器描述VPORT_2;
portsel == 1,对应芯片硬件VDO2,寄存器描述VPORT_1;
*/
int nvp6124b_set_portmode(unsigned char chip, unsigned char portsel, unsigned char portmode, unsigned char chid)
{
    unsigned char chipaddr = nvp6124_slave_addr[chip];
	unsigned char tmp=0, reg1=0, reg2=0;
	
    if(portsel>1)
	{
		printk("nvp6124b_set_portmode portsel[%d] error!!!\n", (int)portsel);
		return -1;
	}
	switch(portmode)
	{
		case NVP6124_OUTMODE_1MUX_SD:
			/*输出720H或者960H单通道,数据37.125MHz,时钟37.125MHz,单沿采样.*/
			gpio_i2c_write(chipaddr, 0xFF, 0x00);
			gpio_i2c_write(chipaddr, 0x56, 0x10);
			gpio_i2c_write(chipaddr, 0xFF, 0x01);
			gpio_i2c_write(chipaddr, 0x90+portsel, 0x03);
			gpio_i2c_write(chipaddr, 0x92+portsel, 0x03);
			gpio_i2c_write(chipaddr, 0xA0+chid, 0x00);
			gpio_i2c_write(chipaddr, 0xC0+portsel*2, (chid<<4)|chid);  
			tmp = gpio_i2c_read(chipaddr, 0xC8) & (portsel%2?0x0F:0xF0);
			gpio_i2c_write(chipaddr, 0xC8, tmp);
			gpio_i2c_write(chipaddr, 0xC8+portsel, 0x00);
			gpio_i2c_write(chipaddr, 0xCF-(portsel<<1), 0x86);			
		break;
		case NVP6124_OUTMODE_1MUX_HD:
			/*输出720P或者1280H或者1440H单通道,数据74.25MHz,时钟74.25MHz,单沿采样.*/
			gpio_i2c_write(chipaddr, 0xFF, 0x00);
			gpio_i2c_write(chipaddr, 0x56, 0x10);
			gpio_i2c_write(chipaddr, 0xFF, 0x01);
			gpio_i2c_write(chipaddr, 0x90+portsel, 0x01);
			gpio_i2c_write(chipaddr, 0x92+portsel, 0x01);
			gpio_i2c_write(chipaddr, 0xA0+chid, 0x00);
			gpio_i2c_write(chipaddr, 0xC0+portsel*2, (chid<<4)|chid);    
			tmp = gpio_i2c_read(chipaddr, 0xC8) & (portsel%2?0x0F:0xF0);
			gpio_i2c_write(chipaddr, 0xC8, tmp);
			gpio_i2c_write(chipaddr, 0xCF-(portsel<<1), 0x16);   
		break;
		case NVP6124_OUTMODE_1MUX_HD5060:
		case NVP6124_OUTMODE_1MUX_FHD:	
			/*输出720P@5060或者1080P单通道,数据148.5MHz,时钟148.5MHz,单沿采样.*/
			gpio_i2c_write(chipaddr, 0xFF, 0x00);
			gpio_i2c_write(chipaddr, 0x56, 0x10);
			gpio_i2c_write(chipaddr, 0xFF, 0x01);
			gpio_i2c_write(chipaddr, 0x90+portsel, 0x01);
			gpio_i2c_write(chipaddr, 0x92+portsel, 0x01);
			gpio_i2c_write(chipaddr, 0xA0+chid, 0x01);
			gpio_i2c_write(chipaddr, 0xC0+portsel*2, (chid<<4)|chid);    
			tmp = gpio_i2c_read(chipaddr, 0xC8) & (portsel%2?0x0F:0xF0);
			gpio_i2c_write(chipaddr, 0xC8, tmp);
			gpio_i2c_write(chipaddr, 0xCF-(portsel<<1), 0x43);
			break;
		default:
			printk("portmode %d not supported yet\n", (int)portmode);
			break;		
  	}
	printk("nvp6124b_set_portmode portsel %d portmode %d setting\n", (int)portsel, (int)portmode);
	return 0;		
}
void nvp6114a_outport_1mux_chseq_mode(unsigned char vformat,unsigned char mode)
{
	int i = 0; 
	int ch = 0;
	//unsigned char ch_seq[4]={2,1,0,3};
	int ret = 0;
	
	nvp6124_video_mode vmode;

	nvp6124_mode = vformat;
	nvp6124_difinition = mode;
    nvp6124_cnt = 0;
	//vmode = vformat;
	
	printk("NVP6124&NVP6114A Test Driver 2015.3.16\n");

	for(i=0;i<4;i++)
	{
		chip_id[i] = check_id(nvp6124_slave_addr[i]);
		if( (chip_id[i] != NVP6124_R0_ID ) && (chip_id[i] != NVP6114A_R0_ID) 
			&& (chip_id[i] != NVP6124B_R0_ID ))
		{
			printk("nvp6124/6114a Device ID Error... %x\n",(unsigned short)chip_id[i]);
		}
		else
		{
			printk("nvp6124/6114a Device (0x%x) ID OK... %x\n", (unsigned short)nvp6124_slave_addr[i], (unsigned short)chip_id[i]);
			nvp6124_cnt++;
		}
	}
	//software_reset();
	
	printk("NVP6124 Count = %x\n", (unsigned short)nvp6124_cnt);
	
	//if(vformat%2 == 0)
	nvp6124_common_init(); //init
	//else
		//nvp6124_pal_common_init();
	for(ch=0;ch < 4;ch++)
  	{
  		vmode.vformat[0] = nvp6124_mode%2;
		vmode.chmode[ch] = mode;
	}
	nvp6124_each_mode_setting(&vmode);
	for(i=0;i<1;i++)
	{
	    nvp6124b_set_portmode(i, 0, mode-1, channel);
		nvp6124b_set_portmode(i, 1, mode-1, channel);
	}
	nvp6124_datareverse();//init
	printk("nvp6114a_outport_1mux_chseq_mode\n");
}

