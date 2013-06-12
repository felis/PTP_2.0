#include "eoseventhandlers.h"
#include <eosvaluetitles.h>

extern EEPROMByteList          vlAperture;
extern EEPROMByteList          vlShutterSpeed;
extern EEPROMByteList          vlWhiteBalance;
extern EEPROMByteList          vlPictureStyle;
extern EEPROMByteList          vlExpCompensation;
extern EEPROMByteList          vlIso;

extern uint8_t  dpMode;
extern uint8_t  dpAperture;
extern uint8_t  dpShutterSpeed;
extern uint8_t  dpWb;
extern uint8_t  dpPStyle;
extern uint8_t  dpIso;
extern uint8_t  dpExpComp;

void EosEventHandlers::OnPropertyChanged(const EOSEvent *evt)
{
    switch (evt->propCode)
    {
    case EOS_DPC_Aperture:
        dpAperture = (uint8_t) evt->propValue;
        E_Notify(PSTR("F:"),0x80);
        E_Notify((char*)FindTitle<VT_APERTURE, VT_APT_TEXT_LEN>(VT_APT_COUNT, ApertureTitles, dpAperture),0x80);
        E_Notify(PSTR("\r\n"),0x80);
        break;
    case EOS_DPC_ShutterSpeed:
        dpShutterSpeed = (uint8_t) evt->propValue;
        E_Notify(PSTR("T:"),0x80);
        E_Notify((char*)FindTitle<VT_SHSPEED, VT_SHSPEED_TEXT_LEN>(VT_SHSPEED_COUNT, ShutterSpeedTitles, dpShutterSpeed),0x80);
        E_Notify(PSTR("\r\n"),0x80);
        break;
    case EOS_DPC_ShootingMode:
        dpMode = (uint8_t) evt->propValue;
        E_Notify(PSTR("Mode:"),0x80);
        E_Notify((char*)FindTitle<VT_MODE, VT_MODE_TEXT_LEN>(VT_MODE_COUNT, ModeTitles, dpMode),0x80);
        E_Notify(PSTR("\r\n"),0x80);
        break;
    case EOS_DPC_WhiteBalance:
        dpWb = (uint8_t) evt->propValue;
        E_Notify(PSTR("WB:"),0x80);
        E_Notify((char*)FindTitle<VT_WB, VT_WB_TEXT_LEN>(VT_WB_COUNT, WbTitles, dpWb),0x80);
        E_Notify(PSTR("\r\n"),0x80);
        break;
    case EOS_DPC_PictureStyle:
        dpPStyle = (uint8_t) evt->propValue;
        E_Notify(PSTR("Pict Style:"),0x80);
        E_Notify((char*)FindTitle<VT_PSTYLE, VT_PSTYLE_TEXT_LEN>(VT_PSTYLE_COUNT, PStyleTitles, dpPStyle),0x80);
        E_Notify(PSTR("\r\n"),0x80);
        break;
    case EOS_DPC_Iso:
        dpIso = (uint8_t) evt->propValue;
        E_Notify(PSTR("ISO:"),0x80);
        E_Notify((char*)FindTitle<VT_ISO, VT_ISO_TEXT_LEN>(VT_ISO_COUNT, IsoTitles, dpIso),0x80);
        E_Notify(PSTR("\r\n"),0x80);
        break;
    case EOS_DPC_ExposureCompensation:
        dpExpComp = (uint8_t) evt->propValue;
        E_Notify(PSTR("Exp Comp:"),0x80);
        E_Notify((char*)FindTitle<VT_EXPCOMP, VT_EXPCOMP_TEXT_LEN>(VT_EXPCOMP_COUNT, ExpCompTitles, dpExpComp),0x80);
        E_Notify(PSTR("\r\n"),0x80);
        break;
    };
}

void EosEventHandlers::OnAcceptedListSize(const EOSEvent *evt, const uint16_t size)
{
    switch (evt->propCode)
    {
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
    case EOS_DPC_ExposureCompensation:
        vlExpCompensation.SetSize((uint8_t)size);
        break;
    };
}

void EosEventHandlers::OnPropertyValuesAccepted(const EOSEvent *evt, const uint16_t index, const uint32_t &val)
{
    switch (evt->propCode)
    {
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
    case EOS_DPC_ExposureCompensation:
        vlExpCompensation.Set(index, (uint8_t)val);
        break;
    case EOS_DPC_Iso:
        vlIso.Set(index, (uint8_t)val);
        break;
    } // switch (evt->propCode)
}


