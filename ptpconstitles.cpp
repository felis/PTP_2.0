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

#include "ptpconstitles.h"

const char* const ptpopNames[] PROGMEM = 
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

const char* const mtpopNames[] PROGMEM = 
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

const char* const ptpevNames[] PROGMEM = 
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

const char* const mtpevNames[] PROGMEM = 
{
	msgUndefined,
	msgObjectPropChanged,		
	msgObjectPropDescChanged,	
	msgObjectReferencesChanged
};

const char* const ptpprNames[] PROGMEM = 
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

const char* const mtpprNames[] PROGMEM = 
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

const char* const acNames[] PROGMEM = 
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

const char* const imNames[] PROGMEM = 
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

bool PrintPTPOperation(uint16_t op)
{
	if ((op & 0xFF) <= (PTP_OC_InitiateOpenCapture & 0xFF))
	{
		E_Notify((char*)pgm_read_word(&ptpopNames[(op & 0xFF)]), 0x80);
		return true;
	}
	return false;
}

bool PrintMTPOperation(uint16_t op)
{
	if ((op & 0xFF) <= (MTP_OC_SendObjectPropList & 0xFF))
		E_Notify((char*)pgm_read_word(&mtpopNames[(op & 0xFF)]), 0x80);
	else
	{
		switch (op)
		{
		case MTP_OC_GetObjectReferences:
			E_Notify(msgGetObjectReferences, 0x80);
			break;
		case MTP_OC_SetObjectReferences:
			E_Notify(msgSetObjectReferences, 0x80);
			break;
		case MTP_OC_Skip:
			E_Notify(msgSkip, 0x80);
			break;
		default:
			return false;
		}
	}
	return true;
}

void PrintEvent(uint16_t op)
{
	if ((((op >> 8) & 0xFF) == 0x40) && ((op & 0xFF) <= (PTP_EC_UnreportedStatus & 0xFF)))
		E_Notify((char*)pgm_read_word(&ptpevNames[(op & 0xFF)]), 0x80);
	else
		if ((((op >> 8) & 0xFF) == 0xC8) && ((op & 0xFF) <= (MTP_EC_ObjectReferencesChanged & 0xFF)))
			E_Notify((char*)pgm_read_word(&mtpevNames[(op & 0xFF)]), 0x80);
		else
			E_Notify(msgVendorDefined, 0x80);
}

void PrintDevProp(uint16_t op)
{
	if ((((op >> 8) & 0xFF) == 0x50) && ((op & 0xFF) <= (PTP_DPC_CopyrightInfo & 0xFF)))
		E_Notify((char*)pgm_read_word(&ptpprNames[(op & 0xFF)]), 0x80);
	else
		if (((op >> 8) & 0xFF) == 0xD4) 
		{
			if ( (op & 0xFF) <= (MTP_DPC_Perceived_Device_Type & 0xFF) )
				E_Notify((char*)pgm_read_word(&mtpprNames[(op & 0xFF)]), 0x80);
			else
			{
				switch (op)
				{
				case MTP_DPC_Playback_Rate:
					E_Notify(msgPlayback_Rate, 0x80);
					break;
				case MTP_DPC_Playback_Object:
					E_Notify(msgPlayback_Object, 0x80);
					break;
				case MTP_DPC_Playback_Container:
					E_Notify(msgPlayback_Container, 0x80);
					break;
				default:
					E_Notify(msgVendorDefined, 0x80);
				}
			}
		}
		else
			E_Notify(msgVendorDefined, 0x80);
}

void PrintFormat(uint16_t op)
{
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
				E_Notify(msgVendorDefined, 0x80);
			}
		}
}


