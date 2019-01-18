#include "ptpobjinfoparser.h"

const char* const PTPObjInfoParser::acNames[] PROGMEM = 
{
	msgUndefined,
	msgAssociation,	
	msgScript,		
	msgExecutable,	
	msgText,			
	msgHTML,			
	msgDPOF,			
	msgAIFF,	 		
	msgWAV,			
	msgMP3,		
	msgAVI,			
	msgMPEG,			
	msgASF,			
	msgQT			
};

const char* const PTPObjInfoParser::imNames[] PROGMEM = 
{
	msgUndefined,
	msgEXIF_JPEG,			
	msgTIFF_EP,			
	msgFlashPix,			
	msgBMP,				
	msgCIFF,				
	msgUndefined_0x3806,	
	msgGIF,				
	msgJFIF,				
	msgPCD,				
	msgPICT,				
	msgPNG,				
	msgUndefined_0x380C,	
	msgTIFF,				
	msgTIFF_IT,			
	msgJP2,				
	msgJPX,				
};

void PTPObjInfoParser::PrintFormat(uint16_t op)
{
	Serial.print(op, HEX);
	Serial.print("\t");
	//E_Notify(msgTab,0x80);

	if ((((op >> 8) & 0xFF) == 0x30) && ((op & 0xFF) <= (PTP_OFC_QT & 0xFF)))
		E_Notify((char*)pgm_read_word(&acNames[(op & 0xFF)]), 0x80);
	else
		if ((((op >> 8) & 0xFF) == 0x38) && ((op & 0xFF) <= (PTP_OFC_JPX & 0xFF)))
			E_Notify((char*)pgm_read_word(&imNames[(op & 0xFF)]), 0x80);
		else
		{
			switch (op)
			{
			case MTP_OFC_Undefined_Firmware:
				E_Notify(msgUndefined_Firmware, 0x80);
				break;
			case MTP_OFC_Windows_Image_Format:	
				E_Notify(msgWindows_Image_Format, 0x80);
				break;
			case MTP_OFC_Undefined_Audio:			
				E_Notify(msgUndefined_Audio, 0x80);
				break;
			case MTP_OFC_WMA:						
				E_Notify(msgWMA, 0x80);
				break;
			case MTP_OFC_OGG:						
				E_Notify(msgOGG, 0x80);
				break;
			case MTP_OFC_AAC:						
				E_Notify(msgAAC, 0x80);
				break;
			case MTP_OFC_Audible:					
				E_Notify(msgAudible, 0x80);
				break;
			case MTP_OFC_FLAC:					
				E_Notify(msgFLAC, 0x80);
				break;
			case MTP_OFC_Undefined_Video:			
				E_Notify(msgUndefined_Video, 0x80);
				break;
			case MTP_OFC_WMV:						
				E_Notify(msgWMV, 0x80);
				break;
			case MTP_OFC_MP4_Container:			
				E_Notify(msgMP4_Container, 0x80);
				break;
			case MTP_OFC_MP2:						
				E_Notify(msgMP2, 0x80);
				break;
			case MTP_OFC_3GP_Container:
				E_Notify(msg3GP_Container, 0x80);
				break;
			default:
				E_Notify(PSTR("Vendor defined"), 0x80);
			}
		}
	E_Notify(PSTR("\r\n"), 0x80);
}

void PTPObjInfoParser::Parse(const uint16_t len, const uint8_t *pbuf,
        const uint32_t &offset __attribute__((unused)))
{
	uint16_t cntdn = (uint16_t)len;
	uint8_t	 *p = (uint8_t*)pbuf;

	switch (nStage)
	{
	case 0:
		p	+= 12;
		cntdn	-= 12;
		nStage	++;
	case 1:
		E_Notify(PSTR("Storage ID:\t\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 2:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.StorageID = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 3:
		E_Notify(PSTR("\r\nObject Format:\t\t"), 0x80);
		theBuffer.valueSize = 2;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 4:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ObjectFormat = *((uint16_t*)theBuffer.pValue);
		PrintFormat(*((uint16_t*)theBuffer.pValue));
		nStage	++;
	case 5:
		E_Notify(PSTR("Protection Status:\t"), 0x80);
		theBuffer.valueSize = 2;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 6:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ProtectionStatus = *((uint16_t*)theBuffer.pValue);
		PrintHex<uint16_t>(*((uint16_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 7:
		E_Notify(PSTR("\r\nObject Compressed Size:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 8:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ObjectCompressedSize = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 9:
		E_Notify(PSTR("\r\nThumb Format:\t\t"), 0x80);
		theBuffer.valueSize = 2;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 10:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ThumbFormat = *((uint16_t*)theBuffer.pValue);
		PrintFormat(*((uint16_t*)theBuffer.pValue));
		nStage	++;
	case 11:
		E_Notify(PSTR("Thumb Compressed Size:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 12:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ThumbCompressedSize = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 13:
		E_Notify(PSTR("\r\nThumb Pix Width:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 14:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ThumbPixWidth = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 15:
		E_Notify(PSTR("\r\nThumb Pix Height:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 16:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ThumbPixHeight = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 17:
		E_Notify(PSTR("\r\nImage Pix Width:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 18:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ImagePixWidth = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 19:
		E_Notify(PSTR("\r\nImage Pix Height:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 20:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ImagePixHeight = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 21:
		E_Notify(PSTR("\r\nImage Bit Depth:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 22:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ImageBitDepth = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 23:
		E_Notify(PSTR("\r\nParent Object:\t\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 24:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.ParentObject = *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 25:
		E_Notify(PSTR("\r\nAssociation Type:\t"), 0x80);
		theBuffer.valueSize = 2;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 26:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.AssociationType = *((uint16_t*)theBuffer.pValue);
		PrintHex<uint16_t>(*((uint16_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 27:
		E_Notify(PSTR("\r\nAssociation Description:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 28:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.AssociationDescription =
                        *((uint32_t*)theBuffer.pValue);
		PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 29:
		E_Notify(PSTR("\r\nSequence Number:\t"), 0x80);
		theBuffer.valueSize = 4;
		valueParser.Initialize(&theBuffer);
		nStage	++;
	case 30:
		if (!valueParser.Parse(&p, &cntdn)) {
			return;
                }
                objectinfo_ds.SequenceNumber = *((uint32_t*)theBuffer.pValue);
                PrintHex<uint32_t>(*((uint32_t*)theBuffer.pValue), 0x80);
		nStage	++;
	case 31:
		E_Notify(PSTR("\r\nFile Name:\t\t"), 0x80);
		arrayParser.Initialize(1, 2, &theBuffer);
		nStage	++;
	case 32:
                objectinfo_ds.FilenameLength = *((uint8_t*)&p);
		if (!arrayParser.Parse(&p, &cntdn,
                        (PTP_ARRAY_EL_FUNC)&PrintChar)) {
			return;
                }
                E_Notify(PSTR("\r\nFileName Length:\t"), 0x80);
                PrintHex<uint8_t>(objectinfo_ds.FilenameLength, 0x80);
		nStage	++;
	case 33:
		E_Notify(PSTR("\r\nDate Created:\t\t"), 0x80);
		arrayParser.Initialize(1, 2, &theBuffer);
		nStage	++;
	case 34:
                objectinfo_ds.DateCreatedLength = *((uint8_t*)&p);
		if (!arrayParser.Parse(&p, &cntdn,
                        (PTP_ARRAY_EL_FUNC)&PrintChar)) {
			return;
                }
                E_Notify(PSTR("\r\nDate Created Length:\t"), 0x80);
                PrintHex<uint8_t>(objectinfo_ds.DateCreatedLength, 0x80);
		nStage	++;
	case 35:
		E_Notify(PSTR("\r\nDate Modified:\t"), 0x80);
		arrayParser.Initialize(1, 2, &theBuffer);
		nStage	++;
	case 36:
                objectinfo_ds.DateModifiedLength = *((uint8_t*)&p);
		if (!arrayParser.Parse(&p, &cntdn,
                        (PTP_ARRAY_EL_FUNC)&PrintChar)) {
			return;
                }
                E_Notify(PSTR("\r\nDate Modified Length:\t"), 0x80);
                PrintHex<uint8_t>(objectinfo_ds.DateModifiedLength, 0x80);
		nStage	++;
	case 37:
		E_Notify(PSTR("\r\nKeywords:\t"), 0x80);
		arrayParser.Initialize(1, 2, &theBuffer);
		nStage	++;
	case 38:
                objectinfo_ds.KeywordsLength = *((uint8_t*)&p);
		if (!arrayParser.Parse(&p, &cntdn,
                        (PTP_ARRAY_EL_FUNC)&PrintChar)) {
			return;
                }
                E_Notify(PSTR("\r\nKeywords Length:\t"), 0x80);
		E_Notify(PSTR("\r\n"), 0x80);
		nStage	= 0;
	}
        
        PrintHex<uint32_t>(objectinfo_ds.StorageID,0x80);
        PrintHex<uint16_t>(objectinfo_ds.ObjectFormat,0x80);
        PrintHex<uint16_t>(objectinfo_ds.ProtectionStatus,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ObjectCompressedSize,0x80);
        PrintHex<uint16_t>(objectinfo_ds.ThumbFormat,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ThumbCompressedSize,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ThumbPixWidth,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ThumbPixHeight,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ImagePixWidth,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ImagePixHeight,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ImageBitDepth,0x80);
        PrintHex<uint32_t>(objectinfo_ds.ParentObject,0x80);
        PrintHex<uint16_t>(objectinfo_ds.AssociationType,0x80);
        PrintHex<uint32_t>(objectinfo_ds.AssociationDescription,0x80);
        PrintHex<uint32_t>(objectinfo_ds.SequenceNumber,0x80);
        PrintHex<uint8_t>(objectinfo_ds.FilenameLength,0x80);
        PrintHex<uint8_t>(objectinfo_ds.DateCreatedLength,0x80);
        PrintHex<uint8_t>(objectinfo_ds.DateModifiedLength,0x80);
        PrintHex<uint8_t>(objectinfo_ds.KeywordsLength,0x80);
}

