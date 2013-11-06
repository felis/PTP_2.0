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
#if !defined(__NIKON_H__)
#define __NIKON_H__

#include "ptp.h"

// Nikon Extension Operation Codes
#define NK_OC_GetProfileAllData						0x9006
#define NK_OC_SendProfileData						0x9007
#define NK_OC_DeleteProfile							0x9008
#define NK_OC_SetProfileData						0x9009
#define NK_OC_AdvancedTransfer						0x9010
#define NK_OC_GetFileInfoInBlock					0x9011
#define NK_OC_Capture								0x90C0
#define NK_OC_SetControlMode						0x90C2
#define NK_OC_CurveDownload							0x90C5
#define NK_OC_CurveUpload							0x90C6
#define NK_OC_CheckEvent							0x90C7
#define NK_OC_DeviceReady							0x90C8
#define NK_OC_CaptureInSDRAM						0x90CB
#define NK_OC_GetDevicePTPIPInfo					0x90E0

#define PTP_OC_NIKON_GetPreviewImg					0x9200
#define PTP_OC_NIKON_StartLiveView					0x9201
#define PTP_OC_NIKON_EndLiveView					0x9202
#define PTP_OC_NIKON_GetLiveViewImg					0x9203
#define PTP_OC_NIKON_MfDrive						0x9204
#define PTP_OC_NIKON_ChangeAfArea					0x9205
#define PTP_OC_NIKON_AfDriveCancel					0x9206


// Nikon extention response codes
#define NK_RC_HardwareError							0xA001
#define NK_RC_OutOfFocus							0xA002
#define NK_RC_ChangeCameraModeFailed				0xA003
#define NK_RC_InvalidStatus							0xA004
#define NK_RC_SetPropertyNotSupported				0xA005
#define NK_RC_WbResetError							0xA006
#define NK_RC_DustReferenceError					0xA007
#define NK_RC_ShutterSpeedBulb						0xA008
#define NK_RC_MirrorUpSequence						0xA009
#define NK_RC_CameraModeNotAdjustFNumber			0xA00A
#define NK_RC_NotLiveView							0xA00B
#define NK_RC_MfDriveStepEnd						0xA00C
#define NK_RC_MfDriveStepInsufficiency				0xA00E
#define NK_RC_AdvancedTransferCancel				0xA022

// Nikon extension Event Codes
#define NK_EC_ObjectAddedInSDRAM					0xC101
#define NK_EC_CaptureOverflow						0xC102
#define NK_EC_AdvancedTransfer						0xC103

/* Nikon extension device property codes */
#define NK_DPC_ShootingBank							0xD010
#define NK_DPC_ShootingBankNameA 					0xD011
#define NK_DPC_ShootingBankNameB					0xD012
#define NK_DPC_ShootingBankNameC					0xD013
#define NK_DPC_ShootingBankNameD					0xD014
#define NK_DPC_RawCompression						0xD016
#define NK_DPC_WhiteBalanceAutoBias					0xD017
#define NK_DPC_WhiteBalanceTungstenBias				0xD018
#define NK_DPC_WhiteBalanceFluorescentBias			0xD019
#define NK_DPC_WhiteBalanceDaylightBias				0xD01A
#define NK_DPC_WhiteBalanceFlashBias				0xD01B
#define NK_DPC_WhiteBalanceCloudyBias				0xD01C
#define NK_DPC_WhiteBalanceShadeBias				0xD01D
#define NK_DPC_WhiteBalanceColorTemperature			0xD01E
#define NK_DPC_ImageSharpening						0xD02A
#define NK_DPC_ToneCompensation						0xD02B
#define NK_DPC_ColorModel							0xD02C
#define NK_DPC_HueAdjustment						0xD02D
#define NK_DPC_NonCPULensDataFocalLength			0xD02E
#define NK_DPC_NonCPULensDataMaximumAperture		0xD02F
#define NK_DPC_CSMMenuBankSelect					0xD040
#define NK_DPC_MenuBankNameA						0xD041
#define NK_DPC_MenuBankNameB						0xD042
#define NK_DPC_MenuBankNameC						0xD043
#define NK_DPC_MenuBankNameD						0xD044
#define NK_DPC_A1AFCModePriority					0xD048
#define NK_DPC_A2AFSModePriority					0xD049
#define NK_DPC_A3GroupDynamicAF						0xD04A
#define NK_DPC_A4AFActivation						0xD04B
#define NK_DPC_A5FocusAreaIllumManualFocus			0xD04C
#define NK_DPC_FocusAreaIllumContinuous				0xD04D
#define NK_DPC_FocusAreaIllumWhenSelected 			0xD04E
#define NK_DPC_FocusAreaWrap						0xD04F
#define NK_DPC_A7VerticalAFON						0xD050
#define NK_DPC_ISOAuto								0xD054
#define NK_DPC_B2ISOStep							0xD055
#define NK_DPC_EVStep								0xD056
#define NK_DPC_B4ExposureCompEv						0xD057
#define NK_DPC_ExposureCompensation					0xD058
#define NK_DPC_CenterWeightArea						0xD059
#define NK_DPC_AELockMode							0xD05E
#define NK_DPC_AELAFLMode							0xD05F
#define NK_DPC_MeterOff								0xD062
#define NK_DPC_SelfTimer							0xD063
#define NK_DPC_MonitorOff							0xD064
#define NK_DPC_D1ShootingSpeed						0xD068
#define NK_DPC_D2MaximumShots						0xD069
#define NK_DPC_D3ExpDelayMode						0xD06A
#define NK_DPC_LongExposureNoiseReduction			0xD06B
#define NK_DPC_FileNumberSequence					0xD06C
#define NK_DPC_D6ControlPanelFinderRearControl		0xD06D
#define NK_DPC_ControlPanelFinderViewfinder			0xD06E
#define NK_DPC_D7Illumination						0xD06F
#define NK_DPC_E1FlashSyncSpeed						0xD074
#define NK_DPC_FlashShutterSpeed					0xD075
#define NK_DPC_E3AAFlashMode						0xD076
#define NK_DPC_E4ModelingFlash						0xD077
#define NK_DPC_BracketSet							0xD078
#define NK_DPC_E6ManualModeBracketing				0xD079
#define NK_DPC_BracketOrder							0xD07A
#define NK_DPC_E8AutoBracketSelection				0xD07B
#define NK_DPC_BracketingSet						0xD07C
#define NK_DPC_F1CenterButtonShootingMode			0xD080
#define NK_DPC_CenterButtonPlaybackMode				0xD081
#define NK_DPC_F2Multiselector						0xD082
#define NK_DPC_F3PhotoInfoPlayback					0xD083
#define NK_DPC_F4AssignFuncButton					0xD084
#define NK_DPC_F5CustomizeCommDials					0xD085
#define NK_DPC_ReverseCommandDial					0xD086
#define NK_DPC_ApertureSetting						0xD087
#define NK_DPC_MenusAndPlayback						0xD088
#define NK_DPC_F6ButtonsAndDials					0xD089
#define NK_DPC_NoCFCard								0xD08A
#define NK_DPC_ImageCommentString					0xD090
#define NK_DPC_ImageCommentAttach					0xD091
#define NK_DPC_ImageRotation						0xD092
#define NK_DPC_Bracketing							0xD0C0
#define NK_DPC_ExposureBracketingIntervalDist		0xD0C1
#define NK_DPC_BracketingProgram					0xD0C2
#define NK_DPC_WhiteBalanceBracketStep				0xD0C4
#define NK_DPC_LensID								0xD0E0
#define NK_DPC_FocalLengthMin						0xD0E3
#define NK_DPC_FocalLengthMax						0xD0E4
#define NK_DPC_MaxApAtMinFocalLength				0xD0E5
#define NK_DPC_MaxApAtMaxFocalLength				0xD0E6
#define NK_DPC_ExposureTime							0xD100
#define NK_DPC_ACPower								0xD101
#define NK_DPC_MaximumShots							0xD103
#define NK_DPC_AFLLock								0xD104
#define NK_DPC_AutoExposureLock						0xD105
#define NK_DPC_AutoFocusLock						0xD106
#define NK_DPC_AutofocusLCDTopMode2					0xD107
#define NK_DPC_AutofocusArea						0xD108
#define NK_DPC_LightMeter							0xD10A
#define NK_DPC_CameraOrientation					0xD10E
#define NK_DPC_ExposureApertureLock					0xD111
#define NK_DPC_FlashExposureCompensation			0xD126
#define NK_DPC_OptimizeImage						0xD140
#define NK_DPC_Saturation							0xD142
#define NK_DPC_BeepOff								0xD160
#define NK_DPC_AutofocusMode						0xD161
#define NK_DPC_AFAssist								0xD163
#define NK_DPC_PADVPMode							0xD164
#define NK_DPC_ImageReview							0xD165
#define NK_DPC_AFAreaIllumination					0xD166
#define NK_DPC_FlashMode							0xD167
#define NK_DPC_FlashCommanderMode					0xD168
#define NK_DPC_FlashSign							0xD169
#define NK_DPC_RemoteTimeout						0xD16B
#define NK_DPC_GridDisplay							0xD16C
#define NK_DPC_FlashModeManualPower					0xD16D
#define NK_DPC_FlashModeCommanderPower				0xD16E
#define NK_DPC_CSMMenu								0xD180
#define NK_DPC_BracketingFramesAndSteps				0xD190
#define NK_DPC_LowLight								0xD1B0
#define NK_DPC_FlashOpen							0xD1C0
#define NK_DPC_FlashCharged							0xD1C1

class NikonDSLR : public PTP
{
public:
 	NikonDSLR(USB *pusb, PTPStateHandlers *s);

	virtual uint8_t Init(uint8_t parent, uint8_t port, bool lowspeed);

	uint16_t Capture();
	uint16_t CaptureInSDRAM();

	uint16_t EventCheck(PTPReadParser *parser);
	uint16_t GetLiveViewImage(PTPReadParser *parser);
	uint16_t MoveFocus(uint8_t direction, uint16_t step);
};


#endif // __NIKON_H__