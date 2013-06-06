/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com
*/
#if !defined(__NKVALUETITLES_H__)
#define __NKVALUETITLES_H__

#include <inttypes.h>
#include <avr/pgmspace.h>
#include <valuelist.h>

typedef uint16_t			VT_APERTURE;
typedef uint8_t			VT_MODE;
typedef uint16_t		VT_WB;
typedef uint32_t		VT_SHSPEED;
typedef uint8_t			VT_PSTYLE;
typedef uint16_t		VT_ISO;
typedef int16_t			VT_EXPCOMP;
typedef uint8_t			VT_COMPRESSION;
typedef uint8_t			VT_FOCUSMODE;
typedef uint8_t			VT_FOCUSAREA;

#define VT_MODE_TEXT_LEN			4
#define VT_WB_TEXT_LEN				4
#define VT_SHSPEED_TEXT_LEN			5
#define VT_PSTYLE_TEXT_LEN			4
#define VT_ISO_TEXT_LEN				5
#define VT_EXPCOMP_TEXT_LEN			5
#define VT_APT_TEXT_LEN				4
#define VT_COMPRESSION_TEXT_LEN		6
#define VT_FOCUSMODE_TEXT_LEN		5
#define VT_FOCUSAREA_TEXT_LEN		4

//3872x2592
//2896x1944
//1936x1296

const ValueTitle<VT_FOCUSAREA, VT_FOCUSAREA_TEXT_LEN> FocusAreaTitles[] PROGMEM = 
{
	{0x00,	{'_', '+', '_', 0 }	},
	{0x01,	{'+', '_', '_', 0 }	},
	{0x02,	{'_', '_', '+', 0 }	}
};

const ValueTitle<VT_FOCUSMODE, VT_FOCUSMODE_TEXT_LEN> FocusModeTitles[] PROGMEM = 
{
	{0x00,	{'A', 'F', '-', 'S', 0 }	},
	{0x01,	{'A', 'F', '-', 'C', 0 }	},
	{0x02,	{'A', 'F', '-', 'A', 0 }	},
	{0x04,	{'M', 'F', ' ', ' ', 0 }	}
};

const ValueTitle<VT_COMPRESSION, VT_COMPRESSION_TEXT_LEN> CompressionTitles[] PROGMEM = 
{
	{0x00,	{'B', 'A', 'S', 'I', 'C', 0 }	},
	{0x01,	{'N', 'O', 'R', 'M', ' ', 0 }	},
	{0x02,	{'F', 'I', 'N', 'E', ' ', 0 }	},
	{0x03,	{'R', 'A', 'W', ' ', ' ', 0 }	},
	{0x04,	{'R', 'A', 'W', '+', 'B', 0 }	}
};

const ValueTitle<VT_APERTURE, VT_APT_TEXT_LEN> ApertureTitles[] PROGMEM = 
{
	{0x015E,	{'3', '.', '5', 0 }	},
	{0x0190,	{'4', '.', '0', 0 } },
	{0x01C2,	{'4', '.', '5', 0 }	},
	{0x01E0,	{'4', '.', '8', 0 }	},
	{0x01F4,	{' ', ' ', '5', 0 } },
	{0x0230,	{'5', '.', '6', 0 }	},
	{0x0276,	{'6', '.', '3', 0 }	},
	{0x02C6,	{'7', '.', '1', 0 }	},
	{0x0320,	{' ', ' ', '8', 0 }	},
	{0x0384,	{' ', ' ', '9', 0 }	},
	{0x03E8,	{' ', '1', '0', 0 }	},
	{0x044C,	{' ', '1', '1', 0 }	},
	{0x0514,	{' ', '1', '3', 0 }	},
	{0x0578,	{' ', '1', '4', 0 }	},
	{0x0640,	{' ', '1', '6', 0 }	},
	{0x0708,	{' ', '1', '8', 0 }	},
	{0x07D0,	{' ', '2', '0', 0 }	},
	{0x0898,	{' ', '2', '2', 0 }	},
	{0x09C4,	{' ', '2', '5', 0 }	},
	{0x0B54,	{' ', '2', '9', 0 }	},
	{0x0C80,	{' ', '3', '2', 0 }	},
	{0x0E10,	{' ', '3', '6', 0 }	}
};

const ValueTitle<VT_SHSPEED, VT_SHSPEED_TEXT_LEN> ShutterSpeedTitles[] PROGMEM =
{
	{0x00000002,	{'4','0','0','0',0} },
	{0x00000003,	{'3','2','0','0',0} },
	{0x00000004,	{'2','5','0','0',0} },
	{0x00000005,	{'2','0','0','0',0} },
	{0x00000006,	{'1','6','0','0',0} },
	{0x00000008,	{'1','2','5','0',0} },
	{0x0000000A,	{'1','0','0','0',0} },
	{0x0000000C,	{' ','8','0','0',0} },
	{0x0000000F,	{' ','6','4','0',0} },
	{0x00000014,	{' ','5','0','0',0} },
	{0x00000019,	{' ','4','0','0',0} },
	{0x0000001F,	{' ','3','2','0',0} },
	{0x00000028,	{' ','2','5','0',0} },
	{0x00000032,	{' ','2','0','0',0} },
	{0x0000003E,	{' ','1','6','0',0} },
	{0x00000050,	{' ','1','2','5',0} },
	{0x00000064,	{' ','1','0','0',0} },
	{0x0000007D,	{' ',' ','8','0',0} },
	{0x000000A6,	{' ',' ','6','0',0} },
	{0x000000C8,	{' ',' ','5','0',0} },
	{0x000000FA,	{' ',' ','4','0',0} },
	{0x0000014D,	{' ',' ','3','0',0} },
	{0x00000190,	{' ',' ','2','5',0} },
	{0x000001F4,	{' ',' ','2','0',0} },
	{0x0000029A,	{' ',' ','1','5',0} },
	{0x00000301,	{' ',' ','1','3',0} },
	{0x000003E8,	{' ',' ','1','0',0} },
	{0x000004E2,	{' ',' ',' ','8',0} },
	{0x00000682,	{' ',' ',' ','6',0} },
	{0x000007D0,	{' ',' ',' ','5',0} },
	{0x000009C4,	{' ',' ',' ','4',0} },
	{0x00000D05,	{' ',' ',' ','3',0} },
	{0x00000FA0,	{' ','2','.','5',0} },
	{0x00001388,	{' ',' ',' ','2',0} },
	{0x0000186A,	{' ','1','.','6',0} },
	{0x00001E0C,	{' ','1','.','3',0} },
	{0x00002710,	{' ',' ','1','"',0} },
	{0x000032C8,	{'1','.','3','"',0} },
	{0x00003E80,	{'1','.','6','"',0} },
	{0x00004E20,	{' ',' ','2','"',0} },
	{0x000061A8,	{'2','.','5','"',0} },
	{0x00007530,	{' ',' ','3','"',0} },
	{0x00009C40,	{' ',' ','4','"',0} },
	{0x0000C350,	{' ',' ','5','"',0} },
	{0x0000EA60,	{' ',' ','6','"',0} },
	{0x00013880,	{' ',' ','8','"',0} },
	{0x000186A0,	{' ','1','0','"',0} },
	{0x0001FBD0,	{' ','1','3','"',0} },
	{0x000249F0,	{' ','1','5','"',0} },
	{0x00030D40,	{' ','2','0','"',0} },
	{0x0003D090,	{' ','2','5','"',0} },
	{0x000493E0,	{' ','3','0','"',0} },
	{0xFFFFFFFF,	{'B','u','l','B',0} }
};

// 
const ValueTitle<VT_ISO, VT_ISO_TEXT_LEN> IsoTitles[] PROGMEM =
{
	{0x0064,	{' ','1','0','0',0} },
	{0x007D,	{' ','1','2','5',0} },
	{0x00A0,	{' ','1','6','0',0} },
	{0x00C8,	{' ','2','0','0',0} },
	{0x00FA,	{' ','2','5','0',0} },
	{0x0140,	{' ','3','2','0',0} },
	{0x0190,	{' ','4','0','0',0} },
	{0x01F4,	{' ','5','0','0',0} },
	{0x0280,	{' ','6','4','0',0} },
	{0x0320,	{' ','8','0','0',0} },
	{0x03E8,	{'1','0','0','0',0} },
	{0x04E2,	{'1','2','5','0',0} },
	{0x0640,	{'1','6','0','0',0} },
	{0x07D0,	{'2','0','0','0',0} },
	{0x09C4,	{'2','5','0','0',0} },
	{0x0C80,	{'3','2','0','0',0} },
	{0x0FA0,	{'4','0','0','0',0} },
	{0x1388,	{'5','0','0','0',0} },
	{0x1900,	{'6','4','0','0',0} },
	{0x1F40,	{'H','i','0','3',0} },
	{0x2710,	{'H','i','0','7',0} },
	{0x3200,	{'H','i',' ','1',0} },
	{0x6400,	{'h','i',' ','2',0} }
};

// Exposure Compensation Title Array
const ValueTitle<VT_EXPCOMP, VT_EXPCOMP_TEXT_LEN> ExpCompTitles[] PROGMEM =
{
	{0xEC78,	{'+','5','.','0',0} },				
	{0xEDC6,	{'+','4','.','7',0} },				
	{0xEF13,	{'+','4','.','3',0} },				
	{0xF060,	{'+','4','.','0',0} },				
	{0xF1AE,	{'+','3','.','7',0} },				
	{0xF2FB,	{'+','3','.','3',0} },				
	{0xF448,	{'+','3','.','0',0} },				
	{0xF596,	{'+','2','.','7',0} },				
	{0xF6E3,	{'+','2','.','3',0} },
	{0xF830,	{'+','2','.','0',0} },
	{0xF97E,	{'+','1','.','7',0} },				
	{0xFACB,	{'+','1','.','3',0} },				
	{0xFC18,	{'+','1','.','0',0} },
	{0xFD66,	{'+','0','.','7',0} },
	{0xFEB3,	{'+','0','.','3',0} },				
	{0x0000,	{'0',' ',' ',' ',0} },
	{0x014D,	{'-','0','.','3',0} },				
	{0x029A,	{'-','0','.','7',0} },				
	{0x03E8,	{'-','1','.','0',0} },				
	{0x0535,	{'-','1','.','3',0} },				
	{0x0682,	{'-','1','.','7',0} },				
	{0x07D0,	{'-','2','.','0',0} },				
	{0x091D,	{'-','2','.','3',0} },				
	{0x0A6A,	{'-','2','.','7',0} },				
	{0x0BB8,	{'-','3','.','0',0} },				
	{0x0D05,	{'-','3','.','3',0} },				
	{0x0E52,	{'-','3','.','7',0} },				
	{0x0FA0,	{'-','4','.','0',0} },				
	{0x10ED,	{'-','4','.','3',0} },				
	{0x123A,	{'-','4','.','7',0} },				
	{0x1388,	{'-','5','.','0',0} }
};

// White Balance Title Array
const ValueTitle<VT_WB, VT_WB_TEXT_LEN> WbTitles[] PROGMEM = 
{ 
	{0x0002,	{'A','W','B',0} },            // Auto White Balance
	{0x0004,	{'D','a','y',0} },            // Daylight
	{0x0005,	{'F','l','r',0} },            // Fluoriscent
	{0x0006,	{'T','n','g',0} },            // Tungsteen 2
	{0x0007,	{'S','t','r',0} },            // Strobe 
	{0x8010,	{'C','l','d',0} },            // Clouds
	{0x8011,	{'S','h','d',0} },            // Shade
	{0x8013,	{'P','R','E',0} }             // Preset
};

// Picture Style Title Array
const ValueTitle<VT_PSTYLE, VT_PSTYLE_TEXT_LEN> PStyleTitles[] PROGMEM = 
{ 
	{0x21,	{'U','s','1',0} },            // User 1
	{0x22,	{'U','s','2',0} },            // User 2
	{0x23,	{'U','s','3',0} },            // User 3
	{0x81, 	{'S','t','d',0} },            // Standard
	{0x82, 	{'P','r','t',0} },            // Portrait
	{0x83, 	{'L','n','d',0} },            // Landscape
	{0x84,	{'N','t','l',0} },            // Neutral
	{0x85, 	{'F','t','h',0} },            // Faithful
	{0x86,	{'M','o','n',0} }             // Monochrome
};

#define VT_APT_COUNT             sizeof(ApertureTitles)     / (sizeof(VT_APERTURE)   + VT_APT_TEXT_LEN)
#define VT_MODE_COUNT            sizeof(ModeTitles)         / (sizeof(VT_MODE)       + VT_MODE_TEXT_LEN)
#define VT_WB_COUNT              sizeof(WbTitles)           / (sizeof(VT_WB)         + VT_WB_TEXT_LEN)
#define VT_SHSPEED_COUNT         sizeof(ShutterSpeedTitles) / (sizeof(VT_SHSPEED)    + VT_SHSPEED_TEXT_LEN)
#define VT_PSTYLE_COUNT          sizeof(PStyleTitles)       / (sizeof(VT_PSTYLE)     + VT_PSTYLE_TEXT_LEN)
#define VT_ISO_COUNT             sizeof(IsoTitles)          / (sizeof(VT_ISO)        + VT_ISO_TEXT_LEN)
#define VT_EXPCOMP_COUNT         sizeof(ExpCompTitles)      / (sizeof(VT_EXPCOMP)    + VT_EXPCOMP_TEXT_LEN)
#define VT_COMPRESSION_COUNT     sizeof(CompressionTitles)  / (sizeof(VT_COMPRESSION)+ VT_COMPRESSION_TEXT_LEN)
#define VT_FOCUSMODE_COUNT		 sizeof(FocusModeTitles)	/ (sizeof(VT_FOCUSMODE)	 + VT_FOCUSMODE_TEXT_LEN)
#define VT_FOCUSAREA_COUNT		 sizeof(FocusAreaTitles)	/ (sizeof(VT_FOCUSAREA)	 + VT_FOCUSAREA_TEXT_LEN)

#endif // __NKVALUETITLES_H__
