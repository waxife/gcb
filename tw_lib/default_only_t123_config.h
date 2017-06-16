/**
 *  @file   config.h
 *  @brief  library configure for only T123
 *  $Id: default_only_t123_config.h,v 1.6 2013/09/13 08:34:42 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.6 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#define	ENABLE	1
#define DISABLE 0

/*
 * CONFIG ENABLE/DISABLE FUNCTION
 */

/* FOR FPGA TEST */
//#define FPGA_VERSION

/* ENABLE EXTERNAL MCU */
#define ENABLE_EX_MCU

#ifndef ENABLE_EX_MCU

	/* INTERRUPT ENABLE/DISABLE */
	
	#define ALL_INTERRUPT		ENABLE
	#define EXTERNAL0_INTERRUPT	ENABLE
	#define SERIAL_INTERRUPT	DISABLE
	#define TIMER0_INTERRUPT	DISABLE
	#define TIMER1_INTERRUPT	DISABLE
	
	/* ENABLE BANK SWITCH */
	#define ENABLE_BANK_SWITCH
	
	/* ENABLE DEBUG MODE */
	#define DEBUG_MODE

	/* SERIAL PORT CONFIG for HW UART (i8051 use) */
	#define HW_UART_BAUD		38400L
	#define HW_UART1_BAUD		38400L
	
	/* ENABLE UART1 OF AUTO FLOW CONTROL */
	//#define ENABLE_UART1_AFC
	
	/* ENABLE EXTERNAL I2C SAFE MODE */
	#define EX_I2C_SAFE_MODE
		#ifdef EX_I2C_SAFE_MODE	
			#define SLAVE_SAFE_ADDR	0x40
			#define SLAVE_WORK_ADDR	0x50
		#endif

	/* ENABLE QUAD I/O FOR QUAD NOR FLASH */
	#define ENABLE_QUAD_IO
	
	/* ENABLE 1X SPIOSD DCLK (DEFAULT SETTING IS 2X) */
	//#define SPIOSD_DCLK_1X
	
	/* COMMAND QUEUE INTERRUPT SETTING */
	#define CQ_ENABLE_INT

	/* I8051 WRITE ORAM DELAY SETTING */
	#define I51_W_ORAM_DELAY	20
	
	
	/* IR SYSTEM CODE */
	#define IR_SYS_CODE		0xFF00
	
	
	/* SAR KEY THRESHOLD VALUE */
	#define SARKEY_TVAL1	0xD6
	#define SARKEY_TVAL2 	0x57
	#define SARKEY_TVAL3 	0xe9
	#define SARKEY_TVAL4 	0x80
	#define SARKEY_TVAL5 	0xAB
	#define SARKEY_TVAL6 	0x2A
	
	
	/* NOR FLASH LAYOUT INFO ADDRESS */
	#define LAYOUT_INFO_ADDR 		0x100L
	#define LAYOUT_MAGIC_ADDR		LAYOUT_INFO_ADDR
	#define LAYOUT_TEXTSTART_ADDR	LAYOUT_INFO_ADDR+4L
	#define LAYOUT_TEXTSIZE_ADDR	LAYOUT_INFO_ADDR+8L
	#define LAYOUT_USERSTART_ADDR	LAYOUT_INFO_ADDR+12L
	#define LAYOUT_USERSIZE_ADDR	LAYOUT_INFO_ADDR+16L
	#define LAYOUT_RESSTART_ADDR	LAYOUT_INFO_ADDR+20L
	#define LAYOUT_RESSIZE_ADDR		LAYOUT_INFO_ADDR+24L
	
	#define LAYOUT_TP_LEFT_ADDR		LAYOUT_INFO_ADDR+28L
	#define LAYOUT_TP_RIGHT_ADDR	LAYOUT_INFO_ADDR+30L
	#define LAYOUT_TP_UP_ADDR		LAYOUT_INFO_ADDR+32L
	#define LAYOUT_TP_DOWN_ADDR		LAYOUT_INFO_ADDR+34L
	#define LAYOUT_TP_FINISH_ADDR	LAYOUT_INFO_ADDR+36L
	
	
	/* REGISTRY NOR LAYOUT PARAMETERS */
	//#define REGISTRY_SECTION_BASE   0x10000L /* 64K, NEED TO MODIFY FOR REALITY */
	//#define REGISTRY_SECTION_SIZE   4096
	#define REGISTRY_SLOT_SIZE      272      /* if size > REGISTRY_SIZE */
	#define REGISTRY_ITEMS          15       /* 4096/272 = 15 */

	/* I2C DRIVERS DEFINE */
	#include "i51_i2c_drivers.h"
	#define IC_WritByte			iIC_WritByte
	#define IC_ReadByte			iIC_ReadByte
	#define IC_WritByte_INT		iIC_WritByte
	#define IC_ReadByte_INT		iIC_ReadByte
	#define	twdDelay			itwdDelay

	/* CHIP CONFIG. REGISTERS PAGE */
	#define PAGE_OFFSET				0x100 

	#define TWIC_P0   				0x00	  
	#define TWIC_P1   				TWIC_P0 + PAGE_OFFSET
	#define TWIC_P2   				TWIC_P1 + PAGE_OFFSET
	#define TWIC_P3					TWIC_P2 + PAGE_OFFSET
	#define TWIC_P4					TWIC_P3 + PAGE_OFFSET
	#define TWIC_P5					TWIC_P4 + PAGE_OFFSET
	#define TWIC_P6					TWIC_P5 + PAGE_OFFSET
	
#else	/* NOT DEF ENABLE_EX_MCU */
	/* ENABLE DEBUG MODE */
	#define DEBUG_MODE

	/* RECIVER INTERUPPT FORM T1XX CPUINT */
	//#define ENABLE_RECIVER_CPUINT

	/* COMMAND QUEUE INTERRUPT SETTING */
	#ifdef	ENABLE_RECIVER_CPUINT 	
		#define CQ_ENABLE_INT
	#endif
	
	/* I2C DRIVERS DEFINE */
	#include "LIB_I2C.h"
	#define IC_WritByte			I2CWriteByte
	#define IC_ReadByte			I2CReadByte
	#define IC_WritByte_INT		I2CWriteByte_INT	// I2CWriteByte
	#define IC_ReadByte_INT		I2CReadByte_INT		// I2CReadByte
	#include "global.h"
	#define	twdDelay(x)		twExDelay(x/10)

	#define Burst_A_MCU 	I2CBurst_A
	#define Burst_D_MCU 	I2CBurst_D
	#define Burst_P_MCU 	I2CBurst_P
	
	/* USE RESOURCE FORMATE ON NOR FLASH */
	//#define RESOURCE_ON_NOR_FLASH

	#ifdef RESOURCE_ON_NOR_FLASH
		/* USE SPIOSD */
		#define ENABLE_EXMCU_USE_SPIOSD
	
		#ifdef ENABLE_EXMCU_USE_SPIOSD 
		/* ENABLE QUAD I/O FOR QUAD NOR FLASH */
			#define ENABLE_QUAD_IO
		/* ENABLE 1X SPIOSD DCLK (DEFAULT SETTING IS 2X) */
			//#define SPIOSD_DCLK_1X
		#endif
	#endif

    /* CHIP CONFIG. REGISTERS PAGE */
    #define PAGE_OFFSET				0x02 

    #define TWIC_P0   				0x50	  
	#define TWIC_P1   				TWIC_P0 + PAGE_OFFSET
	#define TWIC_P2   				TWIC_P1 + PAGE_OFFSET
	#define TWIC_P3					TWIC_P2 + PAGE_OFFSET
	#define TWIC_P4					TWIC_P3 + PAGE_OFFSET
	#define TWIC_P5					TWIC_P4 + PAGE_OFFSET
	#define TWIC_P6					TWIC_P5 + PAGE_OFFSET	

#endif

	/* QUAD NOR FLASH PARAMETERS */
	#ifdef ENABLE_QUAD_IO
		#define MXIC_QUAD_NOR	/* MXIC MX25LXXXX */
		//#define EON_QUAD_NOR	/* EON EN25QXX */
		//#define WIN_QUAD_NOR	/* WINBOND W25QXXXX */
		//#define AMIC_QUAD_NOR	/* AMIC A25LQXXX */
		//#define P_QUAD_NOR	/* P PM25LQXXX */
		//#define GD_128M_QUAD_NOR /* GD25Q128XXX */
		//#define WQ_25Q_128_QUAD_NOR /* W25Q128XXX */
		
		#ifdef MXIC_QUAD_NOR
	        // p7!=p3, p6!=p2, p5!=p1, p4!=p0 --> high performance mode
			#define SUPPORT_HPM
			#define HPM_TOGGLE_VALUE	0xA5
			#define NONE_TOGGLE_VALUE	0xCC
		#endif /* END OF MAXIC_QUAD_NOR */
		#ifdef EON_QUAD_NOR
			#define SUPPORT_HPM
			#define HPM_TOGGLE_VALUE	0xAA
			#define NONE_TOGGLE_VALUE	0x55
		#endif /* END OF EON_QUAD_NOR */
		#ifdef WIN_QUAD_NOR
			#define SUPPORT_HPM
			#define HPM_TOGGLE_VALUE	0xAA
			#define NONE_TOGGLE_VALUE	0x55
			//#define WIN_PROTECT_STATUS	//This will cause flash use "Power Supply Lock-Down"
		#endif /* END OF WIN_QUAD_NOR */
		#ifdef AMIC_QUAD_NOR
			#define SUPPORT_HPM
			#define HPM_TOGGLE_VALUE	0xAA
			#define NONE_TOGGLE_VALUE	0x55
		#endif /* END OF AMIC_QUAD_NOR */
		#ifdef P_QUAD_NOR
			#define SUPPORT_HPM
			#define HPM_TOGGLE_VALUE	0xAA
			#define NONE_TOGGLE_VALUE	0x55
		#endif /* END OF P_QUAD_NOR */
		#ifdef GD_128M_QUAD_NOR
			#define SUPPORT_HPM
			#define HPM_TOGGLE_VALUE	0xAA
			#define NONE_TOGGLE_VALUE	0xCC
			//#define GD_128M_STATUS_PROTECT	//This will cause flash use "Power Supply Lock-Down"
		#endif /* END OF GD_128M_QUAD_NOR */
		#ifdef WQ_25Q_128_QUAD_NOR
			#define SUPPORT_HPM
			#define HPM_TOGGLE_VALUE	0xAA
			#define NONE_TOGGLE_VALUE	0xCC
			//#define WQ_128M_STATUS_PROTECT	//This will cause flash use "Power Supply Lock-Down"
		#endif /* END OF WQ_25Q_128_QUAD_NOR */
	#else
		#define HPM_TOGGLE_VALUE	0xAA
		#define NONE_TOGGLE_VALUE	0x55
	#endif


#endif /* _CONFIG_H */
