#ifndef __VGADETECT_H__
#define __VGADETECT_H__

#include "config.h"

typedef unsigned char  BITS;

typedef signed   char  sCHAR;

typedef unsigned short uWORD;
typedef unsigned short WORD;
typedef signed   short sWORD;

typedef unsigned long  uDWORD;
typedef signed   long  sDWORD;

#ifndef ENABLE_EX_MCU
    typedef unsigned char  uCHAR;
    typedef unsigned char  BYTE;
#endif

typedef struct tagEEPPRIVATE
{
	uCHAR 	cAdj;
	uCHAR	cHPos;
	uCHAR	cVPos;
	uCHAR	cVScaling;
	uCHAR	cHScaling;
	uCHAR   cPhase;
	uCHAR	cFreq;
	uCHAR	cPixelClock;
	uCHAR   cModeID;
	uCHAR	cAltID;
} EEPPRIVATE;

typedef struct tagEEPPUBLIC
{
	uCHAR cOSDLang;
	uCHAR cVolume;
	uCHAR cSource;
	uCHAR cOSDHPos;
	uCHAR cOSDVPos;	  

} EEPPUBLIC;

typedef struct tagMODE_DESCRIPTOR
{
	uWORD  wHTotal      ; // (HTotal = HsyncWidth + Hsst + HRes + HFrontPorch)
   	uWORD  wVTotal      ; // 
   	uWORD  wHBackPorch  ; // (Horizontal BackPorch) 
   	uCHAR  cVBackPorch  ; // (Vertical BackPorch) 
   	uCHAR  cResID       ; // (Index into Resolution table)
	uCHAR  cModeFlags0  ; // HSync(0x02) VSync(0x01)
   	uCHAR  cModeFlags1  ; // PLL     
   	uCHAR  cModeFlags2  ; // bVCORange[7,6] bChargePump[5,4,3] bAPLL_Sel_HighFreq[0]
	uWORD  wHPeriodCount; // Horizontal Period in 32 HSync lines XCLK count
	uDWORD dwVPeriodCount; // Vertical Sync count with XCLK 

} MODE_DESCRIPTOR, *PMODE_DESCRIPTOR;



//---------------------------------------------------------------------------
typedef struct tagALT_HTOTAL 
{
    uCHAR   cModeID;    // Main Mode Table Index
    uCHAR   cResID;     // Resolution Table Index
    uWORD   wHTotal;    // HTotal = HSyncWidth + Hsst + HRes + HBlk
    uWORD   wHBlk;      // BackPorch + LeftBorder
    uCHAR   cVBlk;      // BackPorch + TopBorder
    uCHAR   cModeFlags; // VCORange[2] ChargePump[3] bit[6]:bSubSample  bit[5]:bDoubSample
} ALT_HTOTAL;

//---------------------------------------------------------------------------
typedef struct tagRESOLUTION
{
    uWORD   wHRes;
    uWORD   wVRes;
} RESOLUTION;

typedef struct tagCOLORCALIBRATION
{
	uCHAR cGainR;
	uCHAR cGainG;
	uCHAR cGainB;
	uCHAR cOffsetR;
	uCHAR cOffsetG;
	uCHAR cOffsetB;
} ADC_GAIN_OFFSET;

typedef struct tagVGAState
{
	unsigned short HATV;
	unsigned short VATV;
	unsigned short HDispSize;
	unsigned short VDispSize;
	unsigned short AdjHATV;
	unsigned short AdjVATV;	
	unsigned char Phase;
	unsigned char ADCHBaPorch;
	unsigned char ADCVBaPorch;
	unsigned char cGain[3];
	unsigned char cOffset[3];
}VGAPOSTATE;


#define XCLK				27000
#define XCLK2				27000000
#define HSYNC_PERIOD(x)		(XCLK/x*32)
#define VSYNC_PERIOD(x)		(XCLK2/(x))	


#define DPLL_DIV_I				2//[C9]
#define DPLL_DIV_R				2//[CA]
#define DRDIV			(1<<(DPLL_DIV_R+1))

#define DFDIV_1		0x2A// 
#define DFDIV_2	    0x2C//640X480@75;1280X1024@60;
#define DFDIV_3		0x25//640X480@60;640X480@70;640X480@72;1024X768@60;1024X768@75
#define DFDIV_4		0x2C//800X600@60;800X600@72;800X600@75(NG);1024X768@70;
#define DFDIV_5		0x3E//

#define VLINE_TOLERANCE						4
#define HSYNC_TOLERANCE						0.2
#define NUMINTERLACED						14

#define ADCHIGHRANGE						0x02
#define ADCLOWRANGE							0x04
#define VSYNCOVERRANGE						0x08
#define OVERRANGE       					0x10
#define SYNCERROR							0x20	//unreasonable result, sync rate too high or too low
#define MODEGUEST	      					0x40
#define MODEFOUND							0x80

#define FIXEDVSST 							0x08
#define VSGEPW  								3
#define HOFFSET  								0x38
#define HOFFSET_MIN							0x08
#define FIXEDHSST							0x6//0x62//	0xa2
#define HOFFSETPW  							0x05/0x05 //0x0a

#define CHANGMODE							0x10
#define FORCEDTUNE							0x20
#define FINETUNE								0x40
#define POS1									0x01
#define POS2									0x02
#define PIXEL									0x04
#define PHASE									0x08

#define IN_ACTIVE							0x01
#define IN_BLANK								0x02



typedef enum{
Sync_CS=1,
Sync_SS,
Sync_SOG
}Sync_Type;

extern uCHAR	xdata   m_cBuff[4];
extern uWORD	xdata   m_wBuff[6];
extern uDWORD	xdata  m_dwBuff[2];

extern uWORD	xdata m_wHRes;
extern uWORD	xdata m_wVRes;
extern uCHAR	xdata  m_bAlt;
extern uCHAR	xdata  m_cModeStatus;
extern uCHAR	xdata  m_bInterlaced;
extern uCHAR	xdata 	m_cDspPllSwitch;
extern uWORD	xdata  m_wVTotal;				// Vertical lines total. This will be change with TV system.
extern uCHAR	xdata  m_Synctype;
extern uCHAR	xdata  m_bDosMode;
extern VGAPOSTATE	xdata VGAState;					// For User Fine-Tune VGA State.

extern MODE_DESCRIPTOR code *m_pModeDescriptorPtr;
extern RESOLUTION	code *m_pResolutionPtr;
extern ALT_HTOTAL 	code *m_pAltHTotalPtr;

extern unsigned int HSYNC_PW;
extern unsigned int VSYNC_PW;

#define AUTO_CAL_OUT_TIMING

extern uDWORD GetInputVSyncXclkCnt(void);
extern void GetModeInfo(uWORD *wVTotal, uWORD *wHPeriodCount, uDWORD dwFrameCnt);
extern void SetVGAoutputPll(void);
extern void HPosAutoAdj(void);
extern bit SrcVGAProcess(uCHAR *std, uCHAR autoadj);
extern void VPosAutoAdj(void);
extern bit ModeDetect(uCHAR *vga_std);
extern unsigned char VGA_res_change_check(void);
extern void ACal_output_timing(void);
extern void Sync_Mode_Detect(void);
#endif
