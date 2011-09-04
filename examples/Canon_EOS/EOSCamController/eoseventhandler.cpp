#include "eoseventhandler.h"
#include "dataitem.h"
#include "camcontroller.h"

#ifndef NO_SETTINGS_SCREEN
extern DIT_MODE          diMode;
extern DIT_APERTURE      diAperture;
extern DIT_WB            diWb;
extern DIT_SHUTTER_SPEED diShutterSpeed;
extern DIT_PSTYLE        diPStyle;
extern DIT_ISO           diIso;
#endif

extern DIT_EXPCOMP       diExpComp;

#ifndef NO_SETTINGS_SCREEN
extern EEPROMByteList    vlAperture;
extern EEPROMByteList    vlShutterSpeed;
extern EEPROMByteList    vlWhiteBalance;
extern EEPROMByteList    vlPictureStyle;
extern EEPROMByteList    vlIso;
#endif

extern EEPROMByteList    vlExpCompensation;

void EosEventHandlers::OnPropertyChanged(const EOSEvent *evt)
{
    switch (evt->propCode)
    {
#ifndef NO_SETTINGS_SCREEN
    case EOS_DPC_Aperture:
        diAperture.Set(evt->propValue);
        break;
    case EOS_DPC_ShutterSpeed:
        diShutterSpeed.Set(evt->propValue);
        break;
    case EOS_DPC_ShootingMode:
        diMode.Set(evt->propValue);
        break;
    case EOS_DPC_WhiteBalance:
        diWb.Set(evt->propValue);
        break;
    case EOS_DPC_PictureStyle:
        diPStyle.Set(evt->propValue);
        break;
    case EOS_DPC_Iso:
        diIso.Set(evt->propValue);
        break;
#endif                                            
    case EOS_DPC_ExposureCompensation:
        diExpComp.Set(evt->propValue);
        break;
    };
}

void EosEventHandlers::OnAcceptedListSize(const EOSEvent *evt, const uint16_t size)
{
    switch (evt->propCode)
    {
    #ifndef NO_SETTINGS_SCREEN
    case EOS_DPC_Aperture:
    	vlAperture.SetSize((uint8_t)size);
    	break;
    case EOS_DPC_ShutterSpeed:
        vlShutterSpeed.SetSize((uint8_t)size);
        break;
    case EOS_DPC_WhiteBalance:
        vlWhiteBalance.SetSize((uint8_t)size);
        break;
    case EOS_DPC_PictureStyle:
        vlPictureStyle.SetSize((uint8_t)size);
        break;
    case EOS_DPC_Iso:
        vlIso.SetSize((uint8_t)size);
        break;
    #endif                                                
    case EOS_DPC_ExposureCompensation:
        vlExpCompensation.SetSize((uint8_t)size);
        break;
    };
}

void EosEventHandlers::OnPropertyValuesAccepted(const EOSEvent *evt, const uint16_t index, const uint32_t &val)
{
    switch (evt->propCode)
    {
#ifndef NO_SETTINGS_SCREEN
    case EOS_DPC_Aperture:
        vlAperture.Set(index, (uint8_t)val);
        break;
    case EOS_DPC_ShutterSpeed:
        vlShutterSpeed.Set(index, (uint8_t)val);
        break;
    case EOS_DPC_WhiteBalance:
        vlWhiteBalance.Set(index, (uint8_t)val);
        break;
    case EOS_DPC_PictureStyle:
        vlPictureStyle.Set(index, (uint8_t)val);
        break;
    case EOS_DPC_Iso:
        vlIso.Set(index, (uint8_t)val);
        break;
#endif                                                
    case EOS_DPC_ExposureCompensation:
        vlExpCompensation.Set(index, (uint8_t)val);
        break;
    } // switch (evt->propCode)
}


