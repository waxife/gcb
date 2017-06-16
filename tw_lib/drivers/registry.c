/*
 *  @file   registry.c
 *  @brief  registry for nor 
 *  $Id: registry.c,v 1.13 2013/04/02 05:42:18 dos1236 Exp $
 *  $Author: dos1236 $
 *  $Revision: 1.13 $
 *
 *  Copyright (c) 2012 Terawins Inc. All rights reserved.
 * 
 *  @date   2012/06/11  yc New file.
 *
 */

#include "sys.h"
#include "reg_tw.h"
#include "spirw.h"
#include "serial.h"
#include "spirw.h"
#include "spiosd.h"
#include "registry.h"
extern unsigned long code LAYOUT_USERSTART;
extern unsigned long code LAYOUT_USERSIZE;

#define _dfu_offset 16384L 

#define REGISTRY_SECTION_BASE   LAYOUT_USERSTART /* 64K, NEED TO MODIFY FOR REALITY */
#define REGISTRY_SECTION_SIZE   LAYOUT_USERSIZE
/*
 * 摘要 :void registry_preprocess(void)
 *
 * 描述 :registry 做之前会做的预设动作
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void registry_preprocess(void)
{
    //if(spi_hp_mode_use)
        //spi_hp_mode_disable();
    
    sosd_td_spilt_mode_disable();
    sosd_disable();
	sosd_sp_disable();
    spi_quad_mode_disable();
    spiosd_quad_mode_disable();
	dbg(3,("disable\r\n"));
}
/*
 * 摘要 :void registry_postprocess(void)
 *
 * 描述 :registry 做完恢复之前的状态
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void registry_postprocess(unsigned long status)
{
#if (defined(WIN_QUAD_NOR) || defined(AMIC_QUAD_NOR))
    spi_wrsr(QUAD_ENABLE_WIN);
#else
    spi_wrsr(QUAD_ENABLE);
#endif
    //spi_quad_mode_enable();               
    //spiosd_quad_mode_enable();
	if(((status>>16)&0xff)&(SPI_PRE_WREN|SPI_4X_I8051|SPI_4X_DMA))
	{
	 		spi_quad_mode_enable();
			dbg(3,("SPI_4X_I8051 Enable\r\n"));
	}
	if(((status>>24)&0xff)&OSPI_4X)
	{	
			spiosd_quad_mode_enable();
			dbg(3,("OSPI_4X Enable\r\n"));
	}
	if((status&0xff)&SOSD_TD_EN)
	{
			sosd_td_enable();
			dbg(3,("SOSD_TD_EN\r\n"));
	}
	if(((status)&0xff)&SOSD_TD_COLOR_SPILT_MODE)
	{
			sosd_td_spilt_mode_enable();
			dbg(3,("SOSD_TD_COLOR_SPILT_MODE\r\n"));
	}
	if(((status>>8)&0xff)&SOSD_EN)
	{
			sosd_enable(1);
			dbg(3,("SOSD_EN\r\n"));
	}
	if(((status)&0xff)&0x01)
	{
			sosd_sp_enable(1);
			dbg(3,("Sprite_EN\r\n"));
	}

	//else
	//{
	//	sosd_td_enable();
//		sosd_td_spilt_mode_enable();
//		sosd_enable(0x01);
//	}
	//sosd_td_enable();
    //if(spi_hp_mode_use)
        //spi_hp_mode_enable();
}
/*
 * 摘要 :int registry_slot_timestamp_check(unsigned long addr, unsigned char *ts, char flag)
 *
 * 描述 :registry 做完恢复之前的状态
 *
 * 参数 :unsigned long addr => flash address
 *		 unsigned char *ts  => ts 纪录 start 或者 end 
 *		 char flag  => 标记
 *
 * 回传值 :0=>成功 -1=>失败
 *
 * 注意 : 无
*/
int registry_slot_timestamp_check(unsigned long addr, unsigned char *ts, char flag)
{
    unsigned char buf[4];
    if(spi_dma_read(addr, (unsigned short)buf, 4))
    {
        if(flag == 0)
        {   // start of the registry slot
            *ts = buf[2];
            return 0;
        }
        else
        {   // end of the registry slot
            *ts = buf[2];
            return 0;
        }
    }
    else
    {
        ERROR(("registry_slot_timestamp_check: spi dma read fail\r\n"));
        return -1;
    }
}
/*
 * 摘要 :int registry_read(unsigned long registry, int size)
 *
 * 描述 :registry 读取flash资料
 *
 * 参数 :unsigned long registry =>读取到所储存的位置
 *　　　 int size               =>大小
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
int registry_read(unsigned long registry, int size)
{
    char i = 0;
    unsigned char t1, t2, t3;
	unsigned char t1d, t2d;
    char candi = -1;
    int rc = 0;
    //registry_preprocess();
    
    if(size > REGISTRY_SLOT_SIZE - 8)
    {
        ERROR(("Read length > Registry Slot Size\r\n"));
        rc = -1;
        goto RET;
    }

    // find the latest registry slot
    // copy registry
    // error case process
	//registry_preprocess();
	//printf_tw("test\r\n");
    for(i = 0; i < REGISTRY_ITEMS; i++)
    {
        
        if(registry_slot_timestamp_check(REGISTRY_SECTION_BASE + (i * REGISTRY_SLOT_SIZE)      , &t1, 0) != 0)
            continue;
        if(registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((i+1)*REGISTRY_SLOT_SIZE) - 4, &t2, 1) != 0)
            continue;
        if(registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((i+1)*REGISTRY_SLOT_SIZE)    , &t3, 0) != 0)
            continue;
		dbg(2,("R  i=%bd candi=0x%bx t1=%bx t2=0x%bx t3=%bx\r\n",i,candi,t1,t2,t3));
        if(t1 == t2 && t1 != 0xff)
        {
            candi = i;
			t1d = t1;
			t2d = t2;
			if(i==(REGISTRY_ITEMS-1))
			{
				if(candi != -1)
             	{
                	spi_dma_read(REGISTRY_SECTION_BASE + (candi*REGISTRY_SLOT_SIZE) + 4, (unsigned long)registry, size);
                	dbg(2, ("registry 1 read: dma read from slot %bd\r\n", candi));
                	rc = 0;
                	goto RET;
             	}
			}
        }
        else if(t1 == 0xff && t2 == 0xff)
        {
             if(candi != -1)
             {
                spi_dma_read(REGISTRY_SECTION_BASE + (candi*REGISTRY_SLOT_SIZE) + 4, (unsigned long)registry, size);
                dbg(2, ("registry 2 read: dma read from slot %bd\r\n", candi));
                rc = 0;
                goto RET;
             }
             goto ERR;
        }
        else if(t1 != 0xff && t2 == 0xff && t3 == 0xff)
        {
            if(candi != -1)
            {
                spi_dma_read(REGISTRY_SECTION_BASE + (candi*REGISTRY_SLOT_SIZE) + 4, (unsigned long)registry, size);
                dbg(3, ("registry 3 read: dma read from slot %bd\r\n", candi));
                rc = -1;
                goto RET;
            }
            goto ERR;
        }
    }

ERR:
    ERROR(("Can't find vaild Registry Slot\r\n"));
    rc = -1;
RET:
	dbg(2,("R  i=%bd candi=0x%bx t1=%bx t2=0x%bx t3=%bx\r\n",i,candi,t1d,t2d,t3));
    //registry_postprocess();
    return rc;    
}


// check: size < REGISTRY_SIZE - 8;
// find a slot to write registry
// if there is no slot, erase this section and put registry into first slot
/*
 * 摘要 :int registry_write(unsigned long registry, int size)
 *
 * 描述 :registry 写入资料到flash
 *
 * 参数 :unsigned long registry =>从哪一个位置写入到flash
 *　　　 int size               =>大小
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
int registry_write(unsigned long registry, int size)
{
    char i = 0;
    unsigned char t1, t2, t3;
    char candi = -1;
    unsigned char buf[4];
    int rc = 0;
	unsigned long status=0x00;
	
    if(size > REGISTRY_SLOT_SIZE - 8)
    {
        ERROR(("Write length > Registry Slot Size\r\n"));
        rc = -1;
        goto RET;
    }

    // find the latest registry slot
    // copy registry
    // error case process
    for(i = 0; i < REGISTRY_ITEMS; i++)
    { 
        if(registry_slot_timestamp_check(REGISTRY_SECTION_BASE + (i * REGISTRY_SLOT_SIZE)      , &t1, 0) != 0)
            continue;
        if(registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((i+1)*REGISTRY_SLOT_SIZE) - 4, &t2, 1) != 0)
            continue;
        if(registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((i+1)*REGISTRY_SLOT_SIZE)    , &t3, 0) != 0)
            continue;
        if(t1 == 0xff && t2 == 0xff)
        {
            buf[0] = '>'; buf[1] = '>'; buf[2] = i; buf[3] = 0x0a;
            if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + (i*REGISTRY_SLOT_SIZE), (unsigned long)buf, 4) == 0)
                goto ERR;
                        
            if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + (i*REGISTRY_SLOT_SIZE+4), (unsigned long)registry, size) == 0)
                goto ERR;

            buf[0] = '<'; buf[1] = '<'; buf[2] = i; buf[3] = 0x0a;
            if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + ((i+1)*REGISTRY_SLOT_SIZE)-4, (unsigned long)buf, 4) == 0)
                goto ERR;

           	registry_slot_timestamp_check(REGISTRY_SECTION_BASE + (i * REGISTRY_SLOT_SIZE)      , &t1, 0);
			if(t1!=i)
				dbg(0, ("write t1 error\r\n"));	
       		registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((i+1)*REGISTRY_SLOT_SIZE) - 4, &t2, 1);
			if(t2!=i)
				dbg(0, ("write t2 error\r\n"));
			if((t2!=i)||(t1!=i))
			{
				buf[0] = '>'; buf[1] = '>'; buf[2] = i; buf[3] = 0x0a;
	            if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + (i*REGISTRY_SLOT_SIZE), (unsigned long)buf, 4) == 0)
	                goto ERR;
	                        
	            if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + (i*REGISTRY_SLOT_SIZE+4), (unsigned long)registry, size) == 0)
	                goto ERR;
	
	            buf[0] = '<'; buf[1] = '<'; buf[2] = i; buf[3] = 0x0a;
	            if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + ((i+1)*REGISTRY_SLOT_SIZE)-4, (unsigned long)buf, 4) == 0)
	                goto ERR;
	
	           	registry_slot_timestamp_check(REGISTRY_SECTION_BASE + (i * REGISTRY_SLOT_SIZE)      , &t1, 0);
				if(t1!=i)
					dbg(0, ("write t1 error\r\n"));	
	       		registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((i+1)*REGISTRY_SLOT_SIZE) - 4, &t2, 1);
				if(t2!=i)
					dbg(0, ("write t2 error\r\n"));
			}
			registry_slot_timestamp_check(REGISTRY_SECTION_BASE + (i * REGISTRY_SLOT_SIZE)      , &t1, 0);
			if(t1!=i)
				dbg(0, ("write t3 error\r\n"));	
       		registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((i+1)*REGISTRY_SLOT_SIZE) - 4, &t2, 1);
			if(t2!=i)
				dbg(0, ("write t4 error\r\n"));
          	
            dbg(2, ("registry write: dma wirte to slot %bd\r\n", i));
            rc = 0;
            goto RET;
        }
    }
	
	status=status|IC_ReadByte(TWIC_P3, SOSD_TD_CTRL_REG)<<0;
	status=status|IC_ReadByte(TWIC_P3, SOSD_CONTROL_REG)<<8;
	status=status|IC_ReadByte(TWIC_P6, SPI_4X_ENABLE_REG)<<16;
	status=status|IC_ReadByte(TWIC_P6, OSPI_COMMAND_REG)<<24;
	status=status|((IC_ReadByte(TWIC_P3, SOSD_SP_CONTROL_REG)&SOSD_SP_EN)>>7);//bit 0 get sprite bit
	registry_preprocess();
    if(spi_erase_sector(REGISTRY_SECTION_BASE))
    {
        buf[0] = '>'; buf[1] = '>'; buf[2] = 0; buf[3] = 0x0a;
        if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset , (unsigned long)buf, 4) == 0)
            goto ERR;
                    
        if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + 4, (unsigned long)registry, size) == 0)
            goto ERR;
        
        buf[0] = '<'; buf[1] = '<'; buf[2] = 0; buf[3] = 0x0a;
        if(spi_dma_write((REGISTRY_SECTION_BASE+_dfu_offset + REGISTRY_SLOT_SIZE - 4), (unsigned long)buf, 4) == 0)
            goto ERR;

        dbg(2, ("registry write: erase sector and dma wirte to slot 0\r\n"));
        rc = 0;

			registry_slot_timestamp_check(REGISTRY_SECTION_BASE + (0 * REGISTRY_SLOT_SIZE)      , &t1, 0);
			if(t1!=0)
				dbg(0, ("flash write t1 error\r\n"));	
	       	registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((0+1)*REGISTRY_SLOT_SIZE) - 4, &t2, 1);
			if(t2!=0)
				dbg(0, ("flash write t2 error\r\n"));
			if((t2!=0)||(t1!=0))
			{
		        buf[0] = '>'; buf[1] = '>'; buf[2] = 0; buf[3] = 0x0a;
		        if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset , (unsigned long)buf, 4) == 0)
		            goto ERR;
		                    
		        if(spi_dma_write(REGISTRY_SECTION_BASE+_dfu_offset + 4, (unsigned long)registry, size) == 0)
		            goto ERR;
		        
		        buf[0] = '<'; buf[1] = '<'; buf[2] = 0; buf[3] = 0x0a;
		        if(spi_dma_write((REGISTRY_SECTION_BASE+_dfu_offset + REGISTRY_SLOT_SIZE - 4), (unsigned long)buf, 4) == 0)
		            goto ERR;
			}
			registry_slot_timestamp_check(REGISTRY_SECTION_BASE + (0 * REGISTRY_SLOT_SIZE)      , &t1, 0);
			if(t1!=0)
				dbg(0, ("flash write t3 error\r\n"));	
       		registry_slot_timestamp_check(REGISTRY_SECTION_BASE + ((0+1)*REGISTRY_SLOT_SIZE) - 4, &t2, 1);
			if(t2!=0)
				dbg(0, ("flash write t4 error\r\n"));

		registry_postprocess(status);
        goto RET;
    }
    else
    {
			registry_postprocess(status);
            ERROR(("registry_write, Spi Erase Sector Fail\r\n"));
            rc = -1;
            goto RET;
    }
    
ERR:
    ERROR(("registry_write, DMA Write Fail\r\n"));
    rc = -1;
RET:
	dbg(2,("W candi=0x%bx t1=%bx t2=0x%bx t3=%bx\r\n",i,t1,t2,t3));
    return rc;    
}

struct reg_test{
    char            v1;
    unsigned char   v2;
    int             v3;
    unsigned int    v4;
    short           v5;
    unsigned short  v6;
    long            v7;
    unsigned long   v8;
};
/*
 * 摘要 :void registry_test(void)
 *
 * 描述 :registry 写入读取　测试
 *
 * 参数 :无
 *
 * 回传值 :无
 *
 * 注意 : 无
*/
void registry_test(void)
{
    static char c = 0;
    int i, k;
    struct reg_test xdata t1 = {0};
    struct reg_test xdata t2 = {0};
	static unsigned char data count=0x00;
	if(count>0x7f)
		count=0x00;
	count++;
    DBG_PRINT("***** Registry Test ***** \r\n");
	//DBG_PRINT("base address = %lu\r\n",REGISTRY_SECTION_BASE);
	//DBG_PRINT("base address = %lu\r\n",LAYOUT_USERSTART);
	//DBG_PRINT("base address = %lu\r\n",LAYOUT_USERSIZE);
	twdDelay(1000);
    for(i = 0; i < 2; i++)
    {
        //c = getb_t();
        c = (c + 1)%2;
        
        //DBG_PRINT("getb_t = %bd\r\n", c);
        switch(c)
        {
            case 0:

	
				registry_read((unsigned long)&t2, sizeof(t2));
				if(t2.v1==t1.v1)
				{
        		//	DBG_PRINT("Read t2: v1 = %bd, v3 = %hd, v7 = %ld \r\n", t2.v1, t2.v3, t2.v7);
				}
				else
				{
					for(k=0;k<5;k++)
					{
						registry_read((unsigned long)&t2, sizeof(t2));
						DBG_PRINT("Read t2: v1 = %bd, v3 = %hd, v7 = %ld \r\n", t2.v1, t2.v3, t2.v7);	
					}
				}	
                break;
            case 1:
                t1.v1 += count; t1.v2 += count; t1.v3 += count; t1.v4 += count; 
                t1.v5 += count; t1.v6 += count; t1.v7 += count; t1.v8 += count; 
				//registry_write((unsigned long)&t1, sizeof(t1));
				registry_write((unsigned long)&t1, 100);
                //DBG_PRINT("Write t1: v1 = %bd, v3 = %hd, v7 = %ld \r\n", t1.v1, t1.v3, t1.v7);
                break;
            default:
                DBG_PRINT("No Match choose\r\n");
                break;
        }
    }
}