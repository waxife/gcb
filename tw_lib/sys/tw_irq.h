/**
 *  @file   tw_irq.h
 *  @brief  here define irq number and irq functions
 *  $Id: tw_irq.h,v 1.2 2012/12/04 03:05:28 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _TW_IRQ_H
#define _TW_IRQ_H

#define IRQ_VSYNC_MISS		0
#define IRQ_HSYNC_MISS		1
#define IRQ_VSYNC_CHG		2
#define IRQ_HSYNC_CHG		3
#define IRQ_10MS_TIME		4
#define IRQ_VSYNC_LEAD		5
#define IRQ_SPI_DMA			6
#define IRQ_IR_PACKET   	7
#define IRQ_SAR0_TOGGLE		8
#define IRQ_SAR1_TOGGLE		9
#define IRQ_SHORT_VS		10
#define IRQ_SHDN_WAR		11
#define IRQ_UART0_INT		12
#define IRQ_UART1_INT		13
#define IRQ_GPIO_INT		14
#define IRQ_CQ_INT			15
#define IRQ_TOTAL_NUM		16

extern void backup_tw_irq (void);
extern void mute_tw_irq (void);
extern void restore_tw_irq (void);
extern void clear_tw_irq(unsigned char irq_nr);
extern void disable_tw_irq(unsigned char irq_nr);
extern void enable_tw_irq(unsigned char irq_nr);
extern unsigned short tw_irq_dispatch(void);
extern void init_IRQ(void);

#endif	/* _TW_IRQ_H */
