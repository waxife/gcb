/**
 *  @file   spirw.h
 *  @brief  head file for spirw
 *  $Id: spirw.h,v 1.10 2013/09/09 07:50:23 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.10 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _SPIRW_H
#define _SPIRW_H

extern bit enable_quad_io(void);
extern void dma_int_enable();
extern void dma_int_disable();
extern bit wait_dma_int(void);
extern bit spi_wren(void);
extern bit spi_wrsr(unsigned char reg1, unsigned char reg2);
extern bit spi_rdsr(unsigned char* states);
extern bit wait_dma(void);
extern bit spi_dma_write(unsigned long address, unsigned short xaddr, unsigned short length);
extern bit spi_dma_read(unsigned long address, unsigned short xaddr, unsigned short length);
extern unsigned char spi_read_byte(unsigned long address);
extern bit spi_write_byte(unsigned long address, unsigned char value);
extern bit spi_erase_sector(unsigned long address);
extern bit spi_dma2xram(unsigned long address, unsigned short xaddr, unsigned short length);
extern bit spi_dma2oram(unsigned long base_address, unsigned short oram_address, unsigned int length);
extern bit spi_dma_hp_mode_disable(void);
extern bit spi_dma_hp_mode_enable(void);
extern void spi_quad_mode_enable(void);
extern void spi_quad_mode_disable(void);
extern bit ospi_wren(void);
extern bit ospi_wrsr(unsigned char reg);
extern bit ospi_rdsr(unsigned char* states);
//extern bit spi_rdsr_GD(unsigned char* states,unsigned char* states1);
extern bit spi_rdsr_GD(unsigned char* states,unsigned char* states1,unsigned char* states2);
extern bit spi_rdsr_WQ(unsigned char* states,unsigned char* states1);
extern bit spi_hp_mode_disable(void);
extern bit spi_hp_mode_enable(void);

#endif	/* _SPIRW_H */
