/**
 *  @file   spiosd.h
 *  @brief  head file for SPIOSD control function
 *  $Id: spiosd.h,v 1.6 2013/06/06 09:56:34 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.6 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _SPIOSD_H
#define _SPIOSD_H

#define		LOAD_LUT	1
#define		DONT_LOAD	0

/* Advance Function */
void 
_sosd_emu_addr(unsigned long base_address);
void 
_sosd_img_addr(unsigned long base_address);
void 
_sosd_spr_addr(unsigned long base_address);
void 
_sosd_pLUT_addr(unsigned long base_address);
void 
_sosd_sLUT_addr(unsigned long base_address);
void 
_sosd_hspos(unsigned short h_pos);
void 
_sosd_vspos(unsigned short v_pos);
void 
_sosd_sp_hspos(unsigned short h_pos);
void 
_sosd_sp_vspos(unsigned short v_pos);
void 
_sosd_img_loca(unsigned short x, unsigned short y);
void 
_sosd_spr_loca(unsigned short x, unsigned short y);
void
_sosd_td_loca(unsigned short x, unsigned short y);
void 
_sosd_img_window(unsigned short, unsigned short);
void 
_sosd_spr_window(unsigned char, unsigned short);
void 
_sosd_td_window(unsigned short width, unsigned short height);
void 
_sosd_img_ls(unsigned short line_store);
void 
_sosd_spr_ls(unsigned short line_store);
void
_sosd_td_ls(unsigned short line_store);
void 
_sosd_load_plut(void);
void 
_sosd_load_slut(void);
void
_sosd_tdc_addr(unsigned long address);
void
_sosd_tdc_h_size(unsigned short hsize);
void 
_sosd_td_spilt_mode_en(void);
void
_sosd_td_spilt_mode_dis(void);


extern void spiosd_init(unsigned short h_offset, unsigned short v_offset, unsigned short s_h_offset, unsigned short s_v_offset);
extern void spiosd_quad_mode_enable();
extern void spiosd_quad_mode_disable();
extern void sosd_hvoffset_init(unsigned short h_offset, unsigned short v_offset);
extern void sosd_sp_hvoffset_init(unsigned short h_offset, unsigned short v_offset);
extern void sosd_set_emu_base_addrss(unsigned long address);
extern void sosd_dma_ctrl(void);
extern void sosd_set_pLUT_base_address(unsigned long base_address);
extern void sosd_set_img_base_address(unsigned long base_address);
extern void sosd_set_sLUT_base_address(unsigned long base_address);
extern void sosd_set_spr_base_address(unsigned long base_address);

extern void sosd_pLUTsLUT_load(void);
extern void sosd_enable(unsigned char load);
extern void sosd_disable(void);

extern void sosd_emu_trig(void);
extern void sosd_emu_exit(void);
extern void sosd_wait_emu_done(void);

//extern void sosd_alphab_ctrl(unsigned char aBbit0, unsigned char aBbit1);
extern void sosd_set_hspos(unsigned short h_pos);
extern void sosd_set_vspos(unsigned short v_pos);
extern void sosd_set_sp_hspos(unsigned short h_pos);
extern void sosd_set_sp_vspos(unsigned short v_pos);
extern void sosd_set_img_location(unsigned short x, unsigned short y);
extern void sosd_set_spr_location(unsigned short x, unsigned short y);
extern void sosd_set_td_location(unsigned short x, unsigned short y);
extern void sosd_set_switch_tc(unsigned short stc);
extern void sosd_set_img_active_window(unsigned short, unsigned short);
extern void sosd_set_spr_active_window(unsigned char, unsigned short);
extern void sosd_set_td_active_window(unsigned short width, unsigned short height);
extern void sosd_set_img_line_store(unsigned short line_store);
extern void sosd_set_spr_line_store(unsigned short line_store);
extern void sosd_set_td_line_store(unsigned short line_store);
extern void sosd_update(void);
extern void sosd_img_force_ctrl(unsigned char level);
extern void sosd_img_force_enable(void);
extern void sosd_img_force_disable(void);
extern void sosd_sp_force_ctrl(unsigned char level);
extern void sosd_sp_force_enable(void);
extern void sosd_sp_force_disable(void);
extern void sosd_sp_enable(unsigned char load);
extern void sosd_sp_disable(void);
extern void sosd_td_color_addr(unsigned long address);
extern void sosd_td_color_h_size(unsigned short hsize);

extern void sosd_td_enable(void);
extern void sosd_td_disable(void);
extern void sosd_td_spilt_mode_enable(void);
extern void sosd_td_spilt_mode_disable(void);
extern void sosd_td_spi_gap(unsigned char gap);

extern void _sosd_swtc_h_mode(void);
extern void _sosd_swtc_v_mode(void);
extern void _sosd_swtc_enable(void);
extern void _sosd_swtc_disable(void);
extern void _sosd_swtc_offset(unsigned short offset);
extern void sosd_sw_tc_offset(unsigned short offset);
extern void sosd_sw_tc_h_mode(void);
extern void sosd_sw_tc_v_mode(void);
extern void sosd_sw_tc_enable(void);
extern void sosd_sw_tc_disable(void);

extern void _get_spr_idx_addr(unsigned long *addr);
extern void _get_img_idx_addr(unsigned long *addr);
extern void _get_plut_addr(unsigned long *addr);
extern void _get_slut_addr(unsigned long *addr);
extern void _get_tdc_h_size(unsigned short *hsize);
extern void _get_tdc_addr(unsigned long *addr);
extern void _get_swtc_offset(unsigned short *addr);

extern void spiosd_offset_init(unsigned short h_offset, unsigned short v_offset, unsigned short s_h_offset, unsigned short s_v_offset);
extern void spiosd_offset_adj(unsigned short img_h, unsigned short img_v, unsigned short spr_h, unsigned short spr_v);

#endif	/* _SPIOSD_H */
