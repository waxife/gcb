/**
 *  @file   cq.h
 *  @brief  head file of command queue
 *  $Id: cq.h,v 1.2 2012/12/04 03:06:04 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.2 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/06/04  hugo	New file.
 *
 */

#ifndef __CQ_H
#define __CQ_H

/* error codes */
#define CQ_ERR_UNKNOWN          0x10    /* unknown interrupt status */
#define CQ_ERR_QUEUE_OVERFLOW   0x11    /* queue overflow */
#define CQ_ERR_TRIGGER_TIMEOUT  0x12    /* wait trigger signal timeout */
#define CQ_ERR_INVALID_CMD      0x13    /* invalid command */
#define CQ_ERR_EMPTY            0x20    /* issue empty command queue */
#define CQ_ERR_LENGTH           0x21    /* illegal data length */
#define CQ_ERR_CMD_OVERFLOW     0x22    /* too many data for a command */
#define CQ_ERR_UNFINISHED       0x23    /* the last command not finished */
#define CQ_ERR_ISSUE_TIMEOUT    0x24    /* wait issue done timeout */
#define CQ_ERR_ARGUMENT         0x25    /* illegal arguments */

extern unsigned char cq_debug;
#define CQ_DEBUG_SET_REG    (1 << 0)
#define CQ_DEBUG_DATAPORT   (1 << 1)
#define CQ_DEBUG_ISSUE      (1 << 2)
#define CQ_DEBUG_LINECNT    (1 << 3)
#define CQ_DEBUG_CONFIG     (1 << 4)
#define CQ_DEBUG_WRITE      (1 << 5)
#define CQ_DEBUG_REGTABLE   (1 << 6)

#define CQ_TRIGGER_SW           0
#define CQ_TRIGGER_VSYNC        1
#define CQ_TRIGGER_VDE          2

void cq_info (void);
void cq_dump (void);

char cq_available (void);
char cq_config (unsigned char page, unsigned char addr, unsigned char incr, unsigned char len);
char cq_data (unsigned char byte);
char cq_flush (void);
char cq_flush_now (void);
char cq_flush_vsync (void);
char cq_flush_vde (void);

char cq_write_cmd (unsigned char *cmd);
char cq_write_cmdset (unsigned char code *cmd[], char num);
void cq_trigger_source (unsigned char source);
void cq_trigger_delay (char enable, int count, char dly_in_hsync);
void cq_trigger_timeout (unsigned char count);
void cq_reset (void);
void cq_init (void);
void cq_isr (void);
char cq_busy (void);

char cq_write_byte(unsigned short bPage, unsigned char bAdd, unsigned char bData);
char cq_write_byte_issue(unsigned short bPage, unsigned char bAdd, unsigned char bData,unsigned char trig);
char cq_write_word(unsigned short bPage, unsigned char bAdd, unsigned short bData, unsigned char incr);
char cq_write_word_issue(unsigned short bPage, unsigned char bAdd, unsigned short bData, unsigned char incr,unsigned char trig);
char cq_WriteRegsTable(unsigned char code* RegsTable,unsigned char end_page);
char cq_WriteRegsTable_issue(unsigned char code* RegsTable,unsigned char end_page, unsigned char trig);
void cq_init_timeout(unsigned char timeout);

void cq_try_overflow (void);        /* test error handling of queue overflow */
void cq_try_trigger_timeout(void);  /* test error handling of trigger timeout */
void cq_try_invalid_cmd (void);     /* test error handling of invalid command */
void cq_try_issue_timeout (void);   /* test error handling of issue timeout */

#endif /* __CQ_H */
