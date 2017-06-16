//#include "ModeTbl.h"
//#include "vga_system.h"
#include "ModeData.h"
#include "VGADetect.h"

//#define bDSPPLL_100		0x12
//#define bDSPPLL_108		0x00
//#define bDSPPLL_120		0x04	
//#define bDSPPLL_136		0x08
//#define bDSPPLL_140		0x0C

//-----------------------------------------------------------
// Mode Table: Pixel Clock=(H_Total*32/H_Period)*14.318Mhz
//Refresh Rate=Pixel Clock/(H_Total*V_Total)
//-----------------------------------------------------------
//wHTotal      -> (HTotal = HsyncWidth + Hsst + HRes + HFrontPorch)
//wVTotal      -> 
//cVBackPorch  -> (BackPorch + TopBorder + SyncWidth)  
//wHBackPorch  -> (BackPorch + LeftBorder + SyncWidth)
//cResID       -> (Index into Resolution table)
//cModeFlags0  -> HSync[7] VSync[6]
//cModeFlags1  -> PLL
//cModeFlags2  -> bVCORange[7,6] bChargePump[5,4,3] bAPLL_Sel_HighFreq[0]
//wHPeriodCount-> Horizontal Period in 32 HSync lines XCLK count
//dwVPeriodCount-> Vertical Sync count with XCLK 

MODE_DESCRIPTOR code ModeTable[] = {
// wHTotal, wVTotal, wHBackPorchc,VBackPorch , cResID, cModeFlags0, cModeFlags1,	cModeFlags2, wHPeriodCount,	        dwVPeriodCount			  
// 0 640x480@75Hz	 
  {840,     500,     120,         16,          2,      0x00,        bDSPPLL_3, 	0x50,        HSYNC_PERIOD(37.500),	VSYNC_PERIOD(75.000)}, 
// 1 848x480@60Hz	                                                              
  {1088,    517,     112,         23,          16,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(31.020),	VSYNC_PERIOD(60.000)}, 
// 2 640x480@72Hz                                                               
  {832,     520,     120,         20,          2,      0x00,        bDSPPLL_3, 	0x50,        HSYNC_PERIOD(37.861),	VSYNC_PERIOD(72.809)}, 
// 3 640x480@60Hz	                                                              
  {800,     525,     40,          25,          2,      0x00,        bDSPPLL_3, 	0x50,        HSYNC_PERIOD(31.469),	VSYNC_PERIOD(59.940)}, 
// 4 800x600@56Hz	 
  {1024,    625,     128,         22,          15,     0xC0,        bDSPPLL_2, 	0x50,        HSYNC_PERIOD(35.156),	VSYNC_PERIOD(56.250)}, 
// 5 800x600@75Hz	 
  {1056,    625,     160,         21,          15,     0xC0,        bDSPPLL_1, 	0x51,        HSYNC_PERIOD(46.875),	VSYNC_PERIOD(75.000)}, 
// 6 800x600@60Hz	 
  {1056,    628,     88,          23,          15,     0xC0,        bDSPPLL_2, 	0x16,        HSYNC_PERIOD(37.879),	VSYNC_PERIOD(60.317)}, 
// 7 800x600@72Hz	 
  {1040,    666,     64,          23,          15,     0xC0,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(48.077),	VSYNC_PERIOD(72.188)}, 
// 8 1280x720@60Hz	 
  {1650,    750,     220,         20,          29,     0xC0,        bDSPPLL_3, 	0x31,        HSYNC_PERIOD(45.000),	VSYNC_PERIOD(60.000)}, 
// 9 1280x768@60Hz	 
  {1440,    790,     80,          12,          30,     0x80,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(47.396),	VSYNC_PERIOD(59.995)}, 
// 10 1360x768@60Hz	 
  {1792,    795,     256,         18,          34,     0xC0,        bDSPPLL_3, 	0x71,        HSYNC_PERIOD(47.712),	VSYNC_PERIOD(60.015)}, 
// 11 1366x768@60Hz	 
  {1792,    798,     213,         24,          35,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(47.712),	VSYNC_PERIOD(59.790)}, 
// 12 1280x768@60Hz	 
  {1664,    798,     192,         20,          30,     0x40,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(47.776),	VSYNC_PERIOD(59.870)}, 
// 13 1366x768@60Hz	 
  {1500,    800,     64,          28,          35,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(48.000),	VSYNC_PERIOD(60.000)}, 
// 14 1366x768@70Hz	 
  {1500,    800,     179,         28,          35,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(48.000),	VSYNC_PERIOD(70.000)}, 	  
// 15 1024x768@75Hz	 
  {1312,    800,     176,         28,          19,     0xC0,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(60.023),	VSYNC_PERIOD(75.029)}, 
// 16 1280x768@75Hz	 
  {1696,    805,     208,         27,          30,     0x40,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(60.289),	VSYNC_PERIOD(74.893)}, 
// 17 1024x768@60Hz	 
  {1344,    806,     160,         29,          19,     0x00,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(48.363),	VSYNC_PERIOD(60.004)}, 
// 18 1024x768@70Hz	 
  {1328,    806,     144,         29,          19,     0x00,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(56.476),	VSYNC_PERIOD(70.069)}, 		
// 19 1024x768@59Hz	 
  {1328,    813,     144,         29,          19,     0x00,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(48.193),	VSYNC_PERIOD(59.278)}, 	  
// 20 1280x800@60Hz	 
  {1440,    823,     80,          14,          31,     0x80,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(49.306),	VSYNC_PERIOD(59.910)}, 
// 21 1280x800@60Hz	 
  {1680,    831,     200,         22,          31,     0x40,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(49.702),	VSYNC_PERIOD(59.810)}, 
// 22 1280x800@75Hz	 
  {1696,    838,     208,         29,          31,     0x40,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(62.795),	VSYNC_PERIOD(74.934)}, 
// 23 1152x864@75Hz	 
  {1600,    900,     256,         32,          24,     0x00,        bDSPPLL_4, 	0x19,        HSYNC_PERIOD(67.500),	VSYNC_PERIOD(75.000)}, 
// 24 1440x900@60Hz	 
  {1600,    926,     80,          17,          38,     0x80,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(55.469),	VSYNC_PERIOD(59.901)}, 
// 25 1440x900@60Hz	 
  {1904,    934,     232,         25,          38,     0x40,        bDSPPLL_3, 	0x71,        HSYNC_PERIOD(55.935),	VSYNC_PERIOD(59.887)}, 
// 26 1440x900@75Hz	 
  {1936,    942,     248,         33,          38,     0x40,        bDSPPLL_4, 	0x71,        HSYNC_PERIOD(70.635),	VSYNC_PERIOD(74.984)}, 
// 27 1600x900@60Hz	 
  {1800,    1000,    96,          96,          39,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(60.000),	VSYNC_PERIOD(60.000)}, 
// 28 1280x960@60Hz	 
  {1800,    1000,    312,         36,          32,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(60.000),	VSYNC_PERIOD(60.000)}, 
// 29 1680x1050@75Hz	 
  {2272,    1053,    296,         40,          41,     0x40,        bDSPPLL_5, 	0x51,        HSYNC_PERIOD(82.306),	VSYNC_PERIOD(74.892)}, 
// 30 1280x1024@60Hz	 
  {1688,    1066,    248,         38,          33,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(63.981),	VSYNC_PERIOD(60.020)}, //
// 31 1280x1024@75Hz	 																												   
  {1688,    1066,    248,         38,          33,     0xC0,        bDSPPLL_4, 	0x71,        HSYNC_PERIOD(79.976),	VSYNC_PERIOD(75.025)}, //
// 32 1400x1050@60Hz	 
  {1560,    1080,    80,          23,          37,     0x80,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(64.744),	VSYNC_PERIOD(59.948)}, 
// 33 1680x1050@60Hz	 
  {1840,    1080,    80,          21,          41,     0x80,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(64.674),	VSYNC_PERIOD(59.883)}, 
// 34 1680x1050@60Hz	 
  {2240,    1089,    280,         30,          41,     0x40,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(65.290),	VSYNC_PERIOD(59.954)}, 
// 35 1400x1050@60Hz	 
  {1864,    1089,    232,         32,          37,     0x40,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(65.317),	VSYNC_PERIOD(59.978)}, 
// 36 1400x1050@75Hz	 
  {1896,    1099,    248,         42,          37,     0x40,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(82.278),	VSYNC_PERIOD(74.867)}, 
// 37 1920x1080@60Hz	 
  {2200,    1125,    148,         36,          42,     0xC0,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(67.500),	VSYNC_PERIOD(60.000)}, 
// 38 1600x1200@60Hz	 
  {2160,    1250,    304,         46,          40,     0xC0,        bDSPPLL_3, 	0x71,        HSYNC_PERIOD(75.000),	VSYNC_PERIOD(60.000)}, 
// 39 1600x1200@65Hz	 
  {2160,    1250,    304,         46,          40,     0xC0,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(81.250),	VSYNC_PERIOD(65.000)}, 
// 40 1600x1200@70Hz	 
  {2160,    1250,    304,         46,          40,     0xC0,        bDSPPLL_4, 	0x7D,        HSYNC_PERIOD(87.500),	VSYNC_PERIOD(70.000)}, 
// 41 1600x1200@75Hz	 
  {2160,    1250,    304,         46,          40,     0xC0,        bDSPPLL_4, 	0x71,        HSYNC_PERIOD(93.750),	VSYNC_PERIOD(75.000)}, 
// 42 1920x1200@60Hz	 
  {2080,    1235,    80,          26,          43,     0x80,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(74.038),	VSYNC_PERIOD(59.950)}, 
// 43 1920x1200@60Hz	 
  {2592,    1245,    336,         36,          43,     0x40,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(74.556),	VSYNC_PERIOD(59.885)}, 
// 44 1920x1200@75Hz	 
  {2608,    1255,    208,         46,          43,     0x40,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(94.038),	VSYNC_PERIOD(74.930)}, 
// 45 1920x1440@60Hz	 
  {2600,    1500,    344,         56,          44,     0x40,        bDSPPLL_3, 	0x51,        HSYNC_PERIOD(90.000),	VSYNC_PERIOD(60.000)}, 
// 46 1920x1440@75Hz	 
  {2640,    1500,    352,         56,          44,     0x40,        bDSPPLL_4, 	0x51,        HSYNC_PERIOD(112.500),	VSYNC_PERIOD(75.000)}, 
};





//#define TOTALMODES (sizeof(ModeTable)/sizeof(MODE_DESCRIPTOR)-1)
uCHAR code TOTALMODES=(sizeof(ModeTable)/sizeof(MODE_DESCRIPTOR)-1);



//---------------------------------------------------
// Resolution Table 
//---------------------------------------------------
RESOLUTION code ResolutionTable[] = {
    { 512, 384 },  // 0
    { 560, 384 },  // 1
    { 640, 480 },  // 2			
    { 640, 400 },  // 3
    { 640, 350 },  // 4
    { 640, 870 },  // 5
    { 640, 200 },  // 6
    { 720, 480 },  // 7
    { 720, 574 },  // 8
    { 720, 576 },  // 9
    { 720, 486 },  // 10                                                                                          
    { 720, 400 },  // 11
    { 720, 350 },  // 12
    { 720, 483 },  // 13
    { 752, 484 },  // 14
    { 800, 600 },  // 15 		
    { 848, 480 },  // 16
    { 852, 480 },  // 17
    { 960, 720 },  // 18
    { 1024, 768 }, // 19		
    { 1024, 1024 }, // 20
    { 1053, 754 },  // 21
    { 1056, 768 },  // 22
    { 1120, 750 },  // 23
    { 1152, 864 },  // 24
    { 1152, 870 },  // 25
    { 1152, 900 },  // 26
    { 1184, 884 },  // 27
    { 1280, 1024 }, // 28		
    { 1280, 720 },  // 29
	{ 1280, 768 },  // 30
	{ 1280, 800 },  // 31
	{ 1280, 960 },  // 32
	{ 1280, 1024 }, // 33
	{ 1360, 768 },  // 34
	{ 1366, 768 },  // 35
    { 1360, 1024 }, // 36
    { 1400, 1050 }, // 37
    { 1440, 900 },  // 38
	{ 1600, 900 },  // 39
    { 1600, 1200 }, // 40
    { 1680, 1050 }, // 41
    { 1920, 1080 }, // 42
	{ 1920, 1200 }, // 43
	{ 1920, 1440 }, // 44
};

//MODE_DESCRIPTOR code *m_pModeDescriptorPtr;
//RESOLUTION	code *m_pResolutionPtr;
//ALT_HTOTAL 	code *m_pAltHTotalPtr;

uCHAR code SupportMode[]={
	40,		//640x480@60
	57,		//800x600@60
	73,		//800x600@72
	90,		//1024x768@60
	94,		//1024x768@70
	136,	//1280x1024@60
};

uCHAR code SupportModeNum = sizeof(SupportMode)/sizeof(SupportMode[0]);

uCHAR code pstrSupport[]={"Not Support !\r\n"};
uCHAR code pstrOutrange[]={"Out Range !\r\n"};
uCHAR code pstrNotLock[]={"Not Locked !\r\n"};
uCHAR code pstrChange[]={"Sync Change !\r\n"};
