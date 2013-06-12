#include "stinfoparser.h"

const char* PTPStorageInfoParser::stNames[] PROGMEM =
{
	msgUndefined,
	msgFixedROM,
	msgRemovableROM,	
	msgFixedRAM,		
	msgRemovableRAM	
};

const char* PTPStorageInfoParser::fstNames[] PROGMEM =
{
	msgUndefined,
	msgGenericFlat,			
	msgGenericHierarchical,	
	msgDCF					
};

const char* PTPStorageInfoParser::acNames[] PROGMEM =
{
	msgUndefined,
	msgReadWrite,						
	msgReadOnly,						
	msgReadOnly_with_Object_Deletion
};


void PTPStorageInfoParser::Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset)
{
	uint16_t	cntdn	= (uint16_t)len;
	uint8_t		*p		= (uint8_t*)pbuf;

	switch (nStage)
	{
	case 0:
		p		+= 12;
		cntdn	-= 12;
		nStage ++;
	case 1:
		PrintStorageType(&p, &cntdn);
		nStage ++;
	case 2:
		PrintFileSystemType(&p, &cntdn);
		nStage ++;
	case 3:
		PrintAccessCapability(&p, &cntdn);
		nStage ++;
	case 4:
		PrintMaxCapacity(&p, &cntdn);
		nStage ++;
	case 5:
		PrintFreeSpaceInBytes(&p, &cntdn);
		nStage ++;
	case 6:
		PrintFreeSpaceInImages(&p, &cntdn);
		nStage ++;
	case 7:
		E_Notify(PSTR("Storage Description:\t"), 0x80);
		nStage ++;
	case 8:
		if (!PrintString(&p, &cntdn))
			return;
		strByteCountDown = 0;
		nStage ++;
	case 9:
		E_Notify(PSTR("\nVolume Label:\t"), 0x80);
		nStage ++;
	case 10:
		if (!PrintString(&p, &cntdn))
			return;
		E_Notify(PSTR("\n"), 0x80);
		strByteCountDown = 0;
		nStage ++;
	}
}

void PTPStorageInfoParser::PrintStorageType(uint8_t **pp, uint16_t *pcntdn)
{
	E_Notify(PSTR("\nStorage Type:\t\t"), 0x80);
	uint16_t	type = *((uint16_t*)*pp);

	if (type >= 0 && type <= PTP_ST_RemovableRAM)
		E_Notify((char*)pgm_read_word(&stNames[type]), 0x80);
	else
		E_Notify(PSTR("Vendor Defined"), 0x80);

	E_Notify(PSTR(" (0x"), 0x80);
	PrintHex<uint16_t>(type, 0x80);
	E_Notify(PSTR(")\n"), 0x80);

	(*pp)		+= 2;
	(*pcntdn)	-= 2;
}

void PTPStorageInfoParser::PrintFileSystemType(uint8_t **pp, uint16_t *pcntdn)
{
	E_Notify(PSTR("File System Type:\t"), 0x80);
	uint16_t	type = *((uint16_t*)*pp);

	if (type >= 0 && type <= PTP_FST_DCF)
		E_Notify((char*)pgm_read_word(&fstNames[type]), 0x80);
	else
		E_Notify(PSTR("Vendor Defined"), 0x80);

	E_Notify(PSTR(" (0x"), 0x80);
	PrintHex<uint16_t>(type, 0x80);
	E_Notify(PSTR(")\n"), 0x80);

	(*pp)		+= 2;
	(*pcntdn)	-= 2;
}

void PTPStorageInfoParser::PrintAccessCapability(uint8_t **pp, uint16_t *pcntdn)
{
	E_Notify(PSTR("Access Capability:\t"), 0x80);
	uint16_t	type = *((uint16_t*)*pp);

	if (type >= 0 && type <= PTP_AC_ReadOnly_with_Object_Deletion)
		E_Notify((char*)pgm_read_word(&acNames[type]), 0x80);
	else
		E_Notify(PSTR("Vendor Defined"), 0x80);

	E_Notify(PSTR(" (0x"), 0x80);
	PrintHex<uint16_t>(type, 0x80);
	E_Notify(PSTR(")\n"), 0x80);

	(*pp)		+= 2;
	(*pcntdn)	-= 2;
}

void PTPStorageInfoParser::PrintMaxCapacity(uint8_t **pp, uint16_t *pcntdn)
{
	E_Notify(PSTR("Max Capacity:\t\t0x"), 0x80);
	uint32_t	loword = *((uint32_t*)*pp);

	(*pp)		+= 4;
	(*pcntdn)	-= 4;

	uint32_t	hiword = *((uint32_t*)*pp);

	(*pp)		+= 4;
	(*pcntdn)	-= 4;

	PrintHex<uint32_t>(hiword, 0x80);
	PrintHex<uint32_t>(loword, 0x80);
	E_Notify(PSTR("\n"), 0x80);
}

void PTPStorageInfoParser::PrintFreeSpaceInBytes(uint8_t **pp, uint16_t *pcntdn)
{
	E_Notify(PSTR("Free Space:\t\t0x"), 0x80);
	uint32_t	loword = *((uint32_t*)*pp);

	(*pp)		+= 4;
	(*pcntdn)	-= 4;

	uint32_t	hiword = *((uint32_t*)*pp);

	(*pp)		+= 4;
	(*pcntdn)	-= 4;

	PrintHex<uint32_t>(hiword, 0x80);
	PrintHex<uint32_t>(loword, 0x80);
	E_Notify(PSTR(" bytes\n"), 0x80);
}

void PTPStorageInfoParser::PrintFreeSpaceInImages(uint8_t **pp, uint16_t *pcntdn)
{
	E_Notify(PSTR("Free Space:\t\t0x"), 0x80);
	uint32_t	loword = *((uint32_t*)*pp);

	(*pp)		+= 4;
	(*pcntdn)	-= 4;

	PrintHex<uint32_t>(loword, 0x80);
	E_Notify(PSTR(" images\n"), 0x80);
}

bool PTPStorageInfoParser::PrintString(uint8_t **pp, uint16_t *pcntdn)
{
	if (!(*pcntdn))
		return true;

	if ( !strByteCountDown )
	{
		if ( !(**pp) )
		{
			(*pp) ++;
			(*pcntdn) --;
			return true;
		}
		strByteCountDown = ((**pp) << 1);
		(*pp) ++;
		(*pcntdn) --;
	}

	for (; strByteCountDown && (*pcntdn); strByteCountDown--, (*pcntdn)--, (*pp)++)
	{
		if ( !(strByteCountDown & 1) && ((**pp) > 0))
			Serial.print((unsigned char)(**pp));
	}
	return (strByteCountDown == 0);
}
