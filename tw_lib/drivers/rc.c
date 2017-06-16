/**
 *  @file   resource.c
 *  @brief  this header file put system interface initial function prototypeing
 *  $Id: rc.c,v 1.12 2013/08/01 03:16:13 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.12 $
 *
 *  Copyright (c) 2006 Terawins Inc. All rights reserved.
 * 
 *  @date   2006/12/19  New file.
 *
 */

#include "sys.h"
#include "spirw.h"
#include "rc.h"

//#define TW_RC_DEBUG_MODE

unsigned long code *_fresource = (unsigned long code*)(LAYOUT_RESSTART_ADDR);
//#define _eresource	0x000110D4L

/*
 *	SERACING FUNCTION
 */
unsigned char DEF_SEARCHING= 0;
#define DEF_SEARCHING_FUNCTION

#ifdef DEF_SEARCHING_FUNCTION
#define BINERY_VERSION
//#define FIBONACCI_VERSION
#endif

#ifdef BINERY_VERSION
#undef FIBONACCI_VERSION
#endif

#ifdef FIBONACCI_VERSION
#undef BINERY_VERSION
#endif	

static struct res_t res_s;
static struct res_item_t desc_s;

//#define DBG_RES_INFO
//#define OLD_TOOL_COMBINE

#ifdef OLD_TOOL_COMBINE
	#define RESOURCE_START_INDEX	0
#else
	#define RESOURCE_START_INDEX	1
#endif

/**
 * rc_init: init resource struct
 * return > 0 number of elements
 *        -1 invalid magic 
 *        -2 invalid number of elements
 *        -3 invalid pack struct
 *        -4 invalid element struct
 *        -5 invalid dma 2 xram
 */

 /*
 * Synopsis    int rc_init(void);
 * Description  res初始化
 * Return        0 number of elements
 *              -1 invalid magic 
 *              -2 invalid number of elements
 *              -3 invalid pack struct
 *              -4 invalid element struct
 *              -5 invalid dma 2 xram
 */
#ifdef OLD_TOOL_COMBINE
int rc_init(void)
{
    char rc= 0;
	struct res_t *res = (struct res_t *) (*_fresource);
    //struct res_item_t desc_s;
    //void *pend = (void *) _eresource -4;
    int n;
	unsigned long p_magic= 0;

	dbg(3, (" resource point = %p \r\n", res));

    rc = spi_dma2xram((unsigned long)res, (unsigned short)(&res_s), RES_T_SIZE);
	if(!rc)
		return -5;

    dbg(3, (" resource magic = 0x%lX \r\n", res_s.resource_magic));
	
	if (res_s.resource_magic != RESOURCE_MAGIC)
        return -1;
	
	/* resource validation */
    if (res_s.nelements <= 0)
        return -2;

    n = res_s.nelements;

	//rc = spi_dma2xram((unsigned long)&res->desc[n], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
	rc = spi_dma2xram((unsigned long)&res->desc[n+1], (unsigned short)(&p_magic), sizeof(unsigned long));
	if(!rc)
		return -5;


	//dbg(2, (" resource p = %p \r\n", (unsigned long *)(desc_s.offset + (char *)res)));
	//dbg(2, (" resource pend = %p \r\n", pend));

	//rc = spi_dma2xram((unsigned long) (desc_s.offset + (char *)res), (unsigned short)(&p_magic), sizeof(unsigned long));
	//rc = spi_dma2xram((unsigned long) (desc_s.offset), (unsigned short)(&p_magic), sizeof(unsigned long));
	//if(!rc)
	//	return -5;

	dbg(2, (" resource end magic = 0x%lX \r\n", p_magic));

    if (p_magic != RESOURCE_END_MAGIC) {
            return -3;
    }

#if 0
    for (i = 0; i < n ; i++) {
		rc = spi_dma2xram((unsigned long)&res->desc[i], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
		if(!rc)
			return -5;
        if ((desc_s.offset + (char *)res) >= (char *)pend)
            return -4;
    }
#endif

    return n;
}

#else
int rc_init(unsigned short *n)
{
    char rc= 0;
	struct res_t *res = (struct res_t *) (*_fresource);
	//struct res_item_t desc_s;
    //void *pend = (void *) _eresource -4;
	unsigned long p_magic= 0;

	desc_s.id = 0;

	dbg(3, (" resource point = %p \r\n", res));

    rc = spi_dma2xram((unsigned long)res, (unsigned short)(&res_s), RES_T_SIZE);
	if(!rc)
		return -5;

    dbg(3, (" resource magic = 0x%lX \r\n", res_s.resource_magic));
	
	if (res_s.resource_magic != RESOURCE_MAGIC)
        return -1;
	
	/* resource validation */
    if (res_s.nelements <= 0)
        return -2;

    *n = res_s.nelements;

	dbg(3, (" nelements = %u \r\n", res_s.nelements));

	//rc = spi_dma2xram((unsigned long)&res->desc[n], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
	rc = spi_dma2xram((unsigned long)&res->desc[(*n)], (unsigned short)(&p_magic), 4);
	if(!rc)
		return -5;


	//dbg(3, (" resource p = %p \r\n", (unsigned long *)(desc_s.offset)));
	//dbg(2, (" resource pend = %p \r\n", pend));

	//rc = spi_dma2xram((unsigned long) (desc_s.offset + (char *)res), (unsigned short)(&p_magic), sizeof(unsigned long));
	//if(!rc)
	//	return -5;

	dbg(3, (" resource end magic = 0x%lX \r\n", p_magic));

    DEF_SEARCHING= 0;	// not is seraching function version
	
	if (p_magic != RESOURCE_END_MAGIC) {
    	if (p_magic==RESOURCE_SEARCH_MAGIC)
			DEF_SEARCHING= 1;	// seraching function version
		else
	        return -3;
    }

#if 0
    for (i = 0; i < n ; i++) {
		rc = spi_dma2xram((unsigned long)&res->desc[i], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
		if(!rc)
			return -5;
        if ((desc_s.offset + (char *)res) >= (char *)pend)
            return -4;
    }
#endif

    return 0;
}
#endif

/**
 *	get resource id key
 */

 /*
 * Synopsis    unsigned short get_id_key( unsigned short id);
 * Description  讀取 resource[id] 之id資?
 * Parameters   id	                - res index
 * Return       item id
 */
unsigned short get_id_key(unsigned short id)
{
    char rc= 0;
	struct res_t *res = (struct res_t *) (*_fresource);
	//struct res_item_t desc_s;

	if (res_s.resource_magic != RESOURCE_MAGIC) {
		ERROR(("invalid rc init!\r\n"));
		return NULL;
	}
		
	rc = spi_dma2xram((unsigned long)&res->desc[id], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
    if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return NULL;
	}

	return desc_s.id;
}

#ifdef BINERY_VERSION
/**
 * Binary Searching
 */
int b_searching(int low, int high, unsigned short id)
{
	int i;

	if(id< get_id_key(low) || id> get_id_key(high))
		return 0;
	while(low<=high) {
		i= (low+high)/2;
		if(id< get_id_key(i))
			high = i-1;
		else if(id> get_id_key(i))
				low= i+1;
		else
			return i;
	}
	
	return 0;	
}
#endif

/**
 * Find Fibonacci i
 */
#ifdef FIBONACCI_VERSION
int fibonacci(int i) //reentrant
{
	int j= 0;
	int n_1= 0, n_2= 0, n= 0;

	if(i==0) return (0);
	else if(i==1) return (1);
	//else return (fibonacci(i-1)+fibonacci(i-2));
	else {
		n_2= 0;
		n_1= 1;
		for(j=2; j<=i; j++) {
			n = n_2+n_1;
			n_2 = n_1;
			n_1 = n;
		}
		return n;		
	}
}

/**
 * Fibonacci Search
 */
int f_searching(int n, unsigned short id)
{
	int i, j, k, m, p, q;

	if(id< get_id_key(0) || id> get_id_key(n-1))
		return 0;

	for(k=0; fibonacci(k)< n+1; k++);
	k--;
	m= n+1 - fibonacci(k);

	i= fibonacci(k-1);
	p= fibonacci(k-2);
	q= fibonacci(k-3);

	if(id > get_id_key(i)) {
#ifdef DEBUG_FIBONACCI
		dbg(0, ("id: %u and rc[%d]=%u compare!\n", id, i, get_id_key(i)));
#endif
		i= i+m;
	}

	while(i>= 1) {
		if(id< get_id_key(i)) {
#ifdef DEBUG_FIBONACCI
			dbg(0, ("id: %u and rc[%d]=%u compare!\n", id, i, get_id_key(i)));
#endif
			if(q==0)
				i=0;
			else {
				i= i-q;
				j= p;
				p= q;
				q= j-q;
			}
		} else;

		if(id> get_id_key(i)) {
#ifdef DEBUG_FIBONACCI			
			dbg(0, ("id: %u and rc[%d]=%u compare!\n", id, i, get_id_key(i)));
#endif
			if(p==1)
				i=0;
			else {
				i= i+q;
				p= p-q;
				q= q-p;
			}
		} else;

		if(id == get_id_key(i)) {	/* FIND */
#ifdef DEBUG_FIBONACCI			
			dbg(0, ("Find id: %u and rc[%d]=%u!\n", id, i, get_id_key(i)));
#endif
			return i;
		} else;
	}
	return (-1);
}
#endif


/** 
 * rc_data(ID)
 */

 /*
 * Synopsis    unsigned long rc_data(unsigned short id);
 * Description  讀取 resource[id] 之item
 * Parameters   id	                - res index
 * Return       item 於xram之address
 */
unsigned long rc_data(unsigned short id)
{
    char rc= 0;
	struct res_t *res = (struct res_t *) (*_fresource);
	//struct res_item_t desc_s;
    int i, n;

	if (res_s.resource_magic != RESOURCE_MAGIC) {
		ERROR(("invalid rc init!\r\n"));
		return NULL;
	}
		
    n = res_s.nelements;

	if(DEF_SEARCHING) {
//	#ifdef DEF_SEARCHING_FUNCTION
		if (id == desc_s.id)
	    	return (unsigned long)((char *)res + desc_s.offset);
		
	#ifdef FIBONACCI_VERSION
		/* Fibonacci searching */
		i= f_searching(n, id);
	#endif
	#ifdef BINERY_VERSION
		/* Binery searching */
		i= b_searching(0, n-1, id);
	#endif
		if(i>=0) {
			if (id == desc_s.id)
	            return (unsigned long)((char *)res + desc_s.offset);
		}
		ERROR(("NO ID!\r\n"));
		return NULL;
	} else {
//	#else
	    /* default searching */
		for (i = 0;i < n; i++) {
	        rc = spi_dma2xram((unsigned long)&res->desc[i], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
			if(!rc)
				goto EXIT;
			if (id == desc_s.id)
	            return (unsigned long)((char *)res + desc_s.offset);
	    }
		ERROR(("NO ID!\r\n"));
	EXIT:
	    if(!rc)
			ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return NULL;
	}
//	#endif		
}

/**
 * rc_type(ID)
 * return SUB_TYPE | TYPE
 *        < 0 id not found
 */

/*
 * Synopsis    unsigned short rc_type(unsigned short id);
 * Description  讀取 resource[id] 之type
 * Parameters   id	                - res index
 * Return       item 之 SUB_TYPE | TYPE
 */
unsigned short rc_type(unsigned short id)
{
    char rc= 0;
	struct res_t *res = (struct res_t *) (*_fresource);
	//struct res_item_t desc_s;
    int i, n;

    if (res_s.resource_magic != RESOURCE_MAGIC) {
		ERROR(("invalid rc init!\r\n"));
		return NULL;
	}

    n = res_s.nelements;
	if(DEF_SEARCHING) {
		if (id == desc_s.id)
	    	return desc_s.type | (desc_s.subtype << 16);
	#ifdef FIBONACCI_VERSION
		/* Fibonacci searching */
		i= f_searching(n, id);
	#endif
	#ifdef BINERY_VERSION
		/* Binery searching */
		i= b_searching(0, n-1, id);
	#endif
		if(i>=0) {
			if (id == desc_s.id)
	            return desc_s.type | (desc_s.subtype << 16);
		}
		ERROR(("NO ID!\r\n"));
	    return NULL;
	} else {
		for (i = 0;i < n; i++) {
	        rc = spi_dma2xram((unsigned long)&res->desc[i], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
	        if(!rc)
				goto EXIT;
			if (id == desc_s.id)
	            return desc_s.type | (desc_s.subtype << 16);
	    }
		ERROR(("NO ID!\r\n"));
	EXIT:
		if(!rc)
			ERROR(("DMA 2 XRAM FAILE!\r\n"));
	    return NULL;
	}
}

/**
 * rc_len(ID)
 * return length
 */

/*
 * Synopsis     unsigned long rc_len(unsigned short id);
 * Description  讀取 resource[id] 之資料長度
 * Parameters   id	                - res index
 * Return       item 之資料長度
 */
unsigned long rc_len(unsigned short id)
{
    char rc= 0;
	struct res_t *res = (struct res_t *) (*_fresource);
    struct res_item_t desc_s1;
    int i, n;

    if (res_s.resource_magic != RESOURCE_MAGIC) {
		ERROR(("invalid rc init!\r\n"));
		return NULL;
	}

    n = res_s.nelements;
	if(DEF_SEARCHING) {
//	#ifdef DEF_SEARCHING_FUNCTION
	#ifdef FIBONACCI_VERSION
		/* Fibonacci searching */
		i= f_searching(n, id);
	#endif
	#ifdef BINERY_VERSION
		/* Binery searching */
		i= b_searching(0, n-1, id);
	#endif
		if(i>=0) {
			if (id == desc_s.id) {
				rc = spi_dma2xram((unsigned long)&res->desc[i+1], (unsigned short)(&desc_s1), RES_ITEM_T_SIZE);
		        if(!rc)
					goto EXIT;
	
				if(i==(n-1))
					return 16;
				else
					return (unsigned long)(desc_s1.offset - desc_s.offset);
	        }
		}
	} else {
//	#else
		for (i = 0;i < n; i++) {
	        rc = spi_dma2xram((unsigned long)&res->desc[i], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
	        if(!rc)
				goto EXIT;
			if (id == desc_s.id) {
				rc = spi_dma2xram((unsigned long)&res->desc[i+1], (unsigned short)(&desc_s1), RES_ITEM_T_SIZE);
		        if(!rc)
					goto EXIT;
	
				if(i==(n-1))
					return 16;
				else
					return (unsigned long)(desc_s1.offset - desc_s.offset);
	        }
	    }
	}
//	#endif
	ERROR(("NO ID!\r\n"));
EXIT:
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));	
    return NULL;
}

/*
 * Synopsis     void get_res_img (unsigned short id, 
                                  struct res_img_t *img_s);
 * Description  讀取 img item
 * Parameters   id	                - item id
 *              img_s               - img物件
 * Return       none
 */
void get_res_img (unsigned short id, struct res_img_t *img_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_IMG) {
		ERROR(("Resource %u not is TYPE_IMG (1 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(img_s), RES_IMG_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*img_s).lut_addr += (unsigned long)(*_fresource);
	(*img_s).index_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_img_by_addr ( unsigned short id, 
 *                                         struct res_img_t *img_s,
 *                                         unsigned long *addr); 
 * Description  讀取 td item
 * Parameters   id	                - item id
 *              img_s               - td物件
 *              addr                - item位址
 * Return       none
 */
void get_res_img_by_addr (unsigned short id, struct res_img_t *img_s, unsigned long *addr)
{
	char rc= 0;
	unsigned char type= 0;
	
	/* FIRST GET RESOURCE */
	if(*addr==0) {
		type= rc_type(id);
		if(type!=TYPE_IMG) {
			ERROR(("Resource %u not is TYPE_IMG (1 != %bu)!\n", id, type));
			return;
		}
		*addr = rc_data(id);			
	}
	
	rc = spi_dma2xram(*addr, (unsigned short)(img_s), RES_IMG_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*img_s).lut_addr += (unsigned long)(*_fresource);
	(*img_s).index_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_spr ( unsigned short id, 
 *                                 struct res_spr_t *spr_s); 
 * Description  讀取 spr item
 * Parameters   id	                - item id
 *              spr_s               - spr物件
 * Return       none
 */
void get_res_spr (unsigned short id, struct res_spr_t *spr_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);

	if(type!=TYPE_SPRITE) {
		ERROR(("Resource %u not is TYPE_SPRITE (2 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(spr_s), RES_SPR_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*spr_s).lut_addr += (unsigned long)(*_fresource);
	(*spr_s).index_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_emu ( unsigned short id, 
 *                                 struct res_emu_t *emu_s); 
 * Description  讀取 emu item
 * Parameters   id	                - item id
 *              emu_s               - emu物件
 * Return       none
 */
void get_res_emu (unsigned short id, struct res_emu_t *emu_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_EMU) {
		ERROR(("Resource %u not is TYPE_EMU (3 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(emu_s), RES_EMU_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*emu_s).base_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_menu ( unsigned short id, 
 *                                  struct res_menu_t *menu_s); 
 * Description  讀取 menu item
 * Parameters   id	                - item id
 *              menu_s              - menu物件
 * Return       none
 */
void get_res_menu (unsigned short id, struct res_menu_t *menu_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_MENU) {
		ERROR(("Resource %u not is TYPE_MENU (4 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(menu_s), RES_MENU_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*menu_s).base_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_tile ( unsigned short id, 
 *                                  struct res_tile_t *tile_s); 
 * Description  讀取 tile item
 * Parameters   id	                - item id
 *              tile_s              - tile物件
 * Return       none
 */
void get_res_tile (unsigned short id, struct res_tile_t *tile_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_TILE) {
		ERROR(("Resource %u not is TYPE_TILE (5 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(tile_s), RES_TILE_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*tile_s).base_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_lut ( unsigned short id, 
 *                                 struct res_lut_t *lut_s); 
 * Description  讀取 lut item
 * Parameters   id	                - item id
 *              lut_s               - lut物件
 * Return       none
 */
void get_res_lut (unsigned short id, struct res_lut_t *lut_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_LUT) {
		ERROR(("Resource %u not is TYPE_LUT (6 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(lut_s), RES_LUT_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*lut_s).lut_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_oreg ( unsigned short id, 
 *                                  struct res_oreg_t *oreg_s); 
 * Description  讀取 org item
 * Parameters   id	                - item id
 *              oreg_s              - org物件
 * Return       none
 */
void get_res_oreg (unsigned short id, struct res_oreg_t *oreg_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_OREG) {
		ERROR(("Resource %u not is TYPE_OREG (7 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(oreg_s), RES_OREG_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*oreg_s).base_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_icon ( unsigned short id, 
 *                                  struct res_icon_t *icon_s); 
 * Description  讀取 icon item
 * Parameters   id	                - item id
 *              icon_s              - icon物件
 * Return       none
 */
void get_res_icon (unsigned short id, struct res_icon_t *icon_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_ICON) {
		ERROR(("Resource %u not is RES_ICON_T_SIZE (8 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(icon_s), RES_ICON_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*icon_s).base_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_td ( unsigned short id, 
 *                                struct res_td_t *td_s); 
 * Description  讀取 td item
 * Parameters   id	                - item id
 *              td_s                - td物件
 * Return       none
 */
void get_res_td (unsigned short id, struct res_td_t *td_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_TD) {
		ERROR(("Resource %u not is TYPE_TD (9 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(td_s), RES_TD_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*td_s).color_addr += (unsigned long)(*_fresource);
	(*td_s).index_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_td_by_addr ( unsigned short id, 
 *                                        struct res_td_t *td_s,
 *                                        unsigned long *addr); 
 * Description  讀取 td item
 * Parameters   id	                - item id
 *              td_s                - td物件
 *              addr                - item位址
 * Return       none
 */
void get_res_td_by_addr (unsigned short id, struct res_td_t *td_s, unsigned long *addr)
{
	char rc= 0;
	unsigned char type= 0;
	
	/* FIRST GET RESOURCE */
	if(*addr==0) {
		type= rc_type(id);
		if(type!=TYPE_TD) {
			ERROR(("Resource %u not is TYPE_TD (9 != %bu)!\n", id, type));
			return;
		}
		*addr = rc_data(id);			
	}
	
	rc = spi_dma2xram(*addr, (unsigned short)(td_s), RES_TD_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*td_s).color_addr += (unsigned long)(*_fresource);
	(*td_s).index_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_tof ( unsigned short id, 
 *                                 struct res_tof_t *tof_s); 
 * Description  讀取 tof item
 * Parameters   id	                - item id
 *              tof_s               - tof物件
 * Return       none
 */
void get_res_tof (unsigned short id, struct res_tof_t *tof_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_TOFONT) {
		ERROR(("Resource %u is not TYPE_TOFONT (8 != %bu)!\n", id, type));
		return;
	}
																		  
	rc = spi_dma2xram(rc_data(id), (unsigned short)(tof_s), RES_TOF_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	//printf_tw("addr = %08lX, res_addr = %08lX\n", (*tof_s).base_addr, (unsigned long)*_fresource);

	(*tof_s).base_addr += (unsigned long)(*_fresource);
}

/*
 * Synopsis     void get_res_ch_icon ( unsigned short id, 
 *                                     struct res_ch_icon_t *ch_icon_s); 
 * Description  讀取 ch_icon item
 * Parameters   id	                - item id
 *              ch_icon_s           - ch_icon物件
 * Return       none
 */
void get_res_ch_icon (unsigned short id, struct res_ch_icon_t *ch_icon_s)
{
	char rc= 0;
	unsigned char type= rc_type(id);
	
	if(type!=TYPE_CH_ICON) {
		ERROR(("Resource %u not is TYPE_CH_ICON (11 != %bu)!\n", id, type));
		return;
	}
	
	rc = spi_dma2xram(rc_data(id), (unsigned short)(ch_icon_s), RES_CH_ICON_T_SIZE);
	if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return;
	}

	(*ch_icon_s).base_addr += (unsigned long)(*_fresource);
}


#ifdef TW_RC_DEBUG_MODE
/*
 * Synopsis     void show_img_info (unsigned long fp);
 * Description  打印img物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_img_info (unsigned long fp)
{
	char rc= 0;
	struct res_img_t img_s;

	rc = spi_dma2xram(fp, (unsigned short)(&img_s), RES_IMG_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	img_s.lut_addr += (unsigned long)(*_fresource);
	img_s.index_addr += (unsigned long)(*_fresource);

	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : SOSD IMAGE\r\n");
	DBG_PRINT("lut_addr  : 0x%lX\r\n", img_s.lut_addr);
	DBG_PRINT("index_addr: 0x%lX\r\n", img_s.index_addr);
	DBG_PRINT("line jump : %u\r\n", img_s.line_jump);
	DBG_PRINT("width     : %u\r\n", img_s.width);
	DBG_PRINT("height    : %u\r\n", img_s.height);
	DBG_PRINT("\r\n");
}

/*
 * Synopsis     void show_spr_info (unsigned long fp);
 * Description  打印spr物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_spr_info (unsigned long fp)
{
	char rc= 0;
	struct res_spr_t spr_s;

	rc = spi_dma2xram(fp, (unsigned short)(&spr_s), RES_SPR_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	spr_s.lut_addr += (unsigned long)(*_fresource);
	spr_s.index_addr += (unsigned long)(*_fresource);
	
	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : SOSD SPRITE\r\n");
	DBG_PRINT("lut_addr  : 0x%lX\r\n", spr_s.lut_addr);
	DBG_PRINT("index_addr: 0x%lX\r\n", spr_s.index_addr);
	DBG_PRINT("line jump : %u\r\n", spr_s.line_jump);
	DBG_PRINT("width     : %bu\r\n", spr_s.width);
	DBG_PRINT("height    : %u\r\n", spr_s.height);
	DBG_PRINT("count     : %bu\r\n", spr_s.count);
	DBG_PRINT("\r\n");
}

/*
 * Synopsis     void show_emu_info (unsigned long fp);
 * Description  打印emu物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_emu_info (unsigned long fp)
{
	char rc= 0;
	struct res_emu_t emu_s;

	rc = spi_dma2xram(fp, (unsigned short)(&emu_s), RES_EMU_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	emu_s.base_addr += (unsigned long)(*_fresource);

	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : SOSD EMULATION\r\n");
	DBG_PRINT("base_addr : 0x%lX\r\n", emu_s.base_addr);
	DBG_PRINT("length    : %u\r\n", emu_s.length);
	DBG_PRINT("repeat    : %bu\r\n", emu_s.repeat);
	DBG_PRINT("\r\n");
}

/*
 * Synopsis     void show_menu_info (unsigned long fp);
 * Description  打印menu物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_menu_info (unsigned long fp)
{
	char rc= 0;
	struct res_menu_t menu_s;

	rc = spi_dma2xram(fp, (unsigned short)(&menu_s), RES_MENU_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	menu_s.base_addr += (unsigned long)(*_fresource);

	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : OSD2 MENU\r\n");
	DBG_PRINT("base_addr : 0x%lX\r\n", menu_s.base_addr);
	DBG_PRINT("length    : %u\r\n", menu_s.length);
	DBG_PRINT("\r\n");
}

/*
 * Synopsis     void show_tile_info (unsigned long fp);
 * Description  打印tile物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_tile_info (unsigned long fp)
{
#ifdef NEW_TOOL_VERSION_RC
	char rc= 0;
	struct res_tile_t tile_s;
	unsigned short temp= 0;
	unsigned short i= 0;

	rc = spi_dma2xram(fp, (unsigned short)(&tile_s), RES_TILE_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	tile_s.base_addr += (unsigned long)(*_fresource);
	
	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : OSD2 TILE\r\n");
	DBG_PRINT("base_addr : 0x%lX\r\n", tile_s.base_addr);
	DBG_PRINT("length    : %u\r\n", tile_s.length);
	DBG_PRINT("element   : ");
	for(i= 0; i<tile_s.length; i++) {
		spi_dma2xram((tile_s.base_addr+(i*2)), (unsigned short)(&temp), sizeof(unsigned short));
		DBG_PRINT("%u, ", temp);		
	}
	DBG_PRINT("\r\n");
	DBG_PRINT("\r\n");
#else
	char rc= 0;
	struct res_tile_t tile_s;

	rc = spi_dma2xram(fp, (unsigned short)(&tile_s), RES_TILE_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : OSD2 TILE\r\n");
	DBG_PRINT("base_addr : 0x%lX\r\n", tile_s.base_addr);
	DBG_PRINT("length    : %u\r\n", tile_s.length);
	DBG_PRINT("\r\n");
#endif
}

/*
 * Synopsis     void show_ch_icon_info (unsigned long fp);
 * Description  打印ch_icon物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_ch_icon_info (unsigned long fp)
{
	char rc= 0;
	struct res_ch_icon_t ch_icon_s;
	unsigned char i= 0;
	unsigned short temp= 0;

	rc = spi_dma2xram(fp, (unsigned short)(&ch_icon_s), RES_CH_ICON_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	ch_icon_s.base_addr += (unsigned long)(*_fresource);
	
	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : OSD2 change ICON\r\n");
	DBG_PRINT("base_addr : 0x%lX\r\n", ch_icon_s.base_addr);
	DBG_PRINT("length    : %u\r\n", ch_icon_s.length);
	DBG_PRINT("element   : ");
	for(i= 0; i<ch_icon_s.length; i++) {
		spi_dma2xram((ch_icon_s.base_addr+(i*2)), (unsigned short)(&temp), sizeof(unsigned short));
		DBG_PRINT("%u, ", temp);		
	}
	DBG_PRINT("\r\n\r\n");
}

/*
 * Synopsis     void show_lut_info (unsigned long fp);
 * Description  打印lut物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_lut_info (unsigned long fp)
{
	char rc= 0;
	struct res_lut_t lut_s;

	rc = spi_dma2xram(fp, (unsigned short)(&lut_s), RES_LUT_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	lut_s.lut_addr += (unsigned long)(*_fresource);
	
	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : OSD2 LUT\r\n");
	DBG_PRINT("lut_addr  : 0x%lX\r\n", lut_s.lut_addr);
	DBG_PRINT("main_len  : %u\r\n", lut_s.main_len);
	DBG_PRINT("sec_len   : %bu\r\n", lut_s.sec_len);
	DBG_PRINT("re_2bp_len: %bu\r\n", lut_s.re_2bp_len);
	DBG_PRINT("re_bg_len : %bu\r\n", lut_s.re_bg_len);
	DBG_PRINT("\r\n");
}

/*
 * Synopsis     void show_oreg_info (unsigned long fp);
 * Description  打印oreg物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_oreg_info (unsigned long fp)
{
	char rc= 0;
	struct res_oreg_t oreg_s;

	rc = spi_dma2xram(fp, (unsigned short)(&oreg_s), RES_OREG_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	oreg_s.base_addr += (unsigned long)(*_fresource);
	
	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : OSD2 REG TABLE\r\n");
	DBG_PRINT("base_addr : 0x%lX\r\n", oreg_s.base_addr);
	DBG_PRINT("length    : %bu\r\n", oreg_s.length);
	DBG_PRINT("\r\n");
}

/*
 * Synopsis     void show_icon_info (unsigned long fp);
 * Description  打印icon物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_icon_info (unsigned long fp)
{
	char rc= 0;
	struct res_icon_t icon_s;

	rc = spi_dma2xram(fp, (unsigned short)(&icon_s), RES_ICON_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	icon_s.base_addr += (unsigned long)(*_fresource);
	
	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : OSD2 ICON TABLE\r\n");
	DBG_PRINT("base_addr : 0x%lX\r\n", icon_s.base_addr);
	DBG_PRINT("font_w	 : %bu\r\n", icon_s.font_w);
	DBG_PRINT("font_h    : %bu\r\n", icon_s.font_h);
	DBG_PRINT("width     : %bu\r\n", icon_s.width);
	DBG_PRINT("height    : %bu\r\n", icon_s.height);
	DBG_PRINT("bp    	 : %bu\r\n", icon_s.bp);
	DBG_PRINT("count     : %bu\r\n", icon_s.count);
	DBG_PRINT("\r\n");
}

/*
 * Synopsis     void show_td_info (unsigned long fp);
 * Description  打印td物件資?
 * Parameters   fp	                - item地址
 * Return       none
 */
void show_td_info (unsigned long fp)
{
	char rc= 0;
	struct res_td_t td_s;

	rc = spi_dma2xram(fp, (unsigned short)(&td_s), RES_TD_T_SIZE);
	if(!rc)
		ERROR(("DMA 2 XRAM FAILE!\r\n"));

	td_s.color_addr += (unsigned long)(*_fresource);
	td_s.index_addr += (unsigned long)(*_fresource);
	
	DBG_PRINT("[resource header]\n");
	DBG_PRINT("type      : SOSD TD\r\n");
	DBG_PRINT("color_addr: 0x%lX\r\n", td_s.color_addr);
	DBG_PRINT("index_addr: 0x%lX\r\n", td_s.index_addr);
	DBG_PRINT("line jump : %u\r\n", td_s.line_jump);
	DBG_PRINT("width     : %u\r\n", td_s.width);
	DBG_PRINT("height    : %u\r\n", td_s.height);
	DBG_PRINT("\r\n");
}
#endif

/*
 * Synopsis     void show_resource_header (unsigned short res_id);
 * Description  打印物件資?
 * Parameters   res_id	                - 物件id
 * Return       none
 */
void show_resource_header (unsigned short res_id)
{
	unsigned long fp;
	unsigned short type= 0;
	
	type = rc_type(res_id);

	DBG_PRINT("rc_id   : %u\n", res_id);
	DBG_PRINT("rc_type   : %d\n", type);
	DBG_PRINT("rc_len    : %ld bytes\n", rc_len(res_id));

	fp = rc_data(res_id);

#ifdef TW_RC_DEBUG_MODE
	switch(type) {
		case TYPE_IMG:
			show_img_info(fp);
			break;
		
		case TYPE_SPRITE:
			show_spr_info(fp);
			break;
		
		case TYPE_EMU:
			show_emu_info(fp);
			break;
		
		case TYPE_MENU:
			show_menu_info(fp);
			break;

		case TYPE_TILE:
			show_tile_info(fp);
			break;
		
		case TYPE_LUT:
			show_lut_info(fp);
			break;

		case TYPE_OREG:
			show_oreg_info(fp);
			break;
		
		case TYPE_ICON:
			show_icon_info(fp);
			break;

		case TYPE_TD:
			show_td_info(fp);
			break;	
		
		default:
			ERROR(("type error!\r\n"));
			break;
	}
#endif

}

#ifndef OLD_TOOL_COMBINE
/**
 * get_res_id(e)
 * return res ID
 *        = 0 error
 */

/*
 * Synopsis     unsigned short get_res_id(unsigned short e);
 * Description  讀取 res_item[e] 之id資?
 * Parameters   e	                - res_item index
 * Return       res_item id, 0 ->error
 */
unsigned short get_res_id(unsigned short e)
{
    char rc= 0;
	struct res_t *res = (struct res_t *) (*_fresource);
	struct res_item_t desc_s;
    unsigned short n;

    if (res_s.resource_magic != RESOURCE_MAGIC) {
		ERROR(("invalid rc init!\r\n"));
		return 0;
	}

    n = res_s.nelements;

	if((e>n)||(e==0)) {
		ERROR(("invalid number of elements!\r\n"));
		return 0;
	}

	rc = spi_dma2xram((unsigned long)&res->desc[e-1], (unsigned short)(&desc_s), RES_ITEM_T_SIZE);
    if(!rc) {
		ERROR(("DMA 2 XRAM FAILE!\r\n"));
		return 0;
	}
	
	return desc_s.id;
}
#endif

/*
 * Synopsis     void resource_init (void);
 * Description  resource系統初始化
 * Return       none
 */
void resource_init (void)
{
	unsigned short i=0;
	unsigned short n=0;

	int rc= 0;

	rc = rc_init(&n);
	
	if (rc==-1)
		ERROR(("invalid magic!\r\n"));
	else if (rc==-2)
		ERROR(("invalid number of elements!\r\n"));
	else if (rc==-3)
		ERROR(("invalid pack struct!\r\n"));
	else if (rc==-4)
		ERROR(("invalid element struct!\r\n"));
	else if (rc==-5)
		ERROR(("invalid dma 2 xram!\r\n"));
//	else {
//		ERROR(("rc init faile!\r\n"));	
//		return;
//	}

	if (n>0) {
		DBG_PRINT("rc init success!\r\n");
		DBG_PRINT("number of elements is %d \r\n", n);

#ifdef DBG_RES_INFO
	#ifdef OLD_TOOL_COMBINE
		for(i=0; i<n; i++)
			show_resource_header(i);
	#else
		for(i=1; i<=n; i++)
			show_resource_header(get_res_id(i));
	#endif
#endif
	} else
		ERROR(("no resource elements!\r\n"));	
}
