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

const char* ptpopNames[] PROGMEM = 
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

const char* mtpopNames[] PROGMEM = 
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

const char* ptpevNames[] PROGMEM = 
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

const char* mtpevNames[] PROGMEM = 
{
	msgUndefined,
	msgObjectPropChanged,		
	msgObjectPropDescChanged,	
	msgObjectReferencesChanged
};

const char* ptpprNames[] PROGMEM = 
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

const char* mtpprNames[] PROGMEM = 
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

const char* acNames[] PROGMEM = 
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

const char* imNames[] PROGMEM = 
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
		Notify((char*)pgm_read_word(&ptpopNames[(op & 0xFF)]));
		return true;
	}
	return false;
}

bool PrintMTPOperation(uint16_t op)
{
	if ((op & 0xFF) <= (MTP_OC_SendObjectPropList & 0xFF))
		Notify((char*)pgm_read_word(&mtpopNames[(op & 0xFF)]));
	else
	{
		switch (op)
		{
		case MTP_OC_GetObjectReferences:
			Notify(msgGetObjectReferences);
			break;
		case MTP_OC_SetObjectReferences:
			Notify(msgSetObjectReferences);
			break;
		case MTP_OC_Skip:
			Notify(msgSkip);
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
		Notify((char*)pgm_read_word(&ptpevNames[(op & 0xFF)]));
	else
		if ((((op >> 8) & 0xFF) == 0xC8) && ((op & 0xFF) <= (MTP_EC_ObjectReferencesChanged & 0xFF)))
			Notify((char*)pgm_read_word(&mtpevNames[(op & 0xFF)]));
		else
			Notify(msgVendorDefined);
}

void PrintDevProp(uint16_t op)
{
	if ((((op >> 8) & 0xFF) == 0x50) && ((op & 0xFF) <= (PTP_DPC_CopyrightInfo & 0xFF)))
		Notify((char*)pgm_read_word(&ptpprNames[(op & 0xFF)]));
	else
		if (((op >> 8) & 0xFF) == 0xD4) 
		{
			if ( (op & 0xFF) <= (MTP_DPC_Perceived_Device_Type & 0xFF) )
				Notify((char*)pgm_read_word(&mtpprNames[(op & 0xFF)]));
			else
			{
				switch (op)
				{
				case MTP_DPC_Playback_Rate:
					Notify(msgPlayback_Rate);
					break;
				case MTP_DPC_Playback_Object:
					Notify(msgPlayback_Object);
					break;
				case MTP_DPC_Playback_Container:
					Notify(msgPlayback_Container);
					break;
				default:
					Notify(msgVendorDefined);
				}
			}
		}
		else
			Notify(msgVendorDefined);
}

void PrintFormat(uint16_t op)
{
	if ((((op >> 8) & 0xFF) == 0x30) && ((op & 0xFF) <= (PTP_OFC_QT & 0xFF)))
		Notify((char*)pgm_read_word(&acNames[(op & 0xFF)]));
	else
		if ((((op >> 8) & 0xFF) == 0x38) && ((op & 0xFF) <= (PTP_OFC_JPX & 0xFF)))
			Notify((char*)pgm_read_word(&imNames[(op & 0xFF)]));
		else
		{
			switch (op)
			{
			case MTP_OFC_Undefined_Firmware:
				Notify(msgUndefined_Firmware);
				break;
			case MTP_OFC_Windows_Image_Format:	
				Notify(msgWindows_Image_Format);
				break;
			case MTP_OFC_Undefined_Audio:			
				Notify(msgUndefined_Audio);
				break;
			case MTP_OFC_WMA:						
				Notify(msgWMA);
				break;
			case MTP_OFC_OGG:						
				Notify(msgOGG);
				break;
			case MTP_OFC_AAC:						
				Notify(msgAAC);
				break;
			case MTP_OFC_Audible:					
				Notify(msgAudible);
				break;
			case MTP_OFC_FLAC:					
				Notify(msgFLAC);
				break;
			case MTP_OFC_Undefined_Video:			
				Notify(msgUndefined_Video);
				break;
			case MTP_OFC_WMV:						
				Notify(msgWMV);
				break;
			case MTP_OFC_MP4_Container:			
				Notify(msgMP4_Container);
				break;
			case MTP_OFC_MP2:						
				Notify(msgMP2);
				break;
			case MTP_OFC_3GP_Container:
				Notify(msg3GP_Container);
				break;
			default:
				Notify(msgVendorDefined);
			}
		}
}


