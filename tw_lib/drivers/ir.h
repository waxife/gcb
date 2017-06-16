/**
 *  @file   ir.h
 *  @brief  head file for IR driver
 *  $Id: ir.h,v 1.2 2012/12/04 03:06:04 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _IR_H
#define _IR_H

#define IR_BUFF_SIZE	10

extern void ir_init(void);
extern void CheckIR(void);
extern unsigned char IRscan(unsigned char *value);

extern void ir_clear(void);

#endif	/* _IR_H */
