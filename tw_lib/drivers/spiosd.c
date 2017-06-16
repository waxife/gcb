/**
 *  @file   spiosd.c
 *  @brief  terawins SPIOSD control function
 *  $Id: spiosd.c,v 1.18 2013/06/06 10:17:35 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.18 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#include "sys.h"
#include "reg_tw.h"
#include "iccontrol.h"
#include "cq.h"
#include "spiosd.h"

#define USE_CQ_WRITE
//#define DEBUG_SOSD

#define ENABLE	1
#define DISABLE	0

static unsigned char SOSD_IS_ENABLE= DISABLE;

static unsigned short HSTART_OFFSET;
static unsigned short VSTART_OFFSET;
static unsigned short SHSTART_OFFSET;
static unsigned short SVSTART_OFFSET;

/*
 * Synopsis    void spiosd_init( unsigned short h_offset, 
 *                               unsigned short v_offset, 
 *                               unsigned short s_h_offset, 
 *                               unsigned short s_v_offset);
 * Description  spiosd初始化
 * Parameters   h_offset            - spiosd X軸起始位置
 *              v_offset            - spiosd Y軸起始位置
 *              s_h_offset          - sprite X軸起始位置
 *              s_v_offset          - sprite Y軸起始位置
 * Return       none
 */
void spiosd_init(unsigned short h_offset, unsigned short v_offset, unsigned short s_h_offset, unsigned short s_v_offset)
{
	/* sOSD Disable */
	sosd_disable();

    sosd_td_spilt_mode_disable();
	spiosd_quad_mode_enable();          // oSPI 4x
	
	/* set dPLL sOSD 2x */
#ifndef SPIOSD_DCLK_1X
	IC_WritByte(TWIC_P0, 0xC4, IC_ReadByte(TWIC_P0, 0xC4)|0x20);		// dPLL_sOSD :1  SPI_OSD clk : dclko_2x = 2: 1
#else
	IC_WritByte(TWIC_P0, 0xC4, IC_ReadByte(TWIC_P0, 0xC4)&(~0x20));		// SPI_OSD clk 1x
#endif

	/* sOSD DMA Control */
	sosd_dma_ctrl();

	HSTART_OFFSET=h_offset;
	VSTART_OFFSET=v_offset;
	SHSTART_OFFSET=s_h_offset;
	SVSTART_OFFSET=s_v_offset;

	DBG_PRINT("\n\r");
	DBG_PRINT("SPIOSD Init\n\r");
}

/*
 * Synopsis    void spiosd_quad_mode_enable(void);
 * Description  啟用4bit模式 
 * Return       none
 */
void spiosd_quad_mode_enable(void)
{
    IC_WritByte(TWIC_P6, OSPI_COMMAND_REG, OSPI_4X);
}

/*
 * Synopsis    void spiosd_quad_mode_disable(void);
 * Description 停用4bit模式
 * Return       none
 */
void spiosd_quad_mode_disable(void)
{
    IC_WritByte(TWIC_P6, OSPI_COMMAND_REG, (IC_ReadByte(TWIC_P6, OSPI_COMMAND_REG) & ~OSPI_4X));
}


/* set sOSD H/V start offset */

/*
 * Synopsis    void sosd_hvoffset_init( unsigned short h_offset, 
 *                                      unsigned short v_offset);
 * Description  spiosd初始化
 * Parameters   h_offset            - spiosd X軸起始位置
 *              v_offset            - spiosd Y軸起始位置
 * Return       none
 */
void sosd_hvoffset_init(unsigned short h_offset, unsigned short v_offset)
{
	HSTART_OFFSET=h_offset;
	VSTART_OFFSET=v_offset;
}

/* set sOSD Sprite V start offset */
/*
 * Synopsis    void sosd_sp_hvoffset_init( unsigned short h_offset, 
                                           unsigned short v_offset);
 * Description  sprite初始化
 * Parameters   h_offset            - sprite X軸起始位置
 *              v_offset            - sprite Y軸起始位置
 * Return       none
 */
void sosd_sp_hvoffset_init(unsigned short h_offset, unsigned short v_offset)
{
	SHSTART_OFFSET=h_offset;
	SVSTART_OFFSET=v_offset;
}

/*
 * Synopsis    void _sosd_emu_addr(unsigned long base_address);
 * Description  設定emulation地址
 * Parameters   base_address        - emulation資料地址
 * Return       none
 */
void 
_sosd_emu_addr(unsigned long base_address)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_EMU_ADDR_REG1, (base_address&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_EMU_ADDR_REG2, ((base_address>>8)&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_EMU_ADDR_REG3, ((base_address>>16)&0x000000FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else	
	IC_WritByte(TWIC_P3, SOSD_EMU_ADDR_REG1, (base_address&0x000000FF));		// AL
	IC_WritByte(TWIC_P3, SOSD_EMU_ADDR_REG2, ((base_address>>8)&0x000000FF));	// AM
	IC_WritByte(TWIC_P3, SOSD_EMU_ADDR_REG3, ((base_address>>16)&0x000000FF));	// AH
#endif
#ifdef DEBUG_SOSD   
    dbg(0, ("sosd_emu_addr 0x%lX \n", base_address));
#endif
}

/*
 * Synopsis    void _sosd_img_addr(unsigned long base_address);
 * Description  設定img地址
 * Parameters   base_address        - img資料地址
 * Return       none
 */
void 
_sosd_img_addr(unsigned long base_address)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_IMG_ADDR_REG1, (base_address&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_IMG_ADDR_REG2, ((base_address>>8)&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_IMG_ADDR_REG3, ((base_address>>16)&0x000000FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_IMG_ADDR_REG1, (base_address&0x000000FF));		// AL
	IC_WritByte(TWIC_P3, SOSD_IMG_ADDR_REG2, ((base_address>>8)&0x000000FF));	// AM
	IC_WritByte(TWIC_P3, SOSD_IMG_ADDR_REG3, ((base_address>>16)&0x000000FF));	// AH
#endif
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_img_addr 0x%lX \n", base_address));
#endif
}

/*
 * Synopsis    void _sosd_spr_addr(unsigned long base_address);
 * Description  設定spr地址
 * Parameters   base_address        - spr資料地址
 * Return       none
 */
void 
_sosd_spr_addr(unsigned long base_address)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SPRITE_ADDR_REG1, (base_address&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_SPRITE_ADDR_REG2, ((base_address>>8)&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_SPRITE_ADDR_REG3, ((base_address>>16)&0x000000FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_SPRITE_ADDR_REG1, (base_address&0x000000FF));			// AL
	IC_WritByte(TWIC_P3, SOSD_SPRITE_ADDR_REG2, ((base_address>>8)&0x000000FF));	// AM
	IC_WritByte(TWIC_P3, SOSD_SPRITE_ADDR_REG3, ((base_address>>16)&0x000000FF));	// AH
#endif    
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_spr_addr 0x%lX \n", base_address));
#endif
}

/*
 * Synopsis    void _sosd_pLUT_addr(unsigned long base_address);
 * Description  設定spiosd lut地址
 * Parameters   base_address        - spiosd lut資料地址
 * Return       none
 */
void 
_sosd_pLUT_addr(unsigned long base_address)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_LUT_ADDR_REG1, (base_address&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_LUT_ADDR_REG2, ((base_address>>8)&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_LUT_ADDR_REG3, ((base_address>>16)&0x000000FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_LUT_ADDR_REG1, (base_address&0x000000FF));		// AL
	IC_WritByte(TWIC_P3, SOSD_LUT_ADDR_REG2, ((base_address>>8)&0x000000FF));	// AM
	IC_WritByte(TWIC_P3, SOSD_LUT_ADDR_REG3, ((base_address>>16)&0x000000FF));	// AH
#endif
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_pLUT_addr 0x%lX \n", base_address));
#endif
}

/*
 * Synopsis    void _sosd_sLUT_addr(unsigned long base_address);
 * Description  設定sprite lut地址
 * Parameters   base_address        - sprite lut資料地址
 * Return       none
 */
void 
_sosd_sLUT_addr(unsigned long base_address)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SLUT_ADDR_REG1, (base_address&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_SLUT_ADDR_REG2, ((base_address>>8)&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_SLUT_ADDR_REG3, ((base_address>>16)&0x000000FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_SLUT_ADDR_REG1, (base_address&0x000000FF));			// AL
	IC_WritByte(TWIC_P3, SOSD_SLUT_ADDR_REG2, ((base_address>>8)&0x000000FF));		// AM
	IC_WritByte(TWIC_P3, SOSD_SLUT_ADDR_REG3, ((base_address>>16)&0x000000FF));		// AH
#endif
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_sLUT_addr 0x%lX \n", base_address));
#endif
}

/*
 * Synopsis    void _sosd_hspos(unsigned short h_pos);
 * Description  設定spiosd x軸位置
 * Parameters   h_pos        - spiosd x軸位置
 * Return       none
 */
void 
_sosd_hspos(unsigned short h_pos)
{	
#ifdef USE_CQ_WRITE
    char rc=0;
	h_pos*=2;
	h_pos+=HSTART_OFFSET;
	rc = cq_write_byte (TWIC_P3, SOSD_HSTAR_REG1, (h_pos&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_HSTAR_REG2, ((h_pos&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	h_pos*=2;
	h_pos+=HSTART_OFFSET;
	IC_WritByte(TWIC_P3, SOSD_HSTAR_REG1, (h_pos&0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_HSTAR_REG2, ((h_pos&0x7FF)>>8));		// H
#endif
}

/*
 * Synopsis    void _sosd_vspos(unsigned short v_pos);
 * Description  設定spiosd y軸位置
 * Parameters   v_pos        - spiosd y軸位置
 * Return       none
 */
void 
_sosd_vspos(unsigned short v_pos)
{	
#ifdef USE_CQ_WRITE
    char rc=0;
	v_pos+=VSTART_OFFSET;
	rc = cq_write_byte (TWIC_P3, SOSD_VSTAR_REG1, (v_pos&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_VSTAR_REG2, ((v_pos&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	v_pos+=VSTART_OFFSET;
	IC_WritByte(TWIC_P3, SOSD_VSTAR_REG1, (v_pos&0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_VSTAR_REG2, ((v_pos&0x3FF)>>8));		// H
#endif
}

/*
 * Synopsis    void _sosd_sp_hspos(unsigned short h_pos);
 * Description  設定spr x軸位置
 * Parameters   h_pos        - spr x軸位置
 * Return       none
 */
void 
_sosd_sp_hspos(unsigned short h_pos)
{	
#ifdef USE_CQ_WRITE
    char rc=0;
	h_pos+=SHSTART_OFFSET;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_HSTAR_REG1, (h_pos&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_SP_HSTAR_REG2, ((h_pos&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	h_pos+=SHSTART_OFFSET;
	IC_WritByte(TWIC_P3, SOSD_SP_HSTAR_REG1, (h_pos&0xFF));				// L
	IC_WritByte(TWIC_P3, SOSD_SP_HSTAR_REG2, ((h_pos&0xFFF)>>8));		// H
#endif
}

/*
 * Synopsis    void _sosd_sp_vspos(unsigned short v_pos);
 * Description  設定spr y軸位置
 * Parameters   v_pos        - spr y軸位置
 * Return       none
 */
void 
_sosd_sp_vspos(unsigned short v_pos)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	v_pos+=SVSTART_OFFSET;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_VSTAR_REG1, (v_pos&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_SP_VSTAR_REG2, ((v_pos&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	v_pos+=SVSTART_OFFSET;
	IC_WritByte(TWIC_P3, SOSD_SP_VSTAR_REG1, (v_pos&0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_SP_VSTAR_REG2, ((v_pos&0x3FF)>>8));	// H
#endif
}

/*
 * Synopsis    void _sosd_img_loca( unsigned short x, 
 *                                  unsigned short y);
 * Description  設定img位置
 * Parameters   x            - img X軸位置
 *              y            - img Y軸位置
 * Return       none
 */
void 
_sosd_img_loca(unsigned short x, unsigned short y)
{
    _sosd_hspos(x);
    _sosd_vspos(y);
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_img_loca %u %u \n", x, y));
#endif
}

/*
 * Synopsis    void _sosd_spr_loca( unsigned short x, 
 *                                  unsigned short y);
 * Description  設定spr位置
 * Parameters   x            - spr X軸位置
 *              y            - spr Y軸位置
 * Return       none
 */
void 
_sosd_spr_loca(unsigned short x, unsigned short y)
{
    if(IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)&(SOSD_TD_EN))
		x += 4;
	_sosd_sp_hspos(x);
    _sosd_sp_vspos(y);
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_spr_loca %u %u \n", x, y));
#endif
}

/*
 * Synopsis    void _sosd_td_loca ( unsigned short x, 
 *                                  unsigned short y);
 * Description  設定td位置
 * Parameters   x            - td X軸位置
 *              y            - td Y軸位置
 * Return       none
 */
void
_sosd_td_loca(unsigned short x, unsigned short y)
{
#ifndef SPIOSD_DCLK_1X
    x+= 9;
	y+= 2;
#else
	x+= 16;
	y+= 2;
#endif

	_sosd_hspos(x);
    _sosd_vspos(y);
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_td_loca %u %u \n", x, y));
#endif
}

/*
 * Synopsis    void _sosd_img_window( unsigned short width, 
 *                                    unsigned short height);
 * Description  img顯示區域設定
 * Parameters   width             - img 顯示寬度
 *              height            - img 顯示高度
 * Return       none
 */
void 
_sosd_img_window(unsigned short width, unsigned short height)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_HSIZE_REG1, (width&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_HSIZE_REG2, ((width&0x7FF)>>8));
	rc = cq_write_byte (TWIC_P3, SOSD_VSIZE_REG1, (height&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_VSIZE_REG2, ((height&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_HSIZE_REG1, (width&0x00FF));				// L
	IC_WritByte(TWIC_P3, SOSD_HSIZE_REG2, ((width>>8)&0x00FF));			// H
	IC_WritByte(TWIC_P3, SOSD_VSIZE_REG1, (height&0x00FF));				// L
	IC_WritByte(TWIC_P3, SOSD_VSIZE_REG2, ((height>>8)&0x00FF));		// H
#endif
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_img_window %u, %u \n", width, height));
#endif
}

/*
 * Synopsis    void _sosd_spr_window( unsigned short width, 
 *                                    unsigned short height);
 * Description  spr顯示區域設定
 * Parameters   width             - spr 顯示寬度
 *              height            - spr 顯示高度
 * Return       none
 */
void 
_sosd_spr_window(unsigned char width, unsigned short height)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_HSIZE_REG, width);
	rc = cq_write_byte (TWIC_P3, SOSD_SP_VSIZE_REG1, (height&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_SP_VSIZE_REG2, ((height&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_SP_HSIZE_REG, width);
	IC_WritByte(TWIC_P3, SOSD_SP_VSIZE_REG1, (height&0x00FF));				// L
	IC_WritByte(TWIC_P3, SOSD_SP_VSIZE_REG2, ((height>>8)&0x00FF));			// H
#endif
#ifdef DEBUG_SOSD
    dbg(0, ("_sosd_spr_window %u, %u \n", width, height));
#endif
}

/*
 * Synopsis    void _sosd_td_window( unsigned short width, 
 *                                   unsigned short height);
 * Description  td顯示區域設定
 * Parameters   width             - td 顯示寬度
 *              height            - td 顯示高度
 * Return       none
 */
void 
_sosd_td_window(unsigned short width, unsigned short height)
{
#ifndef SPIOSD_DCLK_1X
	width += 19;
#else
	width += 11;
#endif
	_sosd_img_window(width, height);
}

/*
 * Synopsis    void _sosd_img_ls(unsigned short line_store); 
 * Description  img linejump設定
 * Parameters   line_store            - img linejump
 * Return       none
 */
void 
_sosd_img_ls(unsigned short line_store)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_LJUMP_A_REG1, (line_store&0x00FF));
	rc = cq_write_byte (TWIC_P3, SOSD_LJUMP_A_REG2, ((line_store>>8)&0x00FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_LJUMP_A_REG1, (line_store&0x00FF));		// L
	IC_WritByte(TWIC_P3, SOSD_LJUMP_A_REG2, ((line_store>>8)&0x00FF));	// H
#endif
#ifdef DEBUG_SOSD
    dbg(0, ("_sosd_img_ls %u \n", line_store));
#endif
}

/*
 * Synopsis    void _sosd_spr_ls(unsigned short line_store); 
 * Description  spr linejump設定
 * Parameters   line_store            - spr linejump
 * Return       none
 */
void 
_sosd_spr_ls(unsigned short line_store)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_LJUMP_A_REG1, (line_store&0x00FF));
	rc = cq_write_byte (TWIC_P3, SOSD_SP_LJUMP_A_REG2, ((line_store>>8)&0x00FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_SP_LJUMP_A_REG1, (line_store&0x00FF));        // L
	IC_WritByte(TWIC_P3, SOSD_SP_LJUMP_A_REG2, ((line_store>>8)&0x00FF));	// H
#endif
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_spr_ls %u \n", line_store));
#endif
}

/*
 * Synopsis    void _sosd_td_ls(unsigned short line_store); 
 * Description  td linejump設定
 * Parameters   line_store            - td linejump
 * Return       none
 */
void 
_sosd_td_ls(unsigned short line_store)
{
	if(line_store%4)
		line_store += (4-(line_store%4));
	line_store = line_store/4;
	_sosd_img_ls(line_store);
}

/* set sOSD emulation block of address  */
/*
 * Synopsis    void sosd_set_emu_base_addrss(unsigned long base_address);
 * Description  設定emulation地址並刷新
 * Parameters   base_address        - emulation資料地址
 * Return       none
 */
void sosd_set_emu_base_addrss(unsigned long base_address)
{
	_sosd_emu_addr(base_address);

	sosd_update();		
}

/* set sOSD image block of address  */
/*
 * Synopsis    void sosd_set_img_base_address(unsigned long base_address);
 * Description  設定img地址並刷新
 * Parameters   base_address        - img資料地址
 * Return       none
 */
void sosd_set_img_base_address(unsigned long base_address)
{
	_sosd_img_addr(base_address);

	sosd_update();
}

/* set sOSD sprite block of address  */
/*
 * Synopsis    void sosd_set_spr_base_address(unsigned long base_address);
 * Description  設定spr地址並刷新
 * Parameters   base_address        - spr資料地址
 * Return       none
 */
void sosd_set_spr_base_address(unsigned long base_address)
{
	_sosd_spr_addr(base_address);

	sosd_update();
}

/* set sOSD pLUT of address  */
/*
 * Synopsis    void sosd_set_pLUT_base_address(unsigned long base_address);
 * Description  設定spiosd lut地址並刷新
 * Parameters   base_address        - spiosd lut資料地址
 * Return       none
 */
void sosd_set_pLUT_base_address(unsigned long base_address)
{
	_sosd_pLUT_addr(base_address);

	sosd_update();
}

/* set sOSD sLUT of address  */
/*
 * Synopsis    void sosd_set_sLUT_base_address(unsigned long base_address);
 * Description  設定sprite lut地址並刷新
 * Parameters   base_address        - sprite lut資料地址
 * Return       none
 */
void sosd_set_sLUT_base_address(unsigned long base_address)
{
	_sosd_sLUT_addr(base_address);

	sosd_update();
}

/* set sOSD H start position  */
/*
 * Synopsis    void sosd_set_hspos(unsigned short h_pos);
 * Description  設定spiosd x軸位置並刷新
 * Parameters   h_pos        - spiosd x軸位置
 * Return       none
 */
void sosd_set_hspos(unsigned short h_pos)
{	
	_sosd_hspos(h_pos);

	sosd_update();
}

/* set sOSD V start position  */
/*
 * Synopsis    void sosd_set_vspos(unsigned short v_pos);
 * Description  設定spiosd y軸位置並刷新
 * Parameters   v_pos        - spiosd y軸位置
 * Return       none
 */
void sosd_set_vspos(unsigned short v_pos)
{	
	_sosd_vspos(v_pos);

	sosd_update();
}

/* set sOSD sprite H start position  */
/*
 * Synopsis    void sosd_set_sp_hspos(unsigned short h_pos);
 * Description  設定spr x軸位置並刷新
 * Parameters   h_pos        - spr x軸位置
 * Return       none
 */
void sosd_set_sp_hspos(unsigned short h_pos)
{	
	_sosd_sp_hspos(h_pos);

	sosd_update();
}

/* set sOSD sprite V start position  */
/*
 * Synopsis    void sosd_set_sp_vspos(unsigned short v_pos);
 * Description  設定spr y軸位置並刷新
 * Parameters   v_pos        - spr y軸位置
 * Return       none
 */
void sosd_set_sp_vspos(unsigned short v_pos)
{
	_sosd_sp_vspos(v_pos);

	sosd_update();
}

/*
 * Synopsis    void sosd_set_img_location( unsigned short x, 
 *                                         unsigned short y);
 * Description  設定img位置並刷新
 * Parameters   x            - img X軸位置
 *              y            - img Y軸位置
 * Return       none
 */
void sosd_set_img_location(unsigned short x, unsigned short y)
{
    _sosd_img_loca(x, y);

	sosd_update();
}

/*
 * Synopsis    void sosd_set_spr_location( unsigned short x, 
 *                                         unsigned short y);
 * Description  設定spr位置並刷新
 * Parameters   x            - spr X軸位置
 *              y            - spr Y軸位置
 * Return       none
 */
void sosd_set_spr_location(unsigned short x, unsigned short y)
{
    _sosd_spr_loca(x, y);

	sosd_update();
}

/*
 * Synopsis    void sosd_set_td_location( unsigned short x, 
 *                                        unsigned short y);
 * Description  設定td位置並刷新
 * Parameters   x            - td X軸位置
 *              y            - td Y軸位置
 * Return       none
 */
void sosd_set_td_location(unsigned short x, unsigned short y)
{
    _sosd_td_loca(x, y);

	sosd_update();
}

#if 0
void sosd_set_switch_tc(unsigned short stc)
{
	IC_WritByte(TWIC_P3, SOSD_AB0_REG, (stc & 0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_AB1_REG, ((stc&SOSD_SWITCH_TC_BIT)>>8));		// H
}
#endif

/* set sOSD display size  */
/*
 * Synopsis    void sosd_set_img_active_window( unsigned short width, 
 *                                              unsigned short height);
 * Description  img顯示區域設定並刷新
 * Parameters   width             - img 顯示寬度
 *              height            - img 顯示高度
 * Return       none
 */
void sosd_set_img_active_window(unsigned short width, unsigned short height)
{
    _sosd_img_window(width, height);

	sosd_update();
}

/* set sOSD sprite display size  */
/*
 * Synopsis    void sosd_set_spr_active_window( unsigned short width, 
 *                                              unsigned short height);
 * Description  spr顯示區域設定並刷新
 * Parameters   width             - spr 顯示寬度
 *              height            - spr 顯示高度
 * Return       none
 */
void sosd_set_spr_active_window(unsigned char width, unsigned short height)
{
    _sosd_spr_window(width, height);

	sosd_update();
}

/* set sOSD TD display size  */
/*
 * Synopsis    void sosd_set_td_active_window( unsigned short width, 
 *                                             unsigned short height);
 * Description  td顯示區域設定並刷新
 * Parameters   width             - td 顯示寬度
 *              height            - td 顯示高度
 * Return       none
 */
void sosd_set_td_active_window(unsigned short width, unsigned short height)
{
    _sosd_td_window(width, height);

	sosd_update();
}

/* set sOSD line jump size */
/*
 * Synopsis    void sosd_set_img_line_store(unsigned short line_store); 
 * Description  img linejump設定並刷新
 * Parameters   line_store            - img linejump
 * Return       none
 */
void sosd_set_img_line_store(unsigned short line_store)
{
    _sosd_img_ls(line_store);

	sosd_update();
}

/* set sOSD sprite line jump size */
/*
 * Synopsis    void sosd_set_spr_line_store(unsigned short line_store); 
 * Description  spr linejump設定並刷新
 * Parameters   line_store            - spr linejump
 * Return       none
 */
void sosd_set_spr_line_store(unsigned short line_store)
{
    _sosd_spr_ls(line_store);

	sosd_update();
}

/* set sOSD TD line jump size */
/*
 * Synopsis    void sosd_set_td_line_store(unsigned short line_store); 
 * Description  td linejump設定並刷新
 * Parameters   line_store            - td linejump
 * Return       none
 */
void sosd_set_td_line_store(unsigned short line_store)
{
    _sosd_td_ls(line_store);

	sosd_update();
}

/* sOSD DMA Control */
/*
 * Synopsis    void sosd_dma_ctrl(void);
 * Description  DMA設定
 * Return       none
 */
void sosd_dma_ctrl(void)
{
	IC_WritByte(TWIC_P6, OSPI_COMMAND_REG, 0x6D);		// Control
	IC_WritByte(TWIC_P3, OSPI_DMA_CTRL_REG1, OSPI_XFER_CNT | HS_LEAD_EDGE );	// DMA Control (but Trig by Video Timing)	
	IC_WritByte(TWIC_P6, SOSD_CLK_SEL_REG, OSPI_H_FREQ_CLK | 0x10);				// DMA Control (but Trig by Video Timing)	
}

/*
 * Synopsis    void _sosd_load_plut(void); 
 * Description  寫入spiosd lut
 * Return       none
 */
void _sosd_load_plut(void)
{
#ifdef USE_CQ_WRITE	
	IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_PSLUT|SOSD_SHADOW_UP);						// sOSD_En, Shadow, Load pLUT once
	if (cq_write_byte (TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_PSLUT))
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vde () < 0)
		ERROR (("cq_flush()\n"));
#else
	unsigned char count=0;		
	while(!(IC_ReadByte(TWIC_P3, SOSD_CONTROL_REG)&SOSD_TRIG_LOAD_PSLUT)) {
		IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_PSLUT|SOSD_SHADOW_UP);						// sOSD_En, Shadow, Load PLUT once
		count++;
		if(count==0xFF) {
			ERROR(("Load SPIOSD pLUT timeout!\n"));
			break;
		}			
	}
#endif
#ifdef DEBUG_SOSD		
		INFO(("Load SPIOSD pLUT!\n"));
#endif
}

/*
 * Synopsis    void _sosd_load_slut(void);
 * Description  寫入spr lut
 * Return       none
 */
void _sosd_load_slut(void)
{
#ifdef USE_CQ_WRITE	
	//IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_SSLUT|SOSD_SHADOW_UP);
	if (cq_write_byte (TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_SSLUT))
		ERROR (("cq_write_byte()\n"));
	//if (cq_flush_vsync () < 0)
	//	ERROR (("cq_flush()\n"));
#else
	unsigned char count=0;		
	while(!(IC_ReadByte(TWIC_P3, SOSD_CONTROL_REG)&SOSD_TRIG_LOAD_SSLUT)) {
		IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_SSLUT|SOSD_SHADOW_UP);						// sOSD_En, Shadow, Load sLUT once
		count++;
		if(count==255) {
			ERROR(("Load SPIOSD sLUT timeout!\n"));
			break;
		}			
	}	
#endif
#ifdef DEBUG_SOSD		
		INFO(("Load SPIOSD sLUT!\n"));
#endif
}

/*
 * Synopsis    void _sosd_load_pslut(void);
 * Description  寫入spiosd & spr lut
 * Return       none
 */
void _sosd_load_pslut(void)
{
#ifdef USE_CQ_WRITE	
	IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_PSLUT|SOSD_TRIG_LOAD_SSLUT|SOSD_SHADOW_UP);						// sOSD_En, Shadow, Load pLUT once
	if (cq_write_byte (TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_PSLUT|SOSD_TRIG_LOAD_SSLUT))
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vde () < 0)
		ERROR (("cq_flush()\n"));
#else
	unsigned char count=0;		
	while(!(IC_ReadByte(TWIC_P3, SOSD_CONTROL_REG)&SOSD_TRIG_LOAD_PSLUT)) {
		IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_LUT_ONCE|SOSD_TRIG_LOAD_PSLUT|SOSD_SHADOW_UP);						// sOSD_En, Shadow, Load PLUT once
		count++;
		if(count==0xFF) {
			ERROR(("Load SPIOSD pLUT timeout!\n"));
			break;
		}			
	}
#endif
#ifdef DEBUG_SOSD		
		INFO(("Load SPIOSD pLUT!\n"));
#endif
}

/* sOSD pLUT & sLUT Load */
/*
 * Synopsis    void sosd_pLUTsLUT_load(void);
 * Description  寫入spiosd & spr lut並設定參考變數
 * Return       none
 */
void sosd_pLUTsLUT_load(void)
{
#ifdef USE_CQ_WRITE
	_sosd_load_pslut();

	if (cq_write_byte (TWIC_P3, SOSD_CONTROL_REG, SOSD_EN))
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vde () < 0)
		ERROR (("cq_flush()\n"));
#else
	_sosd_load_plut();
	//IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_SHADOW_UP);		// sOSD_En, Shadow
#endif
	SOSD_IS_ENABLE= ENABLE;
}

/* sOSD Enable */
/*
 * Synopsis    void sosd_enable(unsigned char load);
 * Description  啟用spiosd
 * Parameters   load            - 載入lut
 * Return       none
 */
void sosd_enable(unsigned char load)
{
#ifdef USE_CQ_WRITE
	if(load)
		_sosd_load_plut();

	if (cq_write_byte (TWIC_P3, SOSD_CONTROL_REG, SOSD_EN))
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vde () < 0)
		ERROR (("cq_flush()\n"));
#else
	if(load)
		_sosd_load_plut();
	else
		IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_SHADOW_UP);		// sOSD_En, Shadow
#endif
	SOSD_IS_ENABLE= ENABLE;
}

/* sOSD Disable */
/*
 * Synopsis    void sosd_disable(void);
 * Description  停用spiosd
 * Return       none
 */
void sosd_disable(void)
{
#ifdef USE_CQ_WRITE
	if(cq_write_byte (TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)&(~SOSD_TD_EN))))
		ERROR (("cq_write_byte()\n"));
	if (cq_write_byte (TWIC_P3, SOSD_CONTROL_REG, SOSD_SHADOW_UP))
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vsync () < 0)
		ERROR (("cq_flush()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)&(~SOSD_TD_EN)));
	IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_SHADOW_UP);		// sOSD_En, Shadow
#endif
	SOSD_IS_ENABLE= DISABLE;
}

/* sOSD Shadow Update */
/*
 * Synopsis    void sosd_update(void);
 * Description  刷新spiosd
 * Return       none
 */
void sosd_update(void)
{
#ifdef USE_CQ_WRITE
	if(SOSD_IS_ENABLE) {
		if (cq_flush_vde () < 0)
			ERROR (("cq_flush()\n"));
	} else {
		if (cq_flush_now () < 0)
			ERROR (("cq_flush()\n"));
	}	
#else
	if(SOSD_IS_ENABLE)
		IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_SHADOW_UP);		// sOSD_En, Shadow Update
	else
		IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_SHADOW_UP);		// Shadow Update	
#endif
}

/* sOSD Emulation Trig */
/*
 * Synopsis    void sosd_emu_trig(void);
 * Description  啟動emulation
 * Return       none
 */
void sosd_emu_trig(void)
{
	//if (!(cq_write_byte (TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_EMU_TRIG|SOSD_SHADOW_UP, CQ_TRIGGER_VSYNC)))
	//	ERROR (("cq_write_byte()\n"));
	IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_EMU_TRIG|SOSD_SHADOW_UP);
}

/* sOSD Emulation Exit */
/*
 * Synopsis    void sosd_emu_exit(void);
 * Description  關閉emulation
 * Return       none
 */
void sosd_emu_exit(void)
{
	//unsigned short i= 0;
	//if (!(cq_write_byte (TWIC_P3, SOSD_EMU_CTR_REG, EMU_EXIT, CQ_TRIGGER_VSYNC)))
	//	ERROR (("cq_write_byte()\n"));
	//IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_ENABLE);	// for SPIOSD shadow issue
	//for (i = 0; i < 0xffff; i++) {
    //    if (!(IC_ReadByte(TWIC_P0, SHADOW_CTRL_CONF_REG)&SHADOW_BUSY))
    //        break;
    //}
#if 1	
	IC_WritByte(TWIC_P3, SOSD_EMU_CTR_REG, EMU_EXIT);
#else
	if (cq_write_byte (TWIC_P3, SOSD_EMU_CTR_REG, EMU_EXIT))
		ERROR (("cq_write_byte()\n"));
	if (cq_flush_vde () < 0)
		ERROR (("cq_flush()\n"));
#endif
//	IC_WritByte(TWIC_P3, SOSD_CONTROL_REG, SOSD_EN|SOSD_SHADOW_UP);
//	IC_WritByte (TWIC_P0, SHADOW_CTRL_CONF_REG, SHADOW_CTRL_DISABLE);
/*	
	for (i = 0; i < 0xffff; i++) {
        if ((IC_ReadByte(TWIC_P3,SOSD_CONTROL_REG)&SOSD_EMU_DONE))
            break;
    }
	ERROR(("EMULATION EXIT ERROR!\n"));
*/
}

/* sOSD Wait Emulation Done */
/*
 * Synopsis    void sosd_wait_emu_done(void);
 * Description  等待emulation完成
 * Return       none
 */
void sosd_wait_emu_done(void)
{
	while(!(IC_ReadByte(TWIC_P3,SOSD_CONTROL_REG)&SOSD_EMU_DONE))
    {
    		//uart1_receive_packet();   // YC ADD FOR UART PROTOCOL 20110831
    }
}

/* set sOSD image of force alpha */
/*
 * Synopsis    void sosd_img_force_ctrl(unsigned char level);
 * Description  spiosd透明度控制
 * Parameters   level            - 透明等級
 * Return       none
 */
void sosd_img_force_ctrl(unsigned char level)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_FORCE_AB_REG, (IC_ReadByte(TWIC_P3, SOSD_FORCE_AB_REG)|((level>64?FORCE_ALPHA_MAX:level))));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_FORCE_AB_REG, (IC_ReadByte(TWIC_P3, SOSD_FORCE_AB_REG)|((level>64?FORCE_ALPHA_MAX:level))));
#endif
	sosd_update();	
}

/* enable sOSD image of force */
/*
 * Synopsis    void sosd_img_force_enable(void);
 * Description  啟用spiosd透明效果
 * Return       none
 */
void sosd_img_force_enable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_FORCE_AB_REG, (IC_ReadByte(TWIC_P3, SOSD_FORCE_AB_REG)|SOSD_FORCE_EN));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_FORCE_AB_REG, (IC_ReadByte(TWIC_P3, SOSD_FORCE_AB_REG)|SOSD_FORCE_EN));
#endif
	sosd_update();
}

/* disable sOSD image of force */
/*
 * Synopsis    void sosd_img_force_disable(void);
 * Description  停用spiosd透明效果
 * Return       none
 */
void sosd_img_force_disable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_FORCE_AB_REG, (IC_ReadByte(TWIC_P3, SOSD_FORCE_AB_REG)&(~SOSD_FORCE_EN)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_FORCE_AB_REG, (IC_ReadByte(TWIC_P3, SOSD_FORCE_AB_REG)&(~SOSD_FORCE_EN)));
#endif
	sosd_update();
}

/* set sOSD sprite of force alpha */
/*
 * Synopsis    void sosd_sp_force_ctrl(unsigned char level);
 * Description  spr透明度控制
 * Parameters   level            - 透明等級
 * Return       none
 */
void sosd_sp_force_ctrl(unsigned char level)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)|(level&FORCE_ALPHA_MASK)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)|(level&FORCE_ALPHA_MASK)));
#endif
	sosd_update();
}

/* enable sOSD sprite of force */
/*
 * Synopsis    void sosd_sp_force_enable(void);
 * Description  啟用spr透明效果
 * Return       none
 */
void sosd_sp_force_enable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)|SOSD_SP_FORCE));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)|SOSD_SP_FORCE));
#endif
	sosd_update();
}

/* sOSD Sprite Enable */
/*
 * Synopsis    void sosd_sp_enable(unsigned char load);
 * Description  啟用spr
 * Parameters   load            - 載入lut
 * Return       none
 */
void sosd_sp_enable(unsigned char load)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	
	if(load)
		_sosd_load_slut();
	rc = cq_write_byte (TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)|(SOSD_SP_EN)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	if(load)
		_sosd_load_slut();
	IC_WritByte(TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)|(SOSD_SP_EN)));
#endif	
	sosd_update();
}

/* sOSD Sprite Disable */
/*
 * Synopsis    void sosd_sp_disable(void);
 * Description  停用spr
 * Return       none
 */
void sosd_sp_disable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	//_sosd_load_slut();
	rc = cq_write_byte (TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)&(~SOSD_SP_EN)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_SP_CONTROL_REG, (IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)&(~SOSD_SP_EN)));
#endif
	sosd_update();
}

/*
 * Synopsis    void _sosd_tdc_addr(unsigned long address);
 * Description  設定td 色盤地址
 * Parameters   address            - td 色盤地址
 * Return       none
 */
void _sosd_tdc_addr(unsigned long address)
{
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TD_COLOR_ADDR_REG1, (address&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_TD_COLOR_ADDR_REG2, ((address>>8)&0x000000FF));
	rc = cq_write_byte (TWIC_P3, SOSD_TD_COLOR_ADDR_REG3, ((address>>16)&0x000000FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_TD_COLOR_ADDR_REG1, (address&0xFF));
	IC_WritByte(TWIC_P3, SOSD_TD_COLOR_ADDR_REG2, ((address>>8)&0xFF));
	IC_WritByte(TWIC_P3, SOSD_TD_COLOR_ADDR_REG3, ((address>>16)&0xFF));
#endif
#ifdef DEBUG_SOSD   
    dbg(0, ("_sosd_tdc_addr 0x%lX \n", address));
#endif
}

/*
 * Synopsis    void _sosd_tdc_h_size(unsigned short hsize);
 * Description  設定td 圖片寬度
 * Parameters   hsize            - td 圖片寬度
 * Return       none
 */
void _sosd_tdc_h_size(unsigned short hsize)
{
#ifdef USE_CQ_WRITE
    char rc=0;

	if(hsize%4) {
		hsize = ((hsize/4)*4);
		hsize += 8;
	} else
		hsize += 8;
	rc = cq_write_byte (TWIC_P3, SOSD_TD_COLOR_HSIZE_REG1, (hsize&0x00FF));
	rc = cq_write_byte (TWIC_P3, SOSD_TD_COLOR_HSIZE_REG2, ((hsize>>8)&0x00FF));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    if(hsize%4) {
		hsize += 4-(hsize%4);
		hsize += 8;
	} else
		hsize += 8;
	IC_WritByte(TWIC_P3, SOSD_TD_COLOR_HSIZE_REG1, (hsize&0x00FF));		// L
	IC_WritByte(TWIC_P3, SOSD_TD_COLOR_HSIZE_REG2, ((hsize>>8)&0x00FF));	// H
#endif
#ifdef DEBUG_SOSD
    dbg(0, ("_sosd_tdc_h_size %u \n", hsize));
#endif
}

/*
 * Synopsis    void _sosd_td_spilt_mode_en(void);
 * Description  啟用td spilt mode(td+spr)
 * Return       none
 */
void _sosd_td_spilt_mode_en(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)|(SOSD_TD_COLOR_SPILT_MODE)|(SOSD_TD_EN)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)|(SOSD_TD_COLOR_SPILT_MODE)|(SOSD_TD_EN)));
#endif
}

/*
 * Synopsis    void _sosd_td_spilt_mode_dis(void);
 * Description  停用td spilt mode
 * Return       none
 */
void _sosd_td_spilt_mode_dis(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)&(~SOSD_TD_COLOR_SPILT_MODE)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)&(~SOSD_TD_COLOR_SPILT_MODE)));
#endif
}

/*
 * Synopsis    void sosd_td_color_addr(unsigned long address);
 * Description  設定td 色盤地址並刷新
 * Parameters   address            - td 色盤地址
 * Return       none
 */
void sosd_td_color_addr(unsigned long address)
{
	_sosd_tdc_addr(address);
	
	sosd_update();
}

/*
 * Synopsis    void sosd_td_color_h_size(unsigned short hsize);
 * Description  設定td 圖片寬度並刷新
 * Parameters   hsize            - td 圖片寬度
 * Return       none
 */
void sosd_td_color_h_size(unsigned short hsize)
{
	_sosd_tdc_h_size(hsize);

	sosd_update();
}

/*
 * Synopsis    void sosd_td_enable(void);
 * Description  啟用TD
 * Return       none
 */
void sosd_td_enable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)|(SOSD_TD_EN)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)|(SOSD_TD_EN)));
#endif	
	sosd_update();
}

/*
 * Synopsis    void sosd_td_disable(void);
 * Description  停用TD
 * Return       none
 */
void sosd_td_disable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)&(~SOSD_TD_EN)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)&(~SOSD_TD_EN)));
#endif	
	sosd_update();
}

/*
 * Synopsis    void sosd_td_spilt_mode_enable(void);
 * Description  啟用td spilt mode並刷新
 * Return       none
 */
void sosd_td_spilt_mode_enable(void)
{
	_sosd_td_spilt_mode_en();
		
	sosd_update();
}

/*
 * Synopsis    void sosd_td_spilt_mode_disable(void);
 * Description  停用td spilt mode並刷新
 * Return       none
 */
void sosd_td_spilt_mode_disable(void)
{
	_sosd_td_spilt_mode_dis();
		
	sosd_update();

}

void sosd_td_spi_gap(unsigned char gap)
{
	IC_WritByte(TWIC_P3, SOSD_TD_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG) | (gap & SOSD_TD_SPI_GAP) ));
}

/*
 * Synopsis    void _sosd_swtc_offset(unsigned short offset);
 * Description  設定swtc偏移量
 * Parameters   offset            - 偏移量
 * Return       none
 */
void _sosd_swtc_offset(unsigned short offset)
{
#ifdef USE_CQ_WRITES
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SW_TC_REG1, (offset&0x00FF));
	rc = cq_write_byte (TWIC_P3, SOSD_SW_TC_REG2, ((IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG)&(~SOSD_SW_TC2_BIT)) | ((offset>>8)&SOSD_SW_TC2_BIT)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_SW_TC_REG1, (offset&0x00FF));
	IC_WritByte(TWIC_P3, SOSD_SW_TC_REG2, ((IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG)&(~SOSD_SW_TC2_BIT)) | ((offset>>8)&SOSD_SW_TC2_BIT)));
	
#endif
}

/*
 * Synopsis    void _sosd_swtc_h_mode(void);
 * Description  swtc平移模式
 * Return       none
 */
void _sosd_swtc_h_mode(void)
{
#ifdef USE_CQ_WRITES
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TC_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) & (~SOSD_TC_V_MODE)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_TC_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) & (~SOSD_TC_V_MODE)));
#endif
}

/*
 * Synopsis    void _sosd_swtc_v_mode(void);
 * Description  swtc直移模式
 * Return       none
 */
void _sosd_swtc_v_mode(void)
{
#ifdef USE_CQ_WRITES
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TC_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) | SOSD_TC_V_MODE));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_TC_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) | SOSD_TC_V_MODE));
#endif
}

/*
 * Synopsis    void _sosd_swtc_enable(void);
 * Description  啟用swtc功能
 * Return       none
 */
void _sosd_swtc_enable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TC_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) | SOSD_TC_EN));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_TD_COLOR_HSIZE_REG2, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) | SOSD_TC_EN));
#endif
}

/*
 * Synopsis    void _sosd_swtc_disable(void);
 * Description  停用swtc功能
 * Return       none
 */
void _sosd_swtc_disable(void)
{
#ifdef USE_CQ_WRITE
	char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_TC_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) & (~SOSD_TC_EN)));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
    IC_WritByte(TWIC_P3, SOSD_TC_CTRL_REG, (IC_ReadByte(TWIC_P3, SOSD_TC_CTRL_REG) & (~SOSD_TC_EN)));
#endif
}

/*
 * Synopsis    void sosd_sw_tc_offset(unsigned short offset);
 * Description  設定swtc偏移量並刷新
 * Parameters   offset            - 偏移量
 * Return       none
 */
void sosd_sw_tc_offset(unsigned short offset)
{
	_sosd_swtc_offset(offset);
	sosd_update();
}

/*
 * Synopsis    void sosd_sw_tc_h_mode(void);
 * Description  設定swtc平移模式必並刷新
 * Return       none
 */
void sosd_sw_tc_h_mode(void)
{
	_sosd_swtc_h_mode();
	sosd_update();
}

/*
 * Synopsis    void sosd_sw_tc_v_mode(void);
 * Description  設定swtc直移模式並刷新
 * Return       none
 */
void sosd_sw_tc_v_mode(void)
{
	_sosd_swtc_v_mode();
	sosd_update();
}

/*
 * Synopsis    void sosd_sw_tc_enable(void);
 * Description  啟用swtc功能並刷新
 * Return       none
 */
void sosd_sw_tc_enable(void)
{
	_sosd_swtc_enable();
	sosd_update();
}

/*
 * Synopsis    void sosd_sw_tc_disable(void);
 * Description  停用swtc功能並刷新
 * Return       none
 */
void sosd_sw_tc_disable(void)
{
	_sosd_swtc_disable();
	sosd_update();
}

/*
 * Synopsis    void _get_spr_idx_addr(unsigned long *addr);
 * Description  讀取spr資料地址
 * Parameters   addr            - 地址變數
 * Return       none
 */
void _get_spr_idx_addr(unsigned long *addr)
{	
	*addr = IC_ReadByte(TWIC_P3, SOSD_SPRITE_ADDR_REG3);
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_SPRITE_ADDR_REG2));
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_SPRITE_ADDR_REG1));
}

/*
 * Synopsis    void _get_img_idx_addr(unsigned long *addr);
 * Description  讀取img資料地址
 * Parameters   addr            - 地址變數
 * Return       none
 */
void _get_img_idx_addr(unsigned long *addr)
{	
	*addr = IC_ReadByte(TWIC_P3, SOSD_IMG_ADDR_REG3);
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_IMG_ADDR_REG2));
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_IMG_ADDR_REG1));
}

/*
 * Synopsis    void _get_plut_addr(unsigned long *addr);
 * Description  讀取plut資料地址
 * Parameters   addr            - 地址變數
 * Return       none
 */
void _get_plut_addr(unsigned long *addr)
{	
	*addr = IC_ReadByte(TWIC_P3, SOSD_LUT_ADDR_REG3);
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_LUT_ADDR_REG2));
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_LUT_ADDR_REG1));
}

/*
 * Synopsis    void _get_slut_addr(unsigned long *addr);
 * Description  讀取slut資料地址
 * Parameters   addr            - 地址變數
 * Return       none
 */
void _get_slut_addr(unsigned long *addr)
{	
	*addr = IC_ReadByte(TWIC_P3, SOSD_SLUT_ADDR_REG3);
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_SLUT_ADDR_REG2));
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_SLUT_ADDR_REG1));
}

/*
 * Synopsis    void _get_tdc_h_size(unsigned short *hsize);
 * Description  讀取TD寬度
 * Parameters   hsize            - 資料變數
 * Return       none
 */
void _get_tdc_h_size(unsigned short *hsize)
{
	*hsize = IC_ReadByte(TWIC_P3, SOSD_TD_COLOR_HSIZE_REG2);
	*hsize <<= 8;
	*hsize |= (IC_ReadByte(TWIC_P3, SOSD_TD_COLOR_HSIZE_REG1));
}

/*
 * Synopsis    void _get_tdc_addr(unsigned long *addr);
 * Description  讀取TD色盤地址
 * Parameters   addr            - 地址變數
 * Return       none
 */
void _get_tdc_addr(unsigned long *addr)
{
	*addr = IC_ReadByte(TWIC_P3, SOSD_TD_COLOR_ADDR_REG3);
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_TD_COLOR_ADDR_REG2));
	*addr <<= 8;
	*addr |= (IC_ReadByte(TWIC_P3, SOSD_TD_COLOR_ADDR_REG1));
}

/*
 * Synopsis    void _get_tdc_addr(unsigned long *addr);
 * Description  讀取swtc地址
 * Parameters   addr            - 地址變數
 * Return       none
 */
void _get_swtc_offset(unsigned short *addr)
{
	*addr = ((IC_ReadByte(TWIC_P3, SOSD_SW_TC_REG2)&SOSD_SW_TC2_BIT));
	*addr <<= 8;
	*addr |= IC_ReadByte(TWIC_P3, SOSD_SW_TC_REG1);
}

/*
 * Synopsis     void _get_img_h_offset(unsigned short *h_offset)
 * Description  get img of h position
 * Parameters   *h_offset	- save h of offset of point
 * Return       none
 */
void _get_img_h_offset(unsigned short *h_offset)
{
	*h_offset = ((IC_ReadByte(TWIC_P3, SOSD_HSTAR_REG2)&0x7F));
	*h_offset <<= 8;
	*h_offset |= IC_ReadByte(TWIC_P3, SOSD_HSTAR_REG1);
}

/*
 * Synopsis     void _get_img_v_offset(unsigned short *v_offset)
 * Description  get img of v position
 * Parameters   *v_offset	- save v of offset of point
 * Return       none
 */
void _get_img_v_offset(unsigned short *v_offset)
{
	*v_offset = ((IC_ReadByte(TWIC_P3, SOSD_VSTAR_REG2)&0x3F));
	*v_offset <<= 8;
	*v_offset |= IC_ReadByte(TWIC_P3, SOSD_VSTAR_REG1);
}

/*
 * Synopsis     void _get_spr_h_offset(unsigned short *h_offset)
 * Description  get spr of h position
 * Parameters   *h_offset	- save h of offset of point
 * Return       none
 */
void _get_spr_h_offset(unsigned short *h_offset)
{
	*h_offset = ((IC_ReadByte(TWIC_P3, SOSD_SP_HSTAR_REG2)&0x7F));
	*h_offset <<= 8;
	*h_offset |= IC_ReadByte(TWIC_P3, SOSD_SP_HSTAR_REG1);
}

/*
 * Synopsis     void _get_spr_v_offset(unsigned short *v_offset)
 * Description  get spr of v position
 * Parameters   *v_offset	- save v of offset of point
 * Return       none
 */
void _get_spr_v_offset(unsigned short *v_offset)
{
	*v_offset = ((IC_ReadByte(TWIC_P3, SOSD_SP_VSTAR_REG2)&0x3F));
	*v_offset <<= 8;
	*v_offset |= IC_ReadByte(TWIC_P3, SOSD_SP_VSTAR_REG1);
}

/*
 * Synopsis     void _set_img_h_offset(unsigned short h_offset)
 * Description  setting img of h position
 * Parameters   v_offset	- h of offset
 * Return       none
 */
void _set_img_h_offset(unsigned short h_offset)
{	
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_HSTAR_REG1, (h_offset&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_HSTAR_REG2, ((h_offset&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_HSTAR_REG1, (h_offset&0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_HSTAR_REG2, ((h_offset&0x7FF)>>8));		// H
#endif
}

/*
 * Synopsis     void _set_img_v_offset(unsigned short v_offset)
 * Description  setting img of v position
 * Parameters   v_offset	- v of offset
 * Return       none
 */
void _set_img_v_offset(unsigned short v_offset)
{	
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_VSTAR_REG1, (v_offset&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_VSTAR_REG2, ((v_offset&0x3FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_VSTAR_REG1, (v_offset&0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_VSTAR_REG2, ((v_offset&0x3FF)>>8));		// H
#endif
}

/*
 * Synopsis     void _set_spr_h_offset(unsigned short h_offset)
 * Description  setting spr of h position
 * Parameters   h_offset	- h of offset
 * Return       none
 */
void _set_spr_h_offset(unsigned short h_offset)
{	
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_HSTAR_REG1, (h_offset&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_SP_HSTAR_REG2, ((h_offset&0x7FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_SP_HSTAR_REG1, (h_offset&0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_SP_HSTAR_REG2, ((h_offset&0x7FF)>>8));		// H
#endif
}

/*
 * Synopsis     void _set_spr_v_offset(unsigned short v_offset)
 * Description  setting spr of v position
 * Parameters   v_offset	- v of offset
 * Return       none
 */
void _set_spr_v_offset(unsigned short v_offset)
{	
#ifdef USE_CQ_WRITE
    char rc=0;
	rc = cq_write_byte (TWIC_P3, SOSD_SP_VSTAR_REG1, (v_offset&0xFF));
	rc = cq_write_byte (TWIC_P3, SOSD_SP_VSTAR_REG2, ((v_offset&0x3FF)>>8));
	if(rc<0)
		ERROR (("cq_write_byte()\n"));
#else
	IC_WritByte(TWIC_P3, SOSD_SP_VSTAR_REG1, (v_offset&0xFF));			// L
	IC_WritByte(TWIC_P3, SOSD_SP_VSTAR_REG2, ((v_offset&0x3FF)>>8));		// H
#endif
}


/*
 * Synopsis    void spiosd_offset_init( unsigned short h_offset, 
 *                               unsigned short v_offset, 
 *                               unsigned short s_h_offset, 
 *                               unsigned short s_v_offset);
 * Description  spiosd
 * Parameters   h_offset            - spiosd X軸起始位置
 *              v_offset            - spiosd Y軸起始位置
 *              s_h_offset          - sprite X軸起始位置
 *              s_v_offset          - sprite Y軸起始位置
 * Return       none
 */
void spiosd_offset_init(unsigned short h_offset, unsigned short v_offset, unsigned short s_h_offset, unsigned short s_v_offset)
{    
    HSTART_OFFSET=h_offset-18;
	VSTART_OFFSET=v_offset-2;
	SHSTART_OFFSET=s_h_offset-4;
	SVSTART_OFFSET=s_v_offset;

	_set_img_h_offset(h_offset);
	_set_img_v_offset(v_offset);
	_set_spr_h_offset(s_h_offset);
	_set_spr_v_offset(s_v_offset);    
}    

/*
 * Synopsis     unsigned short _adj_offset_value(unsigned short o_val,
 *									unsigned short m_val,
 *									unsigned short c_val)
 * Description  cal offset of value
 * Parameters   o_val	- origin offset of value
 *				m_val	- modify offset of value
 * 				c_val	- current position of value
 * Return       cal of value
 */
unsigned short _adj_offset_value(unsigned short o_val,unsigned short m_val,unsigned short c_val)
{
	if(o_val>m_val) {
		o_val-= m_val;
		c_val-= o_val;
	} else {
		m_val-= o_val;
		c_val+= m_val;
	}

	return c_val;
}

/*
 * Synopsis     void spiosd_offset_adj(unsigned short img_h
 *									unsigned short img_v,
 *									unsigned short spr_h,
 *									unsigned short spr_v)
 * Description  adj SPIOSD img/spr of position
 * Parameters   img_h	- img of h offset
 *				img_v	- img of v offset
 * 				spr_h	- spr of h offset
 *              spr_v	- spr of v offset
 * Return       none
 */
void spiosd_offset_adj(unsigned short img_h, unsigned short img_v, unsigned short spr_h, unsigned short spr_v)
{
	unsigned short tmp= 0;
	bit update_flag= 0;

	img_h-=18;
	img_v-=2;
	spr_h-=4;

	if(HSTART_OFFSET!=img_h) {
		_get_img_h_offset(&tmp);
		_set_img_h_offset(
			_adj_offset_value(HSTART_OFFSET, img_h, tmp));
		HSTART_OFFSET= img_h;
		update_flag= 1;
	}
	if(VSTART_OFFSET!=img_v) {
		_get_img_v_offset(&tmp);
		_set_img_v_offset(
			_adj_offset_value(VSTART_OFFSET, img_v, tmp));
		VSTART_OFFSET= img_v;
		update_flag= 1;
	}
	if(SHSTART_OFFSET!=spr_h) {
		_get_spr_h_offset(&tmp);
		_set_spr_h_offset(
			_adj_offset_value(SHSTART_OFFSET, spr_h, tmp));
		SHSTART_OFFSET= spr_h;
		update_flag= 1;
	}
	if(SVSTART_OFFSET!=spr_v) {
		_get_spr_v_offset(&tmp);
		_set_spr_v_offset(
			_adj_offset_value(SVSTART_OFFSET, spr_v, tmp));
		SVSTART_OFFSET= spr_v;
		update_flag= 1;
	}
	
	if(update_flag)
		sosd_update();
}
