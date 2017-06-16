/**
 *  @file   tw_offset.c
 *  @brief  terawins adj OSD API for start of position
 *  $Id: tw_offset_table.c,v 1.1 2013/06/07 07:16:32 joec Exp $
 *  $Author: joec $
 *  $Revision: 1.1 $
 *
 *  Copyright (c) 2013 Terawins Inc. All rights reserved.
 *
 *  @date   2013/05/08  ken     New file.
 *
 */

#include "sys.h"
#include "tw_widget_sys.h"
#include "tw_offset_table.h"
#include "tw_offset_table_source.h"
#include "spiosd.h"
#include "osd2api.h"

//static unsigned char TW_OFFSET_AUTO_ADJ= 0;

/*
 * Synopsis     void tw_offset_auto_adj_enable(unsigned char en)
 * Description  enable auto adj OSD of offset
 * Parameters   en	- 1: enable, 0: disable
 * Return       none
 * NOTE         no use
 */
//void tw_offset_auto_adj_enable(unsigned char en)
//{
//	if(en)
//		TW_OFFSET_AUTO_ADJ= 1;
//	else
//		TW_OFFSET_AUTO_ADJ= 0;
//}

/*
 * Synopsis     void tw_osd_offset_init(unsigned char src,
 *                                      unsigned char std,
 *                                      unsigned short address,
 *                                      unsigned char width,
 *                                      unsigned char height)
 * Description  init SPIOSD img/spr and OSD2 of position
 * Parameters   h_offset            - spiosd X軸起始位置
 *              v_offset            - spiosd Y軸起始位置
 *              s_h_offset          - sprite X軸起始位置
 *              s_v_offset          - sprite Y軸起始位置
 * Return       none
 * NOTE         need TWViewer tool gen to offset tables
 */
void tw_osd_offset_init(void)
{
    PTW_OSD_OFFSET tw_temp;
	PTW_OSD_OFFSET_TBL tw_temp_tbl = &t123_osd_offset_tbl; 
	
	tw_temp = (PTW_OSD_OFFSET)&(tw_temp_tbl->desc[0]);
	
	spiosd_offset_init(tw_temp->img_hoffset, tw_temp->img_voffset, tw_temp->sp_hoffset, tw_temp->sp_voffset);	   
}    

/*
 * Synopsis     void tw_offset_adj(unsigned char src,
 *									unsigned char std,
 *									unsigned short address,
 *									unsigned char width,
 *									unsigned char height)
 * Description  adj SPIOSD img/spr and OSD2 of position for diff video input
 * Parameters   src			- video of source
 *				std			- video of standard
 * 				address		- OSD2 of menu address
 *              width		- OSD2 of menu width. not do adj when set '0'
 *              height		- OSD2 of menu row
 * Return       none
 * NOTE         need TWViewer tool gen to offset tables
 */
void tw_offset_adj(unsigned char src, unsigned char std, unsigned short address, unsigned char width, unsigned char height)
{
	PTW_OSD_OFFSET tw_temp;
	PTW_OSD_OFFSET_TBL tw_temp_tbl = &t123_osd_offset_tbl;
	unsigned char i= 0;
    
	src&= 0x0F;
	std&= 0x0F;
	src= ((src<<4)|std);
	
		/* search offset table for current of source and standard */
		for(i= 0; i< tw_temp_tbl->nelements; i++) {
    		tw_temp = (PTW_OSD_OFFSET)&(tw_temp_tbl->desc[i]);
    		
    		if(src==tw_temp->signal)
    			break;
    	}
    
    	if(i>=tw_temp_tbl->nelements) {
    		WARN(("no standard offset table for src= %bu, std= %bu !\n\r", (src>>4), std));
    		for(i= 0; i< tw_temp_tbl->nelements; i++)
    		{
    			tw_temp = (PTW_OSD_OFFSET)&(tw_temp_tbl->desc[i]);
    			
    			if((src&0xF0)==(tw_temp->signal&0xF0))
    				break;
    		}
    		if(i>=tw_temp_tbl->nelements) 
    		{
    			ERROR(("no source offset table for src= %bu, std= %bu !\n\r", (src>>4), std));
    			i= 1;
    			tw_temp = (PTW_OSD_OFFSET)&(tw_temp_tbl->desc[i]);
    		}
    	} else {
    		dbg(0, ("find offset table for src= %bu, std= %bu !\n\r", (src>>4), std));
    	}
        
        /* adj spiosd of offset */
    	spiosd_offset_adj(tw_temp->img_hoffset, tw_temp->img_voffset, tw_temp->sp_hoffset, tw_temp->sp_voffset);	
    	
    	/* check width and adj osd2 of offset */
    	if(width) {
    		osd_menu_location (tw_temp->osd2_ratth, tw_temp->osd2_rattv, address, width, height);
    	}
	}


