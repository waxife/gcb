#ifndef __MODEDATA_H__
#define __MODEDATA_H__
#include "VGADetect.h"

#define bDSPPLL_1		0x01
#define bDSPPLL_2		0x02
#define bDSPPLL_3		0x03	
#define bDSPPLL_4		0x04
#define bDSPPLL_5		0x05

extern MODE_DESCRIPTOR code ModeTable[];
extern ALT_HTOTAL code AltHTotals[];
extern RESOLUTION code ResolutionTable[];

extern uCHAR code SupportMode[];

extern unsigned char code TOTALMODES;
extern unsigned char code ALTHTOTAL;

extern uCHAR code SupportModeNum;

//#define TOTALMODES 149//(sizeof(ModeTable)/sizeof(MODE_DESCRIPTOR)-1)
//#define ALTHTOTAL	29//(sizeof(AltHTotals)/sizeof(ALT_HTOTAL)-1)

uCHAR code pstrSupport[];//={"Not Support !\r\n"};
uCHAR code pstrOutrange[];//={"Out Range !\r\n"};
uCHAR code pstrNotLock[];//={"Not Locked !\r\n"};
uCHAR code pstrChange[];//={"Sync Change !\r\n"};
#endif


