#ifndef NVP6124_VIDEO_HI_
#define NVP6124_VIDEO_HI_

#include "nvp6124.h"

//#define gpio_i2c_write i2cm_wr_byte
//#define msleep twdDelay
#define printk  printf_tw

extern unsigned char nvp6124_difinition;

unsigned char gpio_i2c_read(unsigned char addr, unsigned char offset);
unsigned char gpio_i2c_write(unsigned char slave, unsigned char addr, unsigned char datas);
void msleep(unsigned short ms);
void nvp6124_common_init(void);
void nvp6124_outport_1mux_chseq(void);
void nvp6124_outport_2mux(unsigned char vformat, unsigned char port1_mode, unsigned char port2_mode);
void nvp6124_outport_4mux(unsigned char vformat, unsigned char port1_mode);
void nvp6114a_outport_1mux_chseq(void);
void nvp6114a_outport_2mux(unsigned char vformat, unsigned char port1_mode, unsigned char port2_mode);
void nvp6114a_outport_4mux(unsigned char vformat, unsigned char port1_mode);


void nvp6124_each_mode_setting(nvp6124_video_mode *pvmode );
void video_fmt_det(nvp6124_input_videofmt *pvideofmt);
unsigned int nvp6124_getvideoloss(void);
void nvp6124_syncchange(void);
void nvp6124_set_equalizer(void);

void nvp6124_video_set_contrast(unsigned int ch, unsigned int value, unsigned int v_format);
void nvp6124_video_set_brightness(unsigned int ch, unsigned int value, unsigned int v_format);
void nvp6124_video_set_saturation(unsigned int ch, unsigned int value, unsigned int v_format);
void nvp6124_video_set_hue(unsigned int ch, unsigned int value, unsigned int v_format);
void nvp6124_video_set_sharpness(unsigned int ch, unsigned int value);
void software_reset(void);
void video_fmt_det(nvp6124_input_videofmt *pvideofmt);
unsigned int nvp6124_getvideoloss(void);
void nvp6124_datareverse(void);

enum{
    NTSC_MODE=0,
	PAL_MODE,
};

enum
{  
	NVP6124_VI_SD = 0,
	NVP6124_VI_720H,			//720x576i(480)
	NVP6124_VI_720P_2530,
	NVP6124_VI_720P_5060,
	NVP6124_VI_1080P_2530,
	NVP6124_VI_BUTT
};
typedef enum _nvp6124_outmode_sel
{
	NVP6124_OUTMODE_1MUX_SD = 0,
	NVP6124_OUTMODE_1MUX_HD,
	NVP6124_OUTMODE_1MUX_HD5060,
	NVP6124_OUTMODE_1MUX_FHD,
	NVP6124_OUTMODE_2MUX_SD,
	NVP6124_OUTMODE_2MUX_HD_X,
	NVP6124_OUTMODE_2MUX_HD,
	NVP6124_OUTMODE_2MUX_FHD_X,
	NVP6124_OUTMODE_4MUX_SD,
	NVP6124_OUTMODE_4MUX_HD_X,
	NVP6124_OUTMODE_4MUX_HD,
	NVP6124_OUTMODE_2MUX_FHD,
	NVP6124_OUTMODE_1MUX_HD_X,   //nvp6124b
	NVP6124_OUTMODE_1MUX_FHD_X,
	NVP6124_OUTMODE_4MUX_FHD_X,
	NVP6124_OUTMODE_4MUX_MIX,
	NVP6124_OUTMODE_2MUX_MIX,
	NVP6124_OUTMODE_BUTT
}NVP6124_OUTMODE_SEL;
#endif


