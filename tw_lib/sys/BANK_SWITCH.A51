/**
 *  @file   BANK_SWITCH.A51
 *  @brief  bank switch macros for T123
 *  $Id: BANK_SWITCH.A51,v 1.8 2013/08/09 05:52:58 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.8 $
 *
 *  Copyright (c) 2013 Terawins Inc. All rights reserved.
 *
 *  @date   2011/08/12  ken 	New file.
 *
 */

$NOMOD51 NOLINES
$NOCOND
;------------------------------------------------------------------------------
;  This file is part of the BL51 / LX51 Banked Linker/Locater package
;  Copyright (c) 1988 - 2001 Keil Elektronik GmbH and Keil Software, Inc.
;  Version 2.21 (Code and Variable Banking for Classic 8051 Derivatives)
;------------------------------------------------------------------------------
;************************ Configuration Section *******************************
?B_NBANKS       EQU  16   ;  Dfine maximum Number of Banks                    *
;                         ; following values are allowed: 2, 4, 8, 16, 32, 64 *
;                         ; for BL51 the maximum value for ?B_BANKS is 32     *
;                         ; for LX51 the maximum value for ?B_BANKS is 64     *
;                                                                             *
?B_MODE         EQU  4    ; 0 for Bank-Switching via 8051 Port                *
;                         ; 1 for Bank-Switching via XDATA Port               *
;                         ; 4 for user-provided bank switch code              *
;                                                                             *
?B_RTX          EQU  0    ; 0 for applications without real-time OS           *
;                         ; 1 for applications using the RTX-51 real-time OS  *
;                                                                             *
?B_VAR_BANKING  EQU  0    ; Variable Banking via L51_BANK (far memory support)*
;                         ; 0 Variable Banking does not use L51_BANK.A51      *
;                         ; 1 Variable Banking uses this L51_BANK.A51 module  *
; Notes: ?B_VAR_BANKING uses the 'far' and 'far const' C51 memory types to    *
;        extent the space for variables in RAM and/or ROM of classic 8051     *
;        device.  The same hardware as for code banking is used.  Program     *
;        code banking and variable banking share the same hardware I/O pins.  *
;        The C51 Compiler must be used with the VARBANKING directive.         *
;        Variable Banking is only supported with the LX51 linker/locater.     *
;                                                                             *
?B_RST_BANK     EQU  0x00 ; specifies the active code bank number after CPU   *
;                         ; Reset.  Used to reduce the entries in the         *
;                         ; INTERBANK CALL TABLE.  The value 0xFF disables    *
;                         ; this LX51 linker/locater optimization.            *
; Note:  Interbank Call Table optimization is only possible with LX51.        *
;                                                                             *
;-----------------------------------------------------------------------------*
;                                                                             *
;		                                                                      *
;                                                                             *
IF  ?B_MODE = 4;                                                              *
;-----------------------------------------------------------------------------*
; if ?BANK?MODE is 4 define the following switch macros                       *
; For bank switching via user-provided bank switch code you must define for   *
; each memory bank a own macro which contains the bank switch code.  The      *
; following example shows how to use the I/O lines P1.4 and P1.7 for bank     *
; switching.  Since you can select just 4 banks with two address lines, just  *
; four macros are defined.  The number of macros must conform with the number *
; ?B_NBANKS number, i.e. for an application with 16 memory banks you must     *
; define 16 macros.                                                           *
;                                                                             *
; IMPORTANT NOTES:                                                            *
; 1. The bank switch logic must be initialized before using it.  Therefore    *
;    add the following lines of code at the end of the STARTUP.A51 file:      *
;                :                                                            *
;      EXTRN CODE (?B_SWITCH0)                                                *
;               CALL    ?B_SWITCH0    ; init bank mechanism to code bank 0    *
;               LJMP    ?C_START      ; line already exits at the end of file *
;                :                                                            *
;                                                                             *
; 2. If the bank switch macros and the additional control code generate more  *
;    than 256 bytes, you need to set the LONG_MACRO flag below.  The error    *
;    message "BANK SWITCH CODE BIGGER THAN 256 BYTES, SET LONG_MACRO TO 1"    *
;    is generated in case that this is required.                              *
;                                                                             *
; 3. The only registers that can be modified in this routines without prior   *
;    saving are:  DPTR and ACC.                                               *
;                                                                             *
;                                                                             *
LONG_MACRO      EQU  1    ; 0 default, for normal macros and up to 8 banks    *
;                         ; 1 big macro code or many banks                    *
;                                                                             *
;                                                                             *
;                                                                             *        
SWITCH0         MACRO            ; Switch to Memory Bank #0                
;         Registers[0xF3AA]=0x40;	//Bank3_Offset[15:8]
				MOV		DPTR,#0xF3AA
				MOV		A,#0x40
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x00;	//Bank3_Offset[23:16] 
				CLR		A
				INC		DPTR
				MOVX	@DPTR,A
                ENDM             

SWITCH1         MACRO            ; Switch to Memory Bank #1     
;         Registers[0xF3AA]=0x80;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x80
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x00;	//Bank3_Offset[23:16] 
				CLR		A
				INC		DPTR
				MOVX	@DPTR,A               
				ENDM 

SWITCH2         MACRO            ; Switch to Memory Bank #2     
;         Registers[0xF3AA]=0xC0;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0xC0
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x00;	//Bank3_Offset[23:16] 
				CLR		A
				INC		DPTR
				MOVX	@DPTR,A              
				ENDM         
 
 SWITCH3         MACRO            ; Switch to Memory Bank #3     
;         Registers[0xF3AA]=0x00;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x00
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x01;	//Bank3_Offset[23:16] 
				MOV     A,#0x01
				INC		DPTR
				MOVX	@DPTR,A           
				ENDM

SWITCH4         MACRO            ; Switch to Memory Bank #4     
;         Registers[0xF3AA]=0x40;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x40
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x01;	//Bank3_Offset[23:16] 
				MOV     A,#0x01
				INC		DPTR
				MOVX	@DPTR,A             
				ENDM

SWITCH5         MACRO            ; Switch to Memory Bank #5     
;         Registers[0xF3AA]=0x80;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x80
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x01;	//Bank3_Offset[23:16] 
				MOV     A,#0x01
				INC		DPTR
				MOVX	@DPTR,A            
				ENDM
				
SWITCH6         MACRO            ; Switch to Memory Bank #6     
;         Registers[0xF3AA]=0xc0;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0xc0
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x01;	//Bank3_Offset[23:16] 
				MOV     A,#0x01
				INC		DPTR
				MOVX	@DPTR,A              
				ENDM

SWITCH7         MACRO            ; Switch to Memory Bank #7     
;         Registers[0xF3AA]=0x00;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x00
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x02;	//Bank3_Offset[23:16] 
				MOV     A,#0x02
				INC		DPTR
				MOVX	@DPTR,A              
				ENDM

SWITCH8         MACRO            ; Switch to Memory Bank #8
;         Registers[0xF3AA]=0x40;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x40
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x02;	//Bank3_Offset[23:16] 
				MOV     A,#0x02
				INC		DPTR
				MOVX	@DPTR,A             
				ENDM

SWITCH9         MACRO            ; Switch to Memory Bank #9
;         Registers[0xF3AA]=0x80;	//Bank3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x80
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x02;	//Bank3_Offset[23:16] 
				MOV     A,#0x02
				INC		DPTR
				MOVX	@DPTR,A
				ENDM

SWITCH10        MACRO            ; Switch to Memory Bank #10
;         Registers[0xF3AA]=0xc0;	//Ba8k3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0xc0
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x02;	//Bank3_Offset[23:16] 
				MOV     A,#0x02
				INC		DPTR
				MOVX	@DPTR,A             
				ENDM  

SWITCH11        MACRO            ; Switch to Memory Bank #10
;         Registers[0xF3AA]=0x00;	//Ba8k3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x00
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x03;	//Bank3_Offset[23:16] 
				MOV     A,#0x03
				INC		DPTR
				MOVX	@DPTR,A              
				ENDM 

SWITCH12        MACRO            ; Switch to Memory Bank #10
;         Registers[0xF3AA]=0x40;	//Ba8k3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x40
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x03;	//Bank3_Offset[23:16] 
				MOV     A,#0x03
				INC		DPTR
				MOVX	@DPTR,A               
				ENDM
				
SWITCH13        MACRO            ; Switch to Memory Bank #10
;         Registers[0xF3AA]=0x80;	//Ba8k3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x80
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x03;	//Bank3_Offset[23:16] 
				MOV     A,#0x03
				INC		DPTR
				MOVX	@DPTR,A               
				ENDM
				
SWITCH14        MACRO            ; Switch to Memory Bank #10
;         Registers[0xF3AA]=0xc0;	//Ba8k3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0xc0
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x03;	//Bank3_Offset[23:16] 
				MOV     A,#0x03
				INC		DPTR
				MOVX	@DPTR,A               
				ENDM

SWITCH15        MACRO            ; Switch to Memory Bank #10
;         Registers[0xF3AA]=0x00;	//Ba8k3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x00
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x04;	//Bank3_Offset[23:16] 
				MOV     A,#0x04
				INC		DPTR
				MOVX	@DPTR,A              
				ENDM
				
SWITCH16        MACRO            ; Switch to Memory Bank #10
;         Registers[0xF3AA]=0x40;	//Ba8k3_Offset[15:8] 
				MOV		DPTR,#0xF3AA
				MOV		A,#0x40
				MOVX	@DPTR,A
;         Registers[0xF3AB]=0x02;	//Bank3_Offset[23:16] 
				MOV     A,#0x04
				INC		DPTR
				MOVX	@DPTR,A              
				ENDM
 				
;                                            								  *
;                                                                             *
;-----------------------------------------------------------------------------*
ENDIF;                                                                        *
;									      *
IF ?B_VAR_BANKING = 1;                                                        *							        
;									      * 
XMEM EQU 0x02000000       ; LX51 xdata symbol offset: do not change!	      *
;									      *
;******* Configuration Section for uVision2 Memory Simulation Support *********
;                                                                             *
; The following settings allow you to map the physical xdata and code memory  *
; banks into simulation memory of the uVision2 Simulator.                     *
;                                                                             *
?B?XSTART EQU 0x8000      ; Start of xdata bank area                          *
?B?XEND   EQU 0xFFFF      ; Stop of xdata bank area                           *
?B?XMEM   EQU XMEM+0x010000  ; First HDATA memory bank in xdata space         *
;                                                                             *
; The above setting redirects the symbols in the area X:0x20000 .. X:0x2FFFF  *
; into the uVision2 simulation memory area for the EEPROM  V:0 .. V:0xFFFF    *
;                                                                             *
;-----------------------------------------------------------------------------*
;                                                                             *
                PUBLIC ?B?XSTART, ?B?XEND, ?B?XMEM;                           *
ENDIF;                                                                        *
;                                                                             *
;******************************************************************************
;                                                                             *
; THEORY OF OPERATION                                                         *
; -------------------                                                         *
; The section below describes the code generated by BL51 or LX51 and the      *
; operation of the L51_BANK.A51 module.  BL51/LX51 generates for each         *
; function that is located in a code memory bank and called from the common   *
; area or a different code bank and entry into the INTRABANK CALL TABLE.  The *
; INTRABANK CALL TABLE is located in the SEGMENT ?BANK?SELECT and listed in   *
; the Linker MAP file. The entries in that TABLE have the following format:   *
;                                                                             *
;   ?FCT?1:  MOV  DPTR,#FCT     ; Load Address of target FCT                  *
;            JMP  ?B_BANKn      ; Switch to Bank and Jump to Target Code      *
;                                                                             *
; Instead of directly calling the function FCT, the Linker changes the entry  *
; to ?FCT?1.  This entry selects the bank where the function FCT is located   *
; and calls that function via the routines defined in this L51_BANK.A51 file. *
; The L51_BANK.A51 file contains two sets of functions for each bank:         *
;                                                                             *
; ?B_BANKn    is a routine which saves the entry of the ?B_SWITCHn function   *
;             for the current active bank on the STACK and switches to the    *
;             bank 'n'.  Then it jumps to the address specified by the DPTR   *
;             register.  It is allowed to modify the following registers in   *
;             the ?B_BANKn routine:  A, B, R0, DPTR, PSW                      *
;                                                                             *
; ?B_SWITCHn  is a function which selects the bank 'n'.  This function is     *
;             used at the end of a user function to return to the calling     *
;             code bank.  Only the following registers may be altered in the  *
;             ?B_SWITCHn function:  R0, DPTR                                  *
;                                                                             *
; The current active bank is stored in ?B_CURRENTBANK.  RTX-51 uses this      *
; variable to restore the code bank after a task switch.  To get correct      *
; results, ?B_CURRENTBANK must be set to the code bank before the hardware    *
; switch is done, or the code banking sequences must be interrupt protected.  *
;******************************************************************************

                NAME    ?BANK?SWITCHING

                PUBLIC  ?B_NBANKS, ?B_MODE, ?B_CURRENTBANK, ?B_MASK
                PUBLIC  ?B_FACTOR, ?B_RST_BANK
IF (?B_RTX = 1)
                PUBLIC  ?B_RESTORE_BANK
ENDIF

; Standard SFR Symbols required in L51_BANK.A51
ACC     DATA    0E0H
B       DATA    0F0H
DPL     DATA    82H
DPH     DATA    83H
IE      DATA    0A8H
EA      BIT     IE.7


; generate Mask and Bank Number Information
IF      ?B_NBANKS <= 2
  MASK          EQU     00000001B
ELSEIF  ?B_NBANKS <= 4
  MASK          EQU     00000011B
ELSEIF  ?B_NBANKS <= 8
  MASK          EQU     00000111B
ELSEIF  ?B_NBANKS <= 16
  MASK          EQU     00001111B
ELSEIF  ?B_NBANKS <= 32
  MASK          EQU     00011111B
ELSE
  MASK          EQU     00111111B
ENDIF     								  


IF  ?B_MODE = 4 ;**************************************************************

?B_FACTOR       EQU     0               ; Dummy Declarations
?B_FIRSTBIT     EQU     0
?B_MASK         EQU     MASK

?BANK?SELECT    SEGMENT CODE
?BANK?DATA      SEGMENT DATA
                RSEG    ?BANK?DATA
?B_CURRENTBANK: DS      1

BANK            MACRO   N
                PUBLIC  ?B_BANK&N
?B_BANK&N:
                PUSH    ?B_CURRENTBANK
                MOV     A,#HIGH ?BANK?SWITCH
                PUSH    ACC
                PUSH    DPL
                PUSH    DPH
                ENDM

SWITCH          MACRO   N
                PUBLIC  ?B_SWITCH&N
  IF (LONG_MACRO = 1)
    ?B_SWITCHJ&N:
  ELSE
    ?B_SWITCH&N:
  ENDIF
                CLR		EA				; EA disable
                MOV     ?B_CURRENTBANK,#LOW ?B_SWITCH&N
                MOV		A,0x10			; CURRENT OF BANK
                XRL		A,#&N			; if (pre==current) don't switch (do nothing)
                JZ 		?DONT_SWITCH&N
                MOV		0x10,#&N		; saving bank N		

                SWITCH&N
;         		Registers[0xF1FD]=0x81;	//Invalid iCache
                MOV		DPTR,#0xF1FD
                MOV		A,#0x81
                MOVX	@DPTR,A
				
?DONT_SWITCH&N:
                SETB	EA				; EA enable
                RET
                ENDM

  IF (LONG_MACRO = 1)
    SWITCHJ     MACRO   N
    ?B_SWITCH&N:
                JMP     ?B_SWITCHJ&N
                ENDM
  ENDIF

?BANK?SWITCH    SEGMENT CODE PAGE

                RSEG    ?BANK?SWITCH
B_SWITCH_START  EQU     $

  IF (LONG_MACRO = 1)
    ; Generate ?B_SWITCHn jmp table entries
    CNT         SET     0

                REPT    ?B_NBANKS
                SWITCHJ %CNT
    CNT         SET     CNT+1
                ENDM
  ENDIF


; Generate ?B_SWITCHn functions
CNT             SET     0

                REPT    ?B_NBANKS
                BANK    %CNT
                SWITCH  %CNT
CNT             SET     CNT+1
                ENDM

B_SWITCH_SIZE   EQU     $-B_SWITCH_START

  IF (LONG_MACRO = 0) AND (B_SWITCH_SIZE > 256)
    __ERROR__ "BANK SWITCH CODE BIGGER THAN 256 BYTES, SET LONG_MACRO TO 1"
  ENDIF

ENDIF  ; close block IF ?B_MODE = 4 *******************************************

                RSEG    ?BANK?SELECT

;************************  SWITCHBANK FUNCTION  *******************************
;                                                                             *
; SWITCHBANK allows use of bank-switching for C programs                      *
;                                                                             *
; prototype:   extern switchbank (unsigned char bank_number);                 *
;                                                                             *
;******************************************************************************
                PUBLIC  _SWITCHBANK, ?B_SWITCHBANK_A

_SWITCHBANK:    MOV     A,R7



IF  ?B_MODE = 4 ;**************************************************************

IF (?B_VAR_BANKING = 1)
                SJMP    ?B_SWITCHBANK_A
SELECT_BANK_R3:
                MOV     A,R3
                DEC     A
                ANL     A,#3FH
ENDIF

?B_SWITCHBANK_A:
                MOV	DPTR,#switch_tab
                MOVC    A,@A+DPTR
?B_RESTORE_BANK:                       ; entry for RTX-51/XBANKING bank restore
                MOV     DPTR,#?BANK?SWITCH
                JMP     @A+DPTR

S_ENTRY         MACRO   N
                DB      LOW ?B_SWITCH&N
                ENDM

switch_tab:     
CNT             SET     0

                REPT    ?B_NBANKS
                S_ENTRY %CNT
CNT             SET     CNT+1
                ENDM
ENDIF  ; close block IF ?B_MODE = 4 *******************************************



                END


