#include "devinfoparser.h"

const char* DevInfoParser::ptpopNames[] PROGMEM = 
{
	msgUndefined,				
	msgGetDeviceInfo,			
	msgOpenSession,			
	msgCloseSession,			
	msgGetStorageIDs,			
	msgGetStorageInfo,		
	msgGetNumObjects,			
	msgGetObjectHandles,		
	msgGetObjectInfo,			
	msgGetObject,				
	msgGetThumb,				
	msgDeleteObject,			
	msgSendObjectInfo,		
	msgSendObject,			
	msgInitiateCapture,		
	msgFormatStore,			
	msgResetDevice,			
	msgSelfTest,				
	msgSetObjectProtection,	
	msgPowerDown,				
	msgGetDevicePropDesc,		
	msgGetDevicePropValue,	
	msgSetDevicePropValue,	
	msgResetDevicePropValue,	
	msgTerminateOpenCapture,	
	msgMoveObject,			
	msgCopyObject,			
	msgGetPartialObject,		
	msgInitiateOpenCapture	
};

const char* DevInfoParser::mtpopNames[] PROGMEM = 
{
	msgUndefined,				
	msgGetObjectPropsSupported,	
	msgGetObjectPropDesc,			
	msgGetObjectPropValue,		
	msgSetObjectPropValue,		
	msgGetObjectPropList,			
	msgSetObjectPropList,			
	msgGetInterdependentPropDesc,	
	msgSendObjectPropList		
};

const char* DevInfoParser::ptpevNames[] PROGMEM = 
{
	msgUndefined,
	msgCancelTransaction,
	msgObjectAdded,
	msgObjectRemoved,
	msgStoreAdded,
	msgStoreRemoved,
	msgDevicePropChanged,
	msgObjectInfoChanged,
	msgDeviceInfoChanged,
	msgRequestObjectTransfer,
	msgStoreFull,
	msgDeviceReset,
	msgStorageInfoChanged,
	msgCaptureComplete,
	msgUnreportedStatus
};

const char* DevInfoParser::mtpevNames[] PROGMEM = 
{
	msgUndefined,
	msgObjectPropChanged,		
	msgObjectPropDescChanged,	
	msgObjectReferencesChanged
};

const char* DevInfoParser::ptpprNames[] PROGMEM = 
{
	msgUndefined,					
	msgBatteryLevel,				
	msgFunctionalMode,			
	msgImageSize,					
	msgCompressionSetting,		
	msgWhiteBalance,				
	msgRGBGain,					
	msgFNumber,					
	msgFocalLength,				
	msgFocusDistance,				
	msgFocusMode,					
	msgExposureMeteringMode,		
	msgFlashMode,					
	msgExposureTime,				
	msgExposureProgramMode,		
	msgExposureIndex,				
	msgExposureBiasCompensation,	
	msgDateTime,					
	msgCaptureDelay,				
	msgStillCaptureMode,			
	msgContrast,					
	msgSharpness,					
	msgDigitalZoom,				
	msgEffectMode,				
	msgBurstNumber,				
	msgBurstInterval,				
	msgTimelapseNumber,			
	msgTimelapseInterval,			
	msgFocusMeteringMode,			
	msgUploadURL,					
	msgArtist,					
	msgCopyrightInfo				
};

const char* DevInfoParser::mtpprNames[] PROGMEM = 
{
	msgUndefined,					
	msgSynchronization_Partner,		
	msgDevice_Friendly_Name,			
	msgVolume,						
	msgSupportedFormatsOrdered,		
	msgDeviceIcon,					
	msgSession_Initiator_Version_Info,
	msgPerceived_Device_Type,			
	msgPlayback_Rate,					
	msgPlayback_Object,				
	msgPlayback_Container			
};

const char* DevInfoParser::acNames[] PROGMEM = 
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

const char* DevInfoParser::imNames[] PROGMEM = 
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

DevInfoParser::DevInfoParser() : 
	nStage(0), 
	strByteCountDown(0), 
	waStage(0),
	waLenCountDown(0),
	waByteCountDown(0),
	fmByteCountDown(0),
	idVendor(0)
{
	for (uint8_t i=0; i<4; i++) waLen[i];
	waWord.word = 0;
	fmBytes[0] = 0;
	fmBytes[1] = 0;
};

bool DevInfoParser::PrintFunctMode(uint8_t **pp, uint16_t &count)
{
	if (!count)
		return true;

	if (!fmByteCountDown)
		fmByteCountDown = 2;

	for (; fmByteCountDown && count; fmByteCountDown--, count--, (*pp)++)
		fmBytes[fmByteCountDown & 1] = (**pp);

	if (fmByteCountDown)
		return false;

	Notify(PSTR("Func.Mode:\t0x"), 0x80);
        
        Serial.println((uint16_t)fmBytes, HEX);
	
      return true;
}

void DevInfoParser::PrintOperation(uint16_t op)
{
	bool bResult = false;

	Serial.print(op, HEX);
	Notify(msgTab, 0x80);

	switch (((op >> 8) & 0xFF))
	{
	case 0x10:
		bResult = PrintPTPOperation(op);
		break;
	case 0x98:
		bResult = PrintMTPOperation(op);
		break;
	default:
		switch (idVendor)
		{
		case PTP_VENDOR_MICROSOFT:
		case PTP_VENDOR_CANON:
			if ((bResult = PrintPSOperation(op)))
				break;
			bResult = PrintEOSOperation(op);
			break;
		default:
			Notify(msgVendorDefined, 0x80);
			bResult = true;
		}
	}
	if (!bResult)
		Notify(msgVendorDefined, 0x80);
	Notify(msgCRLF, 0x80);
}

bool DevInfoParser::PrintPTPOperation(uint16_t op)
{
	if ((op & 0xFF) <= (PTP_OC_InitiateOpenCapture & 0xFF))
	{
		Notify((char*)pgm_read_word(&ptpopNames[(op & 0xFF)]), 0x80);
		return true;
	}
	return false;
}

bool DevInfoParser::PrintMTPOperation(uint16_t op)
{
	if ((op & 0xFF) <= (MTP_OC_SendObjectPropList & 0xFF))
		Notify((char*)pgm_read_word(&mtpopNames[(op & 0xFF)]), 0x80);
	else
	{
		switch (op)
		{
		case MTP_OC_GetObjectReferences:
			Notify(msgGetObjectReferences, 0x80);
			break;
		case MTP_OC_SetObjectReferences:
			Notify(msgSetObjectReferences, 0x80);
			break;
		case MTP_OC_Skip:
			Notify(msgSkip, 0x80);
			break;
		default:
			return false;
		}
	}
	return true;
}

bool DevInfoParser::PrintPSOperation(uint16_t op)
{
	const char* msg;

	switch (op)
	{
	case PTP_OC_PS_GetObjectSize:
		msg = msgPS_GetObjectSize;
		break;
	case PTP_OC_PS_StartShootingMode:
		msg = msgPS_StartShootingMode;
		break;
	case PTP_OC_PS_EndShootingMode:			
		msg = msgPS_EndShootingMode;
		break;
	case PTP_OC_PS_ViewfinderOn:				
		msg = msgPS_ViewfinderOn;
		break;
	case PTP_OC_PS_ViewfinderOff:			
		msg = msgPS_ViewfinderOff;
		break;
	case PTP_OC_PS_ReflectChanges:			
		msg = msgPS_ReflectChanges;
		break;
	case PTP_OC_PS_CheckEvent:				
		msg = msgPS_CheckEvent;
		break;
	case PTP_OC_PS_FocusLock:				
		msg = msgPS_FocusLock;
		break;
	case PTP_OC_PS_FocusUnlock:				
		msg = msgPS_FocusUnlock;
		break;
	case PTP_OC_PS_InitiateCaptureInMemory:	
		msg = msgPS_InitiateCaptureInMemory;
		break;
	case PTP_OC_PS_GetPartialObject:			
		msg = msgPS_GetPartialObject;
		break;
	case PTP_OC_PS_GetViewfinderImage:		
		msg = msgPS_GetViewfinderImage;
		break;
	case PTP_OC_PS_GetChanges:		
		msg = msgPS_GetChanges;
		break;
	case PTP_OC_PS_GetFolderEntries:
		msg = msgPS_GetFolderEntries;
		break;
	default:
		return false;
	}
	Notify(msg, 0x80);
	return true;
}

bool DevInfoParser::PrintEOSOperation(uint16_t op)
{
	const char *msg;
	switch (op)
	{
	case PTP_OC_EOS_GetStorageIDs:
		msg = msgEOS_GetStorageIDs;
		break;
	case PTP_OC_EOS_GetStorageInfo:			
		msg = msgEOS_GetStorageInfo;
		break;
	case PTP_OC_EOS_GetObject:				
		msg = msgEOS_GetObject;
		break;
	case PTP_OC_EOS_GetDeviceInfo:			
		msg = msgEOS_GetDeviceInfo;
		break;
	case PTP_OC_EOS_GetObjectIDs:			
		msg = msgEOS_GetObjectIDs;
		break;
	case PTP_OC_EOS_Capture:					
		msg = msgEOS_Capture;
		break;
	case PTP_OC_EOS_SetDevicePropValue:		
		msg = msgEOS_SetDevicePropValue;
		break;
	case PTP_OC_EOS_SetPCConnectMode:		
		msg = msgEOS_SetPCConnectMode;
		break;
	case PTP_OC_EOS_SetExtendedEventInfo:	
		msg = msgEOS_SetExtendedEventInfo;
		break;
	case PTP_OC_EOS_GetEvent:				
		msg = msgEOS_GetEvent;
		break;
	case PTP_OC_EOS_GetLiveViewPicture:		
		msg = msgEOS_GetLiveViewPicture;
		break;
	case PTP_OC_EOS_MoveFocus:
		msg = msgEOS_MoveFocus;
		break;
	default:
		return false;
	}
	Notify(msg, 0x80);
	return true;
}

void DevInfoParser::PrintEvent(uint16_t op)
{
	Serial.print(op, HEX);
	Notify(msgTab, 0x80);

	if ((((op >> 8) & 0xFF) == 0x40) && ((op & 0xFF) <= (PTP_EC_UnreportedStatus & 0xFF)))
		Notify((char*)pgm_read_word(&ptpevNames[(op & 0xFF)]), 0x80);
	else
		if ((((op >> 8) & 0xFF) == 0xC8) && ((op & 0xFF) <= (MTP_EC_ObjectReferencesChanged & 0xFF)))
			Notify((char*)pgm_read_word(&mtpevNames[(op & 0xFF)]), 0x80);
		else
			Notify(msgVendorDefined, 0x80);
	Notify(msgCRLF, 0x80);
}

void DevInfoParser::PrintDevProp(uint16_t op)
{
	Serial.print(op, HEX);
	Notify(msgTab, 0x80);

	if ((((op >> 8) & 0xFF) == 0x50) && ((op & 0xFF) <= (PTP_DPC_CopyrightInfo & 0xFF)))
		Notify((char*)pgm_read_word(&ptpprNames[(op & 0xFF)]), 0x80);
	else
		if (((op >> 8) & 0xFF) == 0xD4) 
		{
			if ( (op & 0xFF) <= (MTP_DPC_Perceived_Device_Type & 0xFF) )
				Notify((char*)pgm_read_word(&mtpprNames[(op & 0xFF)]), 0x80);
			else
			{
				switch (op)
				{
				case MTP_DPC_Playback_Rate:
					Notify(msgPlayback_Rate, 0x80);
					break;
				case MTP_DPC_Playback_Object:
					Notify(msgPlayback_Object, 0x80);
					break;
				case MTP_DPC_Playback_Container:
					Notify(msgPlayback_Container, 0x80);
					break;
				default:
					Notify(msgVendorDefined, 0x80);
				}
			}
		}
		else
			Notify(msgVendorDefined, 0x80);

	Notify(msgCRLF, 0x80);
}

void DevInfoParser::PrintFormat(uint16_t op)
{
	Serial.print(op, HEX);
	Notify(msgTab, 0x80);

	if ((((op >> 8) & 0xFF) == 0x30) && ((op & 0xFF) <= (PTP_OFC_QT & 0xFF)))
		Notify((char*)pgm_read_word(&acNames[(op & 0xFF)]), 0x80);
	else
		if ((((op >> 8) & 0xFF) == 0x38) && ((op & 0xFF) <= (PTP_OFC_JPX & 0xFF)))
			Notify((char*)pgm_read_word(&imNames[(op & 0xFF)]), 0x80);
		else
		{
			switch (op)
			{
			case MTP_OFC_Undefined_Firmware:
				Notify(msgUndefined_Firmware, 0x80);
				break;
			case MTP_OFC_Windows_Image_Format:	
				Notify(msgWindows_Image_Format, 0x80);
				break;
			case MTP_OFC_Undefined_Audio:			
				Notify(msgUndefined_Audio, 0x80);
				break;
			case MTP_OFC_WMA:						
				Notify(msgWMA, 0x80);
				break;
			case MTP_OFC_OGG:						
				Notify(msgOGG, 0x80);
				break;
			case MTP_OFC_AAC:						
				Notify(msgAAC, 0x80);
				break;
			case MTP_OFC_Audible:					
				Notify(msgAudible, 0x80);
				break;
			case MTP_OFC_FLAC:					
				Notify(msgFLAC, 0x80);
				break;
			case MTP_OFC_Undefined_Video:			
				Notify(msgUndefined_Video, 0x80);
				break;
			case MTP_OFC_WMV:						
				Notify(msgWMV, 0x80);
				break;
			case MTP_OFC_MP4_Container:			
				Notify(msgMP4_Container, 0x80);
				break;
			case MTP_OFC_MP2:						
				Notify(msgMP2, 0x80);
				break;
			case MTP_OFC_3GP_Container:
				Notify(msg3GP_Container, 0x80);
				break;
			default:
				Notify(msgVendorDefined, 0x80);
			}
		}
	Notify(msgCRLF, 0x80);
}

bool DevInfoParser::PrintVendor(uint8_t **pp, uint16_t &count)
{
	Notify(PSTR("Vendor Ext. ID:\t0x"), 0x80);
	Serial.print(*((uint32_t*)*pp),HEX);

	switch (*((uint32_t*)*pp))
	{
	case PTP_VENDOR_EASTMAN_KODAK:
		Notify(PSTR("(Eastman Kodak)"), 0x80);
		break;
	case PTP_VENDOR_SEIKO_EPSON:		
		Notify(PSTR("(Seiko Epson)"), 0x80);
		break;
	case PTP_VENDOR_AGILENT:		
		Notify(PSTR("(Agilent)"), 0x80);
		break;
	case PTP_VENDOR_POLAROID:			
		Notify(PSTR("(Polaroid)"), 0x80);
		break;
	case PTP_VENDOR_AGFA_GEVAERT:			
		Notify(PSTR("(AGFA)"), 0x80);
		break;
	case PTP_VENDOR_MICROSOFT:		
		Notify(PSTR("(Microsoft)"), 0x80);
		break;
	case PTP_VENDOR_EQUINOX:		
		Notify(PSTR("(Equinox)"), 0x80);
		break;
	case PTP_VENDOR_VIEWQUEST:		
		Notify(PSTR("(ViewQuest)"), 0x80);
		break;
	case PTP_VENDOR_STMICROELECTRONICS:
		Notify(PSTR("(StMicroelectronics)"), 0x80);
		break;
	case PTP_VENDOR_NIKON:
		Notify(PSTR("(Nikon)"), 0x80);
		break;
	case PTP_VENDOR_CANON:
		Notify(PSTR("(Canon)"), 0x80);
		break;
	case PTP_VENDOR_FOTONATION:
		Notify(PSTR("(FotoNation)"), 0x80);
		break;
	case PTP_VENDOR_PENTAX:
		Notify(PSTR("(Pentax)"), 0x80);
		break;
	case PTP_VENDOR_FUJI:
		Notify(PSTR("(Fuji)"), 0x80);
		break;
	default:
		Notify(PSTR("(Unknown)"), 0x80);
	}
	Notify(msgCRLF, 0x80);
	*pp += 4; count -= 4;
	return true;
}

bool DevInfoParser::PrintString(uint8_t **pp, uint16_t &count)
{
	if (!count)
		return true;

	if ( !strByteCountDown )
	{
		if ( !(**pp) )
		{
			(*pp) ++;
			count --;
			return true;
		}
		strByteCountDown = ((**pp) << 1);
		(*pp) ++;
		count --;
	}

	for (; strByteCountDown && count; strByteCountDown--, count--, (*pp)++)
	{
		if ( !(strByteCountDown & 1) && ((**pp) > 0))
			Serial.write((unsigned char)(**pp));
	}
	return (strByteCountDown == 0);
}

bool DevInfoParser::PrintWordArray(uint8_t **pp, uint16_t &count, PRINTFUNC pf = NULL)
{
	switch (waStage)
	{
	case 0:
		if (!waLenCountDown)
			waLenCountDown = 4;

		for (; waLenCountDown && count; waLenCountDown--, count--, (*pp)++)
			waLen[4-waLenCountDown] = (**pp);

		if (waLenCountDown)
			return false;
		
		waStage ++;

	case 1:
		for (waByteCountDown = (waByteCountDown) ? waByteCountDown : ((*((uint32_t*)waLen) << 1)); 
			 waByteCountDown && count; waByteCountDown--, count--, (*pp)++)
		{
			if (waByteCountDown & 1)
			{
				waWord.bytes[1] = (**pp);

				if ( pf )
					(this->*pf)(waWord.word);
				else
					Serial.println (waWord.word, HEX);
			}
			else
				waWord.bytes[0] = (**pp);
		}
		if (waByteCountDown)
			return false;
	}
	waStage = 0;
	return true;
}

void DevInfoParser::Parse(const uint16_t len, const uint8_t *pbuf, const uint32_t &offset)
{
	uint16_t	count	= (uint16_t)len;
	uint8_t		*p		= (uint8_t*)pbuf;

	switch (nStage)
	{
	case 0:
		// Skip PTP container header
		p += 12;	count -=12;
		nStage ++;
	case 1:
		Notify(PSTR("\r\nStd.Ver.:\t0x"), 0x80);			Serial.println(*(uint16_t*)p, DEC);	
		p += 2;		count -= 2;

		idVendor = *((uint32_t*)p);
		PrintVendor(&p, count);

		Notify(PSTR("\r\nVend.Ext.Ver.:\t0x"), 0x80);
                Serial.println(*((uint16_t*)p), HEX);
		p += 2;		count -=2;

		nStage ++;
	case 2:
		// Vendor extension description
		//if (*p)
		{
			if (!PrintString(&p, count))
				return;

			Notify(msgCRLF, 0x80);
		}
		nStage ++;
	case 3:
		// Functional mode
		if (!PrintFunctMode(&p, count))
			return;
		nStage ++;
	case 4:
		// Operations Supported
		Notify(PSTR("\r\nOperations supported:\r\n"), 0x80);
		nStage++;
	case 5:
		if (!PrintWordArray(&p, count, &DevInfoParser::PrintOperation))
			return;
		nStage ++;
	case 6:
		// Events Supported
		Notify(PSTR("\r\nEvents supported:\r\n"), 0x80);
		nStage ++;
	case 7:
		if (!PrintWordArray(&p, count, &DevInfoParser::PrintEvent))
			return;
		nStage ++;
	case 8:
		// Device Properties Supported
		Notify(PSTR("\r\nDevice properties supported:\r\n"), 0x80);
		nStage ++;
	case 9:
		if (!PrintWordArray(&p, count, &DevInfoParser::PrintDevProp))
			return;
		nStage ++;
	case 10:
		// Capture formats
		Notify(PSTR("\r\nCapture formats:\r\n"), 0x80);
		nStage ++;
	case 11:
		if (!PrintWordArray(&p, count, &DevInfoParser::PrintFormat))
			return;
		nStage ++;
	case 12:
		// Image Formats
		Notify(PSTR("\r\nImage Formats:\r\n"), 0x80);
		nStage ++;
	case 13:
		if (!PrintWordArray(&p, count, &DevInfoParser::PrintFormat))
			return;
		nStage ++;
	case 14:
		// Manufacturer
		Notify(PSTR("\r\nManufacturer:\t"), 0x80);
		nStage ++;
	case 15:
		if (!PrintString(&p, count))
			return;
		nStage ++;
	case 16:
		// Model
		Notify(PSTR("\r\nModel:\t\t"), 0x80);
		nStage ++;
	case 17:
		if (!PrintString(&p, count))
			return;
		nStage ++;
	case 18:
		// Device version
		Notify(PSTR("\r\nDevice ver.:\t"), 0x80);
		nStage ++;
	case 19:
		if (!PrintString(&p, count))
			return;
		nStage ++;
	case 20:
		// Serial number
		Notify(PSTR("\r\nSerial num.:\t"), 0x80);
		nStage ++;
	case 21:
		if (!PrintString(&p, count))
			return;
		Notify(PSTR("\r\n\r\n"), 0x80);
		//nStage = 0;
	}
}
