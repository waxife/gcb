#include <reg51.h>
#include <math.h>
#include "Panel.h"
#include "reg_tw.h"
#include "sys.h"
#include "cq.h"
#include "tw_widget_sys.h"
#include "tw_timer.h"
#include "Modedata.h"
#include "VGADetect.h"

uCHAR	xdata   m_cBuff[4];
uWORD	xdata   m_wBuff[6];
uDWORD	xdata  m_dwBuff[2];

uWORD	xdata m_wHRes;
uWORD	xdata m_wVRes;
uCHAR	xdata  m_bAlt;
uCHAR	xdata  m_cModeStatus;
uCHAR	xdata  m_bInterlaced;
uCHAR	xdata 	m_cDspPllSwitch;
uWORD	xdata  m_wVTotal;				// Vertical lines total. This will be change with TV system.
uCHAR	xdata  m_Synctype;
uCHAR	xdata  m_bDosMode;

EEPPRIVATE	xdata   EepPrivate;
EEPPUBLIC xdata  tagEEPPUBLIC;

VGAPOSTATE	xdata VGAState;					// For User Fine-Tune VGA State.

MODE_DESCRIPTOR code *m_pModeDescriptorPtr;
RESOLUTION	code *m_pResolutionPtr;
ALT_HTOTAL 	code *m_pAltHTotalPtr;

uCHAR xdata Boundary_det_count=0;
uWORD xdata VGAPosData[4];
unsigned int HSYNC_PW;
unsigned int VSYNC_PW;

//#define DEBUGMSG
//#define DEBUGMSG1


// ================================================================
//	ModeDetect			Kevin Hsu.	Mar. 2th, 2006
// ================================================================
//	Description: Detect what kind of Mode in VGA signal. 
//	Output:		 m_Synctype : VGA Mode.
//	---------------------------------------------------------------
bit ModeDetect(uCHAR *vga_std)
{
	int i;
	int mode_count=0;
	int v_total_div=0;	 
	uWORD	wHPeriodCount;

	m_cModeStatus = 0;
 	m_wVTotal = 0;
	m_dwBuff[0]=GetInputVSyncXclkCnt();	
	
	if(m_dwBuff[0]<(27000000/84))   // Not support the frame rate bigger than 84
	{
		DBG_PRINT("NOT SUPPORT ,high than 84\r\n");
		m_cModeStatus = OVERRANGE;
	    goto EXIT;
	}
	if(m_dwBuff[0]>(27000000/55))   // Not support the frame rate slow than 55
	{
		DBG_PRINT("NOT SUPPORT ,less than 55\r\n");
		m_cModeStatus = OVERRANGE;
	    goto EXIT;
	}
	GetModeInfo(&m_wVTotal, &wHPeriodCount, m_dwBuff[0]);
	
	if(m_wVTotal < ModeTable[0].wVTotal)
	{
		//putString(pstrOutrange);
		m_cModeStatus = SYNCERROR;  //unreasonable result, sync error
		goto EXIT;
	}
//#ifdef DEBUGMSG
	DBG_PRINT("====================================\r\n",0);
	DBG_PRINT("dwVPeriodCount=%d\r\n",(unsigned int)(27000000/m_dwBuff[0]));
	DBG_PRINT("wVTotal=%d\r\n",(unsigned int)m_wVTotal);
	DBG_PRINT("wHPeriodCount=%d\r\n",(unsigned int)(27000000/(wHPeriodCount/32)));// 843750 = 27000000/32
//#endif
 
	m_cBuff[0] = 0;
	m_cBuff[1] = 0;
	EepPrivate.cModeID = 0;
	if(m_wVTotal<(ModeTable[0].wVTotal-10))
	{
		m_cModeStatus = SYNCERROR;  //unreasonable result, sync error
		return 0;
	}

	//Find Bottom Boundary
	while ((m_wVTotal > ModeTable[m_cBuff[0]].wVTotal) && (m_cBuff[0] <= TOTALMODES))
	{
		m_cBuff[0]++;
	}
	if(m_cBuff[0]>0)
		m_cBuff[0]--;
	//Find Up Boundary
	m_cBuff[1] = m_cBuff[0];
	while ((m_wVTotal >= ModeTable[m_cBuff[1]].wVTotal) && (m_cBuff[1] <= TOTALMODES))
	{
		m_cBuff[1]++;
	}
#ifdef DEBUGMSG1
	DBG_PRINT("B=%d\r\n",(unsigned int)m_cBuff[0]);
	DBG_PRINT("U=%d\r\n",(unsigned int)m_cBuff[1]);
#endif
	EepPrivate.cModeID = m_cBuff[0];
	m_cModeStatus = MODEGUEST;
	m_pModeDescriptorPtr = &ModeTable[m_cBuff[0]];
	m_pResolutionPtr = &ResolutionTable[m_pModeDescriptorPtr->cResID];
	/* table match */
	if((m_cBuff[1] - m_cBuff[0])==1)
	{
		if(m_wVTotal == ModeTable[m_cBuff[0]].wVTotal)
		{
#ifdef DEBUGMSG1		
				DBG_PRINT("B match: d2=%d\r\n",(unsigned int)(abs(m_dwBuff[0]-ModeTable[m_cBuff[0]].dwVPeriodCount)));
				DBG_PRINT("B match: d3=%d\r\n",(unsigned int)(ModeTable[i].dwVPeriodCount/20));
#endif
			if(abs(m_dwBuff[0]-ModeTable[m_cBuff[0]].dwVPeriodCount)<(ModeTable[m_cBuff[0]].dwVPeriodCount/20))
			{
			 	EepPrivate.cModeID = m_cBuff[0];
				m_cModeStatus = MODEFOUND;
				m_pModeDescriptorPtr = &ModeTable[m_cBuff[0]];
				m_pResolutionPtr = &ResolutionTable[m_pModeDescriptorPtr->cResID];
				goto EXIT;
			}
		}
	}
  	/* set the same Vtotal bottom boundary from first to the last */
	for(i=m_cBuff[0]; i<m_cBuff[1]; i++)
	{
		if(m_wVTotal == ModeTable[i].wVTotal)
		{
			EepPrivate.cModeID = m_cBuff[i];
			m_cModeStatus = MODEGUEST;
			m_pModeDescriptorPtr = &ModeTable[m_cBuff[i]];
			m_pResolutionPtr = &ResolutionTable[m_pModeDescriptorPtr->cResID];
			break;
		}
	}

	v_total_div = 999;
	if(i==m_cBuff[1])
	{
		for(i=m_cBuff[0]; i<=m_cBuff[1]; i++)
		{
			if(v_total_div>= abs(m_wVTotal - ModeTable[i].wVTotal))
			{
#ifdef DEBUGMSG1		
				DBG_PRINT("d2=%d\r\n",(unsigned int)(abs(wHPeriodCount-ModeTable[i].wHPeriodCount)));
				DBG_PRINT("d3=%d\r\n",(unsigned int)(ModeTable[i].wHPeriodCount/100));
#endif
				if(abs(wHPeriodCount-ModeTable[i].wHPeriodCount)<(ModeTable[i].wHPeriodCount/20))
				{
					v_total_div = abs(m_wVTotal - ModeTable[i].wVTotal);
					EepPrivate.cModeID = i;
					m_cModeStatus = MODEFOUND;
					m_pModeDescriptorPtr = &ModeTable[i];
					m_pResolutionPtr = &ResolutionTable[m_pModeDescriptorPtr->cResID];
				}
				
			}
		}
		goto EXIT;
	}  
	else if(i<m_cBuff[1])
	{
		m_cBuff[0] = i;
	}
    for(i=m_cBuff[0]; i<m_cBuff[1]; i++)
	{
		if(m_wVTotal != ModeTable[m_cBuff[0]].wVTotal)
		{
			break;
		}
	}
    m_cBuff[1] = i;
#ifdef DEBUGMSG1	
	DBG_PRINT("B=%d\r\n",(unsigned int)m_cBuff[0]);
	DBG_PRINT("U=%d\r\n",(unsigned int)m_cBuff[1]);
#endif	
	if((m_cBuff[1]-m_cBuff[0])>=1)
	{
		for(i=m_cBuff[0]; i<m_cBuff[1]; i++)
		{
			if(m_wVTotal == ModeTable[i].wVTotal)
			{
#ifdef DEBUGMSG1
				DBG_PRINT("i=%d\r\n",i);
				DBG_PRINT("d2=%d\r\n",(unsigned int)abs(m_dwBuff[0]-ModeTable[i].dwVPeriodCount));
				DBG_PRINT("d3=%d\r\n",(unsigned int)(ModeTable[i].dwVPeriodCount/100));
#endif
				if(abs(m_dwBuff[0]-ModeTable[i].dwVPeriodCount)<(ModeTable[i].dwVPeriodCount/100))
				{
				 	EepPrivate.cModeID = i;
					m_cModeStatus = MODEFOUND;
					m_pModeDescriptorPtr = &ModeTable[i];
					m_pResolutionPtr = &ResolutionTable[m_pModeDescriptorPtr->cResID];
					mode_count++;
					//goto EXIT;
				}
			}
		}
	}
//	DBG_PRINT("MC=%d\r\n",mode_count);
	if(mode_count==0)
		m_cModeStatus = OVERRANGE;
	if(mode_count>1)
	{
		for(i=m_cBuff[0]; i<m_cBuff[1]; i++)
		{
			if(m_wVTotal == ModeTable[i].wVTotal)
			{
				if(abs(m_dwBuff[0]-ModeTable[i].dwVPeriodCount)<(ModeTable[i].dwVPeriodCount/10))
				{
					if(abs(wHPeriodCount-ModeTable[i].wHPeriodCount)<(ModeTable[i].wHPeriodCount/100))
					{
					 	EepPrivate.cModeID = i;
						*vga_std = EepPrivate.cModeID;
						m_cModeStatus = MODEFOUND;
						m_pModeDescriptorPtr = &ModeTable[i];
						m_pResolutionPtr = &ResolutionTable[m_pModeDescriptorPtr->cResID];
						mode_count++;
					}
					//goto EXIT;
				}
			}
		}
	}				


EXIT:
//#ifdef DEBUGMSG
	DBG_PRINT("Mode ID=%d\r\n",(unsigned int)EepPrivate.cModeID);
	if(m_cModeStatus==MODEFOUND)
		DBG_PRINT("Mode Match.\r\n",0);
	else if(m_cModeStatus==MODEGUEST)
		DBG_PRINT("Mode Guest!\r\n",0);
	else
	    DBG_PRINT("Mode Not Support!\r\n",0);
	DBG_PRINT("====================================\r\n",0);
//#endif
	if((m_cModeStatus==MODEGUEST)||(m_cModeStatus==MODEFOUND))
		return m_cModeStatus;
	else 
		return 0;
}

void SetModeCaptureData(uWORD wHTotal, uWORD wHsst, uCHAR cVsst, uCHAR cVCO, uCHAR HVSyncPol)
{
		
	//~Get Sync Polarity. ~//
	IC_WritByte(TWIC_P0, 0x10, IC_ReadByte(TWIC_P0, 0x10)&0x3F); 
	twdDelay(30);
	HVSyncPol =  ((~IC_ReadByte(TWIC_P0, 0x3F))&0x0C)<<4;  
	//~Set Sync Polarity. Auto polarity have some thing wrong after T108.~//
	IC_WritByte(TWIC_P0, 0x10, IC_ReadByte(TWIC_P0, 0x10)|HVSyncPol);

	//~ Set PLL Freq. ~// 	
	//~ Set PLL Divider 0x16[3~0],0x15[7~0] ~//
  	IC_WritByte(TWIC_P0, 0x15, (uCHAR)(wHTotal)); 
	IC_WritByte(TWIC_P0, 0x16, (IC_ReadByte(TWIC_P0, 0x16)&0xf0)|(((uCHAR)(wHTotal>> 8))|((cVCO&0x01)<<6))); 
	//~ Set VCO and Charge Pump ~//      
	IC_WritByte(TWIC_P0, 0x17, cVCO&0xFE);	
	//~ Set ADC Capture HSync Back Proch 0x4C, 0x4D ~//
  	IC_WritByte(TWIC_P0, 0x4C, (uCHAR)wHsst);
  	IC_WritByte(TWIC_P0, 0x4D, (uCHAR)(wHsst>>8));

  	//~ Set ADC Capture VSync Back Proch 0x4E, 0x4F ~//
	IC_WritByte(TWIC_P0, 0x4E, (uCHAR)cVsst);
	IC_WritByte(TWIC_P0, 0x4F, (uCHAR)(cVsst >> 8));

	//~ Set ADC Capture HSize 0x48, 0x49 ~//
  	IC_WritByte(TWIC_P0, 0x48, (uCHAR)m_wHRes);
  	IC_WritByte(TWIC_P0, 0x49, (uCHAR)(m_wHRes>>8));
	//~ Set ADC Capture VSize 0x4A, 0x4B ~//
  	IC_WritByte(TWIC_P0, 0x4A, (uCHAR)m_wVRes);
  	IC_WritByte(TWIC_P0, 0x4B, (uCHAR)(m_wVRes>>8));
} 



void SetupMode(void)
{  	
	m_wHRes = ResolutionTable[m_pModeDescriptorPtr->cResID].wHRes;
	m_wVRes = ResolutionTable[m_pModeDescriptorPtr->cResID].wVRes;	
	DBG_PRINT("m_wHRes=%d\r\n",(unsigned int)m_wHRes);
	DBG_PRINT("m_wVRes=%d\r\n",(unsigned int)m_wVRes);
	DBG_PRINT("m_pModeDescriptorPtr->wHTotal=%d\r\n",(unsigned int)m_pModeDescriptorPtr->wHTotal);
	DBG_PRINT("m_pModeDescriptorPtr->wHBackPorch=%d\r\n",(unsigned int)m_pModeDescriptorPtr->wHBackPorch);
	DBG_PRINT("m_pModeDescriptorPtr->cVBackPorch=%d\r\n",(unsigned int)m_pModeDescriptorPtr->cVBackPorch);
	DBG_PRINT("m_pModeDescriptorPtr->cModeFlags2=%x\r\n",(unsigned int)m_pModeDescriptorPtr->cModeFlags2);
	DBG_PRINT("m_pModeDescriptorPtr->cModeFlags1=%x\r\n",(unsigned int)m_pModeDescriptorPtr->cModeFlags1);
	DBG_PRINT("m_pModeDescriptorPtr->cModeFlags0=%x\r\n",(unsigned int)m_pModeDescriptorPtr->cModeFlags0);
	SetModeCaptureData(m_pModeDescriptorPtr->wHTotal, m_pModeDescriptorPtr->wHBackPorch, 
						m_pModeDescriptorPtr->cVBackPorch, m_pModeDescriptorPtr->cModeFlags2,
						m_pModeDescriptorPtr->cModeFlags0);
	
	SetVGAoutputPll();
	VGAState.HDispSize = IC_ReadByte(TWIC_P0, 0xDC)|(IC_ReadByte(TWIC_P0, 0xDD)<<8);
	VGAState.VDispSize = IC_ReadByte(TWIC_P0, 0xDE)|(IC_ReadByte(TWIC_P0, 0xDF)<<8);
	VGAState.AdjHATV = VGAState.HATV = m_pModeDescriptorPtr->wHBackPorch;
	VGAState.AdjVATV = VGAState.VATV= m_pModeDescriptorPtr->cVBackPorch;
	twdDelay(1000);
}

void VGAScaling(void)
{

	//Horizontal	
	m_wBuff[0] = (float)(m_wHRes)/VGAState.HDispSize*0x2000; 
	IC_WritByte(TWIC_P0, 0x72, (unsigned char)m_wBuff[0]);
    IC_WritByte(TWIC_P0, 0x73, (unsigned char)(m_wBuff[0]>>8));

	//Vertical 
	if(m_wVRes>=(VGAState.VDispSize*2))
	{
		 m_wVRes = m_wVRes>>1;
		 IC_WritByte(TWIC_P0, 0x70, IC_ReadByte(TWIC_P0, 0x70)|0x08);
	}
	else
		 IC_WritByte(TWIC_P0, 0x70, IC_ReadByte(TWIC_P0, 0x70)&0xF7);
	m_wBuff[0] = (float)(m_wVRes)/(VGAState.VDispSize)*0x2000;   
	IC_WritByte(TWIC_P0, 0x74, (unsigned char)m_wBuff[0]);
	IC_WritByte(TWIC_P0, 0x75, (unsigned char)(m_wBuff[0]>>8)); 
}

void SetVGAoutputPll(void)
{								
	switch((m_pModeDescriptorPtr->cModeFlags1))
	{
		case bDSPPLL_1:
			m_cDspPllSwitch=DFDIV_1;
			break;
		case bDSPPLL_2:
			m_cDspPllSwitch=DFDIV_2;
			break;
		case bDSPPLL_3:
			m_cDspPllSwitch=DFDIV_3;
			break;
		case bDSPPLL_4:
			m_cDspPllSwitch=DFDIV_4;
			break;
		case bDSPPLL_5:
			m_cDspPllSwitch=DFDIV_5;
			break;
		default:
			m_cDspPllSwitch=DFDIV_3;
			break;
	}
    IC_WritByte(TWIC_P0, 0xC8,m_cDspPllSwitch);	      
	IC_WritByte(TWIC_P0, 0xC9,DPLL_DIV_I);
	IC_WritByte(TWIC_P0, 0xCA,(IC_ReadByte(TWIC_P0, 0xCA)&0xFC)|DPLL_DIV_R); 
}												  
	
void Sync_Mode_Detect(void)
{
	int i;

	//~ Enable Sync exist detect ~//
	IC_WritByte(TWIC_P0, 0x21, 0x04);
	
	//~ Wait for Checking HS/VS finish. ~//
	i=300;
	while(i)
	{
		twdDelay(20);
		i--;					  
		if(IC_ReadByte(TWIC_P0, 0x21)&0x04)	
			break;
	}
	//~ Check if HS and VS is exist. ~//
	if((IC_ReadByte(TWIC_P0, 0x21)&0x03)==3)	//~ Synchronize is possible the Composite type or Separate type. ~//
	{	
		//~ Set up HS/VS polarity. We set it as auto-adjusting. ~//	
		IC_WritByte(TWIC_P0,0x10,0x09);     
		//~ Suppose	that sync. is Composite type and setup its route. ~//
		m_Synctype=Sync_CS;
        IC_WritByte(TWIC_P0, 0x14, 0x61);
		twdDelay(500);
		//~ Enable CSync detect and XCLK ~//
		IC_WritByte(TWIC_P0,0x13,0x02);
		IC_WritByte(TWIC_P0,0x13,0x22);
		twdDelay(500);		
		if(IC_ReadByte(TWIC_P0, 0x13)&0x80) //~ Check CSync detection is done or not. ~//
		{
			//~ If true, sync is Composite type. ~//
			m_Synctype=Sync_CS;
		}
		else							     //~ Sync. is Separate type. ~//
		{
			//~ Setup Separate type. ~// 	  			
			m_Synctype=Sync_SS;
			//~ Setup Separate type route. ~// 
			IC_WritByte(TWIC_P0,0x14,IC_ReadByte(TWIC_P0, 0x14)&0xF0);		
		}  
	}
	else  									 //~ No HS and VS, we suppose that synchronize is SOG type. ~//
	{	
		//~ Setup SOG/SOY type. ~//
		m_Synctype=Sync_SOG;
		//~ Set up SOG/SOY  polarity. We set it as non-invert. ~//
		IC_WritByte(TWIC_P0,0x10,0x39);
		//~ Setup SOG/SOY  type route. ~//
		IC_WritByte(TWIC_P0,0x11,0x00);//20
		IC_WritByte(TWIC_P0,0x12,0xef);
		IC_WritByte(TWIC_P0,0x14,0x68);	
	}
	
}

void GetModeInfo(uWORD *wVTotal, uWORD *wHPeriodCount, uDWORD dwFrameCnt)
{	
	unsigned short	wTemp;
	int i;
	
	//~ Get HSync period. ~//
	IC_WritByte(TWIC_P0, 0x14, IC_ReadByte(TWIC_P0, 0x14)&0xEF);	// Disable HSo source from PLL.
	IC_WritByte(TWIC_P0, 0x16, IC_ReadByte(TWIC_P0, 0x16)|0x04);	// Set PLL divider default value.
	wTemp=0;
	for(i=0; i<30; i++)
	{
		twdDelay(50);	
		*wHPeriodCount = IC_ReadByte(TWIC_P0, 0x59);
		*wHPeriodCount <<= 8;
		*wHPeriodCount += IC_ReadByte(TWIC_P0, 0x58);
		if((wTemp&0xFF00)==((*wHPeriodCount)&0xFF00))
			break;
		else
			wTemp = *wHPeriodCount;
	}
	wTemp = dwFrameCnt * 32 * 10 / (*wHPeriodCount); 
	if(wTemp % 10 >= 9)
		m_wBuff[0] = wTemp/10 + 1;
	else	
		m_wBuff[0] = wTemp/10;
	
	*wVTotal = m_wBuff[0]; 
	IC_WritByte(TWIC_P0, 0x14, IC_ReadByte(TWIC_P0, 0x14)|(~0xEF)); 	// Enable HSo source from PLL.
}			   


// ================================================================
//	GetInputVSyncXclkCnt			Kevin Hsu.	Mar. 7th, 2006
// ================================================================
//	Description: Get VSync. period  
//	Output:		 dwVSyncCount : VSync period.
//	---------------------------------------------------------------
uDWORD GetInputVSyncXclkCnt(void)			 
{
	uDWORD dwVSyncCount;
   IC_WritByte(TWIC_P0, 0x14, IC_ReadByte(TWIC_P0, 0x14)&0xEF);	// Disable HSo source from PLL.
	
	m_dwBuff[0] = 0;
	m_cBuff[1]=20;
	m_cBuff[2]=7;
	IC_WritByte(TWIC_P0, 0x50, (IC_ReadByte(TWIC_P0, 0x50) & 0xEF));
RECHECK:		
	twdDelay(1000);		
	m_cBuff[1]=m_cBuff[1]-1;
	//twdDelay(30);	
	//m_cBuff[0] = IC_ReadByte(TWIC_P0, 0x50);
	IC_WritByte(TWIC_P0, 0x50, (IC_ReadByte(TWIC_P0, 0x50) | 0x10));    	//Enable VSYNC counter
	m_cBuff[0] = 100;
	while(m_cBuff[0]--)
	{
		twdDelay(30);
		if ((IC_ReadByte(TWIC_P0, 0x50)&0x20)) //V sync counter is done exit
			break;
	}
	m_cBuff[0] = IC_ReadByte(TWIC_P0, 0x50);
	m_cBuff[0] &= 0xCF;			//Disable Auto Position
	IC_WritByte(TWIC_P0, 0x50, m_cBuff[0]);

	dwVSyncCount =  IC_ReadByte(TWIC_P0, 0x53);
	dwVSyncCount <<= 8;
	dwVSyncCount |= IC_ReadByte(TWIC_P0, 0x52);
	dwVSyncCount <<= 8;
	dwVSyncCount |= IC_ReadByte(TWIC_P0, 0x51);
	if((m_dwBuff[0]&0xFFFF00)!=(dwVSyncCount&0xFFFF00))
	{
		m_cBuff[2]=7;
		m_dwBuff[0] = dwVSyncCount;
		if(m_cBuff[1])
			goto RECHECK;
	}
	else 
	{
		m_cBuff[2] = m_cBuff[2] - 1;
		if(m_cBuff[2] != 0)
			goto RECHECK;
	}
	IC_WritByte(TWIC_P0, 0x14, IC_ReadByte(TWIC_P0, 0x14)|(~0xEF)); 	// Enable HSo source from PLL.
	return dwVSyncCount;		
}
#if 0
bit AutoPositionAdj(uCHAR cTestMode)
{									
	int RestartCount = 0;
	
	if(cTestMode) 
	{
		
		if(Boundary_det_count>=50)	 
		{	 
			return 1; 		
		}
		else
			Boundary_det_count++;
	}
//	char strTemp[20];
	RestartCount = 0;
RESTART:     
 
	//Reset Hsync and Vsync offset.
	IC_WritByte(TWIC_P0, 0x41, 0);
	IC_WritByte(TWIC_P0, 0x43, 0);				   
	//~ Set threshold. ~//
	IC_WritByte(TWIC_P0, 0x27, 0x40);

	//~ Enable boundary detection ~//		  	
	IC_WritByte(TWIC_P0, 0x26, (IC_ReadByte(TWIC_P0, 0x26)&0x7F));
	twdDelay(1000);
	IC_WritByte(TWIC_P0, 0x26, (IC_ReadByte(TWIC_P0, 0x26)|0x80));
	m_wBuff[0]=20;
	while(m_wBuff[0])
	{
		twdDelay(500);
		if(IC_ReadByte(TWIC_P0, 0x26)&0x80)	break;
		m_wBuff[0]--;
	}
	//~ Disable boundary detection ~//
	IC_WritByte(TWIC_P0, 0x26, (IC_ReadByte(TWIC_P0, 0x26)&0x7F));	
	//data start

	m_wBuff[0] = IC_ReadByte(TWIC_P0, 0x29);
	m_wBuff[0] <<= 8;
	m_wBuff[0] |= IC_ReadByte(TWIC_P0, 0x28); 
	m_wBuff[0] -= 2;//For more good edge.					
	//data end
	m_wBuff[1] = IC_ReadByte(TWIC_P0, 0x2B);       				
	m_wBuff[1] <<= 8;
	m_wBuff[1] |= IC_ReadByte(TWIC_P0, 0x2A);

	if(m_wBuff[1]<=m_wBuff[0])
	{

		RestartCount++;
		if(RestartCount<5)
	    	goto RESTART;
		else
			return 0;
			//return 0;
	}

	if(cTestMode)
	{			   
   		if(abs(VGAState.HATV-m_wBuff[0])>(VGAState.HATV))
		{
			m_wBuff[0] = VGAState.HATV;
		}
	
   		  if((VGAPosData[0] != m_wBuff[0])||(VGAPosData[1] != m_wBuff[1]))
		  {
//		  		DBG_PRINT("L1=  %d\r\n",VGAPosData[0]);
//				DBG_PRINT("L2=  %d\r\n",m_wBuff[0]);
				//DBG_PRINT("R1=  %d\r\n",VGAPosData[1]);
				//DBG_PRINT("R2=  %d\r\n",m_wBuff[1]);
			  	RestartCount++;
				if(RestartCount<10)
					goto RESTART;
				else
					return 0;
		  }
		  else
		  	return 1;
	}
	else{
		m_wHRes = m_wBuff[1] - m_wBuff[0] + 1;	 
		HPosAutoAdj();	 
		VGAPosData[0] = m_wBuff[0];
		VGAPosData[1] = m_wBuff[1];	
	} 	  
	DBG_PRINT("m_wHRes =  %d\r\n",m_wHRes);
	//data start
	m_wBuff[0] = IC_ReadByte(TWIC_P0, 0x2D);
	m_wBuff[0] <<= 8;
	m_wBuff[0] |= IC_ReadByte(TWIC_P0, 0x2C); 

	//data end
	m_wBuff[1] = IC_ReadByte(TWIC_P0, 0x2F);       
	m_wBuff[1] <<= 8;
	m_wBuff[1] |= IC_ReadByte(TWIC_P0, 0x2E);


	if(m_wBuff[1]<=m_wBuff[0])
	{
		RestartCount++;
		if(RestartCount<5)
	    	goto RESTART;
		else
			return 0;
	}
	if(cTestMode)
	{		
		if(abs(VGAState.VATV-m_wBuff[0])>(VGAState.VATV))
		{
		
			m_wBuff[0] = m_pModeDescriptorPtr->cVBackPorch;
		}	   
		  if((VGAPosData[2] != m_wBuff[0])||(VGAPosData[3] != m_wBuff[1]))
		  {
			  	RestartCount++;
				if(RestartCount<10)
				goto RESTART;
				else
				return 0;
		  }
		  else
		  	return 1;
	}
	else
	{  
		m_wVRes = m_wBuff[1] - m_wBuff[0] + 1;
		VPosAutoAdj();	
		VGAPosData[2] = m_wBuff[0];
		VGAPosData[3] = m_wBuff[1];  		
	}
	DBG_PRINT("m_wVRes =  %d\r\n",m_wVRes);
	VGAState.AdjHATV = m_wHRes; 
	VGAState.AdjVATV = m_wVRes; 
	return 1;	  
}
#endif
bit AutoPhaseAdj(void)
{	
	uWORD MaxTrackValue, MinTarckValue, TrackValue;
	uCHAR MaxTrackAddre, MinTrackAddre;
	uCHAR i, PhSetTemp, ADCDetTemp, CountTemp;
	
	//~ Find Max and Min phase ~//
	MaxTrackValue=0x0000;
	MinTarckValue=0xFFFF;
	PhSetTemp = IC_ReadByte(TWIC_P0, 0x20)&0x07;
	ADCDetTemp = IC_ReadByte(TWIC_P0, 0x21)&0x7F;

	//~ Find Max. and Min. phase tracking. ~//
	for(i=0; i<32; i+=2)
	{
		//~ Set pahse ~//
		IC_WritByte(TWIC_P0, 0x20,(PhSetTemp|(i<<3)));
		CountTemp = 255;
		//~ Enable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp|0x80);
		/*if((I2CReadByte(TW803_P0, 0x21)&0x80)==0)
		{
			  CountTemp =20;
		} */
		while(CountTemp)
		{
			if(IC_ReadByte(TWIC_P0, 0x21)&0x80)	
				break;
			else{ 
		/*		TrackValue = TrackValue*1000;
				TrackValue = TrackValue/1000;  */
				CountTemp--;
				twdDelay(50);
			}
		}
		TrackValue = IC_ReadByte(TWIC_P0, 0x23) | (IC_ReadByte(TWIC_P0, 0x24)<<8);
		//~ Disable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp&0x7F);

		if(TrackValue>MaxTrackValue)
		{
			MaxTrackValue = TrackValue;
			MaxTrackAddre = i;
		}
		if(TrackValue<MinTarckValue)
		{
			MinTarckValue = TrackValue;
			MinTrackAddre = i;
		}  
	}

	//~ Define evaluation range. ~//
	if(MaxTrackAddre>MinTrackAddre)
	{
		if((MaxTrackAddre + 6) > 31 )
		{
			m_cBuff[1] = 31;
		}
		else
		{
			m_cBuff[1] = MaxTrackAddre + 6;
		}
		if(MaxTrackAddre<6)
		{
			m_cBuff[0] = 0;
		}
		else
		{
			m_cBuff[0] = MaxTrackAddre - 6;
		}
  	}
	else
	{
		if((MaxTrackAddre + 6) > 31 )
		{
			m_cBuff[1] = 31;
		}
		else
		{
			m_cBuff[1] = MaxTrackAddre + 6;
		}
		if(MaxTrackAddre<6)
		{
			m_cBuff[0] = 0;
		}
		else
		{
			m_cBuff[0] = MaxTrackAddre - 6;
		}
 	}

	//~ Find Min. Delta ~//
	MaxTrackValue = 0xFFFF;
	MaxTrackAddre = 0;
	if(m_cBuff[0]==0) m_cBuff[0] += 1;
	if(m_cBuff[1]==31) m_cBuff[1] -= 1;
	for(i=m_cBuff[0]; i<m_cBuff[1]; i++)
	{
		//~ Set pahse 1 ~//
		
		IC_WritByte(TWIC_P0, 0x20,(PhSetTemp|((i-1)<<3)));
		CountTemp = 255;
		//~ Enable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp|0x80);
		while(CountTemp)
		{
			if(IC_ReadByte(TWIC_P0, 0x21)&0x80)	
				break;
			else
			{
				twdDelay(50);
				CountTemp--;
			}
		}
		m_wBuff[0] = IC_ReadByte(TWIC_P0, 0x23) | (IC_ReadByte(TWIC_P0, 0x24)<<8);
		//~ Disable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp&0x7F);		
		//~ Set pahse 2 ~//
		IC_WritByte(TWIC_P0, 0x20, (PhSetTemp|((i)<<3)));
		CountTemp = 255;
		//~ Enable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp|0x80);
		while(CountTemp)
		{
			if(IC_ReadByte(TWIC_P0, 0x21)&0x80)	
				break;
			else 
			{
				twdDelay(50);
				CountTemp--;
			}
		}
		m_wBuff[1] = IC_ReadByte(TWIC_P0, 0x23) | (IC_ReadByte(TWIC_P0, 0x24)<<8);	
		//~ Disable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp&0x7F);
		//~ Set pahse 3 ~//
		IC_WritByte(TWIC_P0, 0x20,(PhSetTemp|((i+1)<<3)));
		CountTemp = 255;
		//~ Enable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp|0x80);
		while(CountTemp)
		{
			if(IC_ReadByte(TWIC_P0, 0x21)&0x80)		//
				break;
			else {
				twdDelay(50);
				CountTemp--;
			}
		}
		m_wBuff[2] = IC_ReadByte(TWIC_P0, 0x23) | (IC_ReadByte(TWIC_P0, 0x24)<<8);
		//~ Disable phase tracking. ~//
		IC_WritByte(TWIC_P0, 0x21, ADCDetTemp&0x7F);
		MinTarckValue = (abs(m_wBuff[1]-m_wBuff[0]) + abs(m_wBuff[2]-m_wBuff[1]))>>1;
		if(MinTarckValue<MaxTrackValue)
		{
			MaxTrackValue = MinTarckValue;
			MaxTrackAddre = i;
		}
	}

	//~ Set phase ~//
	IC_WritByte(TWIC_P0, 0x20,(PhSetTemp|((MaxTrackAddre)<<3)));
	
	return 1;		
}


bit SrcVGAProcess(uCHAR *std,uCHAR autoadj)
{	
	if(!ModeDetect(std))
		return 0;	
	SetupMode();	 
	
#if 0
	if(AutoPositionAdj(0)==0) 
	{
		DBG_PRINT("AutoPositionAdj failed \r\n");
		return 0;		
	}
#endif	
	ACal_output_timing();	
	Boundary_det_count = 0;
	DBG_PRINT("Open panel\r\n");
	if(autoadj)
		AutoPhaseAdj();	
	IC_WritByte(TWIC_P6, 0x12, 0x3f);
	return 1;
}

void HPosAutoAdj(void)
{
	if(abs(m_pResolutionPtr->wHRes-m_wHRes)>(m_pResolutionPtr->wHRes*0.2))
	{
		//~ Difference is too large, so following the mode table. ~//
		m_wHRes=m_pResolutionPtr->wHRes;
		IC_WritByte(TWIC_P0, 0x48, (uCHAR)m_wHRes);
		IC_WritByte(TWIC_P0, 0x49, (uCHAR)(m_wHRes>>8));
	}
	else
	{
		//~ Reset ADC Capture HSize 0x48, 0x49 ~//
		IC_WritByte(TWIC_P0, 0x48, (uCHAR)m_wHRes);
		IC_WritByte(TWIC_P0, 0x49, (uCHAR)(m_wHRes>>8));
	}
	//~ Fill in the HSync Start Point by the register POS_LB. ~//

	if(abs(m_pModeDescriptorPtr->wHBackPorch-m_wBuff[0])>(m_pModeDescriptorPtr->wHBackPorch))
	{
		IC_WritByte(TWIC_P0, 0x4C, (uCHAR)m_pModeDescriptorPtr->wHBackPorch);
		IC_WritByte(TWIC_P0, 0x4D, (uCHAR)(m_pModeDescriptorPtr->wHBackPorch>>8));
		m_wBuff[0] = m_pModeDescriptorPtr->wHBackPorch;
	}
	else
	{
		IC_WritByte(TWIC_P0, 0x4C, (uCHAR)m_wBuff[0]);
		IC_WritByte(TWIC_P0, 0x4D, (uCHAR)(m_wBuff[0]>>8));
	}

}

void VPosAutoAdj()
{
	if(abs(m_pResolutionPtr->wVRes-m_wVRes)>(m_pResolutionPtr->wVRes*0.2))
	{
		//~ Difference is too large, so following the mode table. ~//
		m_wVRes=m_pResolutionPtr->wVRes;
		IC_WritByte(TWIC_P0, 0x4A, (uCHAR)m_wVRes);
		IC_WritByte(TWIC_P0, 0x4B, (uCHAR)(m_wVRes>>8));
	}
	else
	{
		//~ Reset ADC Capture VSize 0x4A, 0x4B ~//
		IC_WritByte(TWIC_P0, 0x4A, (uCHAR)m_wVRes);
		IC_WritByte(TWIC_P0, 0x4B, (uCHAR)(m_wVRes>>8));
	}
	//~ Fill in the VSync Start Point by the register POS_LB. ~//  
	if(abs(m_pModeDescriptorPtr->cVBackPorch-m_wBuff[0])>(m_pModeDescriptorPtr->cVBackPorch))
	{
		IC_WritByte(TWIC_P0, 0x4E, (uCHAR)m_pModeDescriptorPtr->cVBackPorch);
		IC_WritByte(TWIC_P0, 0x4F, (uCHAR)(m_pModeDescriptorPtr->cVBackPorch>>8));
		m_wBuff[0] = m_pModeDescriptorPtr->cVBackPorch;
	}
	else
	{
		IC_WritByte(TWIC_P0, 0x4E, (uCHAR)m_wBuff[0]);
		IC_WritByte(TWIC_P0, 0x4F, (uCHAR)(m_wBuff[0]>>8));
	}
}
unsigned char VGA_res_change_check(void)
{
	unsigned int val;
	unsigned long FrmXclk;

	FrmXclk = IC_ReadByte(TWIC_P0,0x53);
	FrmXclk = FrmXclk << 8;
	FrmXclk = FrmXclk | IC_ReadByte(TWIC_P0,0x52);
	FrmXclk = FrmXclk << 8;
	FrmXclk = FrmXclk | IC_ReadByte(TWIC_P0,0x51);

	val = IC_ReadByte(TWIC_P0,0x59);
	val = val <<8;
	val = val | IC_ReadByte(TWIC_P0,0x58);
#ifdef DEBUGMSG1	
	DBG_PRINT("-->h val=%d\n",(int)val/32);
	DBG_PRINT("-->HSYNC_PW=%d\n",(int)HSYNC_PW/32);
#endif
	if (abs((val-HSYNC_PW)/32) >= 10)
		return 1;
	
	val = FrmXclk*32/val;
#ifdef DEBUGMSG1
	DBG_PRINT("-->v val=%d\n",(int)val);
	DBG_PRINT("-->VSYNC_PW=%d\n",(int)VSYNC_PW);
#endif
	if (abs(val-VSYNC_PW) >= 12)
		return 1;
	
	return 0;
}

void ACal_output_timing(void)
{
	unsigned long FrmXclk;
	unsigned long VBi_Xclk;
	unsigned int h_size;
	unsigned int v_size;
	unsigned int DWVSZ;
	unsigned int DWVS;
	unsigned int DWHSZ;
	unsigned int DWHS;
	unsigned int PH_TOT;
	unsigned int PV_TOT;
	unsigned char PLLDIV_F;
	unsigned char PLLDIV_I;
	unsigned char PLLDIV_O;
	unsigned int PV_PW;
	unsigned int PH_PW;
	unsigned int LBprfl;
	unsigned int N;	
	unsigned long val;
	unsigned int scalling_v;
	unsigned int scalling_h;
	float Tpll;
	unsigned long XTALI;
	float hz;
	float Thi; 
	float val_d;
	float v_i_tot;
	float f;
	float val_f;
	float Temp;
	unsigned int i;
	
	XTALI = 27000000;
	DWVSZ = IC_ReadByte(TWIC_P0,0xB7);
	DWVSZ = DWVSZ << 8;
	DWVSZ = DWVSZ | IC_ReadByte(TWIC_P0,0xB6);
#ifdef VGA_DBG	
	DBG_PRINT("DWVSZ=%d\n",(int)DWVSZ);
#endif	
	DWHSZ = IC_ReadByte(TWIC_P0,0xB5);
	DWHSZ = DWHSZ << 8;
	DWHSZ = DWHSZ | IC_ReadByte(TWIC_P0,0xB4);
#ifdef VGA_DBG
	DBG_PRINT("DWHSZ=%d\n",(int)DWHSZ);
#endif

	/* get parameter from reg. */
	VBi_Xclk = IC_ReadByte(TWIC_P0,0x83);	
	VBi_Xclk = VBi_Xclk << 8;
	VBi_Xclk = VBi_Xclk | IC_ReadByte(TWIC_P0,0x82);	
	VBi_Xclk = VBi_Xclk << 8;
	VBi_Xclk = VBi_Xclk | IC_ReadByte(TWIC_P0,0x81);
	//DBG_PRINT("VBi_Xclk=%x h\n",(int)VBi_Xclk);
	
	PH_PW = IC_ReadByte(TWIC_P0,0xBD);
	PH_PW = PH_PW << 8;
	PH_PW = PH_PW | IC_ReadByte(TWIC_P0,0xBC);
#ifdef VGA_DBG	
	DBG_PRINT("PH_PW=%d\n",(int)PH_PW);
#endif

	PV_PW = IC_ReadByte(TWIC_P0,0xBF);
	PV_PW = PV_PW << 8;
	PV_PW = PV_PW | IC_ReadByte(TWIC_P0,0xBE);
#ifdef VGA_DBG	
	DBG_PRINT("PV_PW=%d\n",(int)PV_PW);
#endif

	h_size = IC_ReadByte(TWIC_P0,0x55);
	h_size = h_size << 8;
	h_size = h_size | IC_ReadByte(TWIC_P0,0x54);
#ifdef VGA_DBG	
	DBG_PRINT("h_size=%d\n",(int)h_size);
#endif

	v_size = IC_ReadByte(TWIC_P0,0x57);
	v_size = v_size << 8;
	v_size = v_size | IC_ReadByte(TWIC_P0,0x56);
#ifdef VGA_DBG	
	DBG_PRINT("v_size=%d\n",(int)v_size);
#endif

	FrmXclk = IC_ReadByte(TWIC_P0,0x53);
	FrmXclk = FrmXclk << 8;
	FrmXclk = FrmXclk | IC_ReadByte(TWIC_P0,0x52);
	FrmXclk = FrmXclk << 8;
	FrmXclk = FrmXclk | IC_ReadByte(TWIC_P0,0x51);
#ifdef VGA_DBG
	DBG_PRINT("FrmXclk=%x h\n",(int)FrmXclk);
#endif

	HSYNC_PW = IC_ReadByte(TWIC_P0,0x59);
	HSYNC_PW = HSYNC_PW << 8;
	HSYNC_PW = HSYNC_PW | IC_ReadByte(TWIC_P0,0x58);
#ifdef VGA_DBG
	DBG_PRINT("HSYNC_PW=%x h\n",(int)HSYNC_PW);
#endif

	PLLDIV_F= IC_ReadByte(TWIC_P0,0xC8)&0x7F;
	PLLDIV_I= IC_ReadByte(TWIC_P0,0xC9)&0x1F;
	PLLDIV_O= IC_ReadByte(TWIC_P0,0xCA)&0x03;
#ifdef VGA_DBG
	DBG_PRINT("PLLDIV_F=%x h\n",(int)PLLDIV_F);
	DBG_PRINT("PLLDIV_I=%x h\n",(int)PLLDIV_I);
	DBG_PRINT("PLLDIV_O=%x h\n",(int)PLLDIV_O);
#endif	
	
	/* SET SCALLING (P0_72~P0_75)*/	
	scalling_h = ((unsigned long)h_size*0x2000)/(unsigned long)DWHSZ;
	if ((h_size/2) > DWHSZ)
	{
#ifdef VGA_DBG	
		DBG_PRINT("H INPUT DIV2\n");
#endif
		IC_WritByte(TWIC_P0,0x78,IC_ReadByte(TWIC_P0,0x78)|0x80);
		scalling_h=scalling_h/2;
	}
	else
		IC_WritByte(TWIC_P0,0x78,IC_ReadByte(TWIC_P0,0x78)&0x7F);
#ifdef VGA_DBG
	DBG_PRINT("scalling_h=%d\n",(int)scalling_h);
#endif
	IC_WritByte(TWIC_P0,0x72,scalling_h&0xff);
	IC_WritByte(TWIC_P0,0x73,(scalling_h >> 8));

	
	scalling_v = ((unsigned long)v_size*0x2000)/(unsigned long)DWVSZ;
	if ((v_size/2) > DWVSZ)
	{
#ifdef VGA_DBG	
		DBG_PRINT("V INPUT DIV2\n");
#endif
		IC_WritByte(TWIC_P0,0x70,IC_ReadByte(TWIC_P0,0x70)|0x08);
		scalling_v=scalling_v/2;
	}
	else
		IC_WritByte(TWIC_P0,0x70,IC_ReadByte(TWIC_P0,0x70)&0xF7);
#ifdef VGA_DBG
	DBG_PRINT("scalling_v=%d\n",(int)scalling_v);
#endif
	IC_WritByte(TWIC_P0,0x74,scalling_v&0xff);
	IC_WritByte(TWIC_P0,0x75,(scalling_v >> 8));

	IC_WritByte(TWIC_P0,0x50,IC_ReadByte(TWIC_P0,0x50)|0x10);
	for(i=0;i<10000;i++)
	{
		if (IC_ReadByte(TWIC_P0,0x50)&0x20)
			break;
		twdDelay(10);
	}
	
	/* SET H TOTAL (P0_B8 P0_B9) */
	hz = (float)XTALI/FrmXclk;
#ifdef VGA_DBG	
	DBG_PRINT("hz=%f\n",(float)hz);
#endif
	v_i_tot = (FrmXclk*32) / HSYNC_PW;
	VSYNC_PW = v_i_tot+0.5;
#ifdef VGA_DBG
	DBG_PRINT("HSYNC_PW=%d\n",(int)HSYNC_PW/32);
	DBG_PRINT("VSYNC_PW=%d\n",(int)VSYNC_PW);
	DBG_PRINT("v_i_tot=%f\n",v_i_tot);
#endif
	Thi = (1/hz)/(VSYNC_PW);
#ifdef VGA_DBG
	DBG_PRINT("Thi=%f\n",(float)Thi);
#endif
	Tpll = 1/(((PLLDIV_F+2)/((PLLDIV_I+2)*(pow(2,(double)(PLLDIV_O+1)))))*XTALI);
#ifdef VGA_DBG
	DBG_PRINT("TPLL=%d\n",(int)Tpll);
#endif
	if (v_size ==  480 )			//fix for V detect not stable issue
		PH_TOT = ((Thi * v_size)/ (Tpll * DWVSZ))+2;
	else
		PH_TOT = ((Thi * v_size)/ (Tpll * DWVSZ))+0.5;
#ifdef VGA_DBG
	DBG_PRINT("PH_TOT=%x h\n",(int)PH_TOT);
#endif

	IC_WritByte(TWIC_P0,0xB8,PH_TOT&0xff);
	IC_WritByte(TWIC_P0,0xB9,(PH_TOT >> 8));

	/* SET V BACK PORCH (P0_B2, P0_B3)  */
	val = VBi_Xclk * (PLLDIV_F+2);
#ifdef VGA_DBG
	DBG_PRINT("BP step1 val=%lx h\n",val);
#endif
	val_f = (float)val / ((PLLDIV_I+2) * pow(2,(double)(PLLDIV_O+1)) );
#ifdef VGA_DBG
	DBG_PRINT("BP step2 val=%f\n",val_f);
#endif
	val_f = val_f / PH_TOT;
#ifdef VGA_DBG
	DBG_PRINT("BP setp3 val=%f\n",val_f);
#endif
	N = (int)val_f-1;
	f = val_f - N;
	DWVS = N - PV_PW;
#ifdef VGA_DBG	
	DBG_PRINT("N=%d\n",(int)N);
	DBG_PRINT("f=%f\n",f);
	DBG_PRINT("DWVS=%d\n",(int)DWVS);
#endif	
	IC_WritByte(TWIC_P0,0xB2,(DWVS&0xff));
	IC_WritByte(TWIC_P0,0xB3,(DWVS >> 8));

	/* SET H BACK PORCH (P0_B0, P0_B1) */
	DWHS = (PH_TOT - PH_PW - DWHSZ)/2;
#ifdef VGA_DBG	
	DBG_PRINT("DWHS=%d\n",(int)DWHS);
#endif

	IC_WritByte(TWIC_P0,0xB0,DWHS&0xff);
	IC_WritByte(TWIC_P0,0xB1,(DWHS >> 8));

	/* SET LINE BUFFER PREFILL (P0_84, P0_85) */
	if (v_size >= 1024 )
		val_d = ((3.5 * (1/hz))/(VSYNC_PW));
	else if (v_size <= 800)
		val_d = ((2.2 * (1/hz))/(VSYNC_PW));
	else
		val_d = ((2.5 * (1/hz))/(VSYNC_PW));
	val_d = val_d*XTALI + (f * PH_TOT * ((PLLDIV_I+2) * pow(2,(double)(PLLDIV_O+1)) ) / (PLLDIV_F+2));
	LBprfl = (int)(val_d+0.5);
#ifdef VGA_DBG	
	DBG_PRINT("val_d=%f h\n",val_d);
	DBG_PRINT("LBprfl=%x h\n",(int)LBprfl);
#endif

	IC_WritByte(TWIC_P0,0x84,LBprfl&0xff);
	IC_WritByte(TWIC_P0,0x85,(LBprfl >> 8));

	/* SET V TOTAL (P0_BA, P0_BB) */
	PV_TOT = (((unsigned long)PLLDIV_F+2) * XTALI) / (hz * PH_TOT * (PLLDIV_I+2)* pow(2,(double)(PLLDIV_O+1)));
#ifdef VGA_DBG
	DBG_PRINT("PV_TOT=%x h\n",(int)PV_TOT);
#endif

	IC_WritByte(TWIC_P0,0xBA,PV_TOT&0xff);
	IC_WritByte(TWIC_P0,0xBB,(PV_TOT >> 8));

	/* SET CLK FASTER (P0_70) */
	Temp = 27000000.0*m_pModeDescriptorPtr->wHTotal/((float)HSYNC_PW/32);
	if(Temp>(((float)(IC_ReadByte(TWIC_P0, 0xc8)+2)/(DPLL_DIV_I+2)/DRDIV)*27000000.0))
		IC_WritByte(TWIC_P0, 0x70, IC_ReadByte(TWIC_P0, 0x70)|0x10);
	else
		IC_WritByte(TWIC_P0, 0x70, IC_ReadByte(TWIC_P0, 0x70)&0xEF);

}

