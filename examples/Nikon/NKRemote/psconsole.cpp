#include "psconsole.h"
#include <nkvaluetitles.h>
#include <valuelist.h>

extern NikonDSLR                Nk;

/* fixes avr-gcc incompatibility with virtual destructors */
//void operator delete( void *p ) {} 

const char* menuMain[] = {"Capture", "View Settings", "Change Settings", "Viewfinder On", "Viewfinder Off"};
const char* menuChangeSettings[] = {"Mode", "Aperture", "Shutter Speed", "WB", "ISO", "Exp Comp", "Compr", "FocusMode", "FocusPoint"};
const char* menuUpDown[] = {"<<", ">>"};

#define MAIN_MENU_COUNT sizeof(menuMain) / sizeof(const char*)
#define CHSET_MENU_COUNT sizeof(menuChangeSettings) / sizeof(const char*)
#define UPDWN_MENU_COUNT sizeof(menuUpDown) / sizeof(const char*)

void PrintAperture()
{
    E_Notify(PSTR("F:"),0x80);
    PrintValueTitle<uint16_t, VT_APERTURE, VT_APT_COUNT, VT_APT_TEXT_LEN>((PTP*)&Nk, PTP_DPC_FNumber, ApertureTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

void PrintShutterSpeed()
{
    E_Notify(PSTR("T:"),0x80);
    PrintValueTitle<VT_SHSPEED, VT_SHSPEED, VT_SHSPEED_COUNT, VT_SHSPEED_TEXT_LEN>((PTP*)&Nk, PTP_DPC_ExposureTime, ShutterSpeedTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

void PrintWB()
{
    E_Notify(PSTR("WB:"),0x80);
    PrintValueTitle<VT_WB, VT_WB, VT_WB_COUNT, VT_WB_TEXT_LEN>((PTP*)&Nk, PTP_DPC_WhiteBalance, WbTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

void PrintIso()
{
    E_Notify(PSTR("ISO:"),0x80);
    PrintValueTitle<VT_ISO, VT_ISO, VT_ISO_COUNT, VT_ISO_TEXT_LEN>((PTP*)&Nk, PTP_DPC_ExposureIndex, IsoTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

void PrintExpCompensation()
{
    E_Notify(PSTR("ExpComp:"),0x80);
    PrintValueTitle<VT_EXPCOMP, VT_EXPCOMP, VT_EXPCOMP_COUNT, VT_EXPCOMP_TEXT_LEN>((PTP*)&Nk, PTP_DPC_ExposureBiasCompensation, ExpCompTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

void PrintCompression()
{
    E_Notify(PSTR("Compression:"),0x80);
    PrintValueTitle<VT_COMPRESSION, VT_COMPRESSION, VT_COMPRESSION_COUNT, VT_COMPRESSION_TEXT_LEN>((PTP*)&Nk, PTP_DPC_CompressionSetting, CompressionTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

void PrintFocusMode()
{
    E_Notify(PSTR("FocusMode:"),0x80);
    PrintValueTitle<VT_FOCUSMODE, VT_FOCUSMODE, VT_FOCUSMODE_COUNT, VT_FOCUSMODE_TEXT_LEN>((PTP*)&Nk, NK_DPC_AutofocusMode, FocusModeTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

void PrintFocusArea()
{
    E_Notify(PSTR("FocusPoint:"),0x80);
    PrintValueTitle<VT_FOCUSAREA, VT_FOCUSAREA, VT_FOCUSAREA_COUNT, VT_FOCUSAREA_TEXT_LEN>((PTP*)&Nk, NK_DPC_AutofocusArea, FocusAreaTitles);
    E_Notify(PSTR("\r\n"),0x80);
}

//void PrintZoom()
//{
//    uint16_t    val = 0;
//    E_Notify(PSTR("Zoom:"),0x80);
//    
//    if (Nk.GetDevicePropValue(PS_DPC_Zoom, (uint16_t&)val) == PTP_RC_OK)
//        PrintHex<uint16_t>(val,0x80); 
//        
//    E_Notify(PSTR("\r\n"),0x80);
//}

void PSConsole::ShowParams()
{
    PrintAperture();
    PrintShutterSpeed();
    PrintWB();
    PrintIso();
    PrintExpCompensation();
//    PrintCamOutput();
    PrintCompression();
    PrintFocusMode();
    PrintFocusArea();
}

QState PSConsole::Initial(PSConsole *me, QEvent const *e) 
{
    return Q_TRAN(&PSConsole::Inactive);
}

QState PSConsole::Inactive(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG: 
            E_Notify(PSTR("Inactive\r\n"),0x80);
            return Q_HANDLED();
        case TICK_SIG: 
            return Q_TRAN(&PSConsole::Active);
    }
    return Q_SUPER(QHsm::top);
}

QState PSConsole::Active(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG: 
            E_Notify(PSTR("Active\r\n"),0x80);
            return Q_HANDLED();
        case Q_INIT_SIG: 
            return Q_TRAN(&PSConsole::MainMenu);
        case TICK_SIG:
            return Q_TRAN(&PSConsole::Inactive);
    }
    return Q_SUPER(QHsm::top);
}

void PSConsole::PrintMenuTitles(uint8_t count, const char **menu)
{
    Serial.println("");
    for (uint8_t i=0; i<=count; i++)
    {
        Serial.print(i, DEC);
        Serial.print(". ");
        
        if (i == 0)
            Serial.println("<..>");
        else
            Serial.println(menu[i-1]);
    }
    Serial.println("");
}

QState PSConsole::MainMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(MAIN_MENU_COUNT, menuMain);
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            switch (((MenuSelectEvt*)e)->item_index)
            {
                case 0:
                    PrintMenuTitles(MAIN_MENU_COUNT, menuMain);
                    return Q_HANDLED();
                case 1:
                    Nk.InitiateCapture();
                    return Q_HANDLED();
                case 2:
                    ShowParams();
                    PrintMenuTitles(MAIN_MENU_COUNT, menuMain);
                    return Q_HANDLED();
                case 3:
                    return Q_TRAN(&PSConsole::ChangeSettingsMenu);
                case 4:
//                    Nk.Operation(PS_OC_ViewfinderOn);
                    return Q_HANDLED();
                case 5:
//                    Nk.Operation(PS_OC_ViewfinderOff);
                    return Q_HANDLED();
            }
        }
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeSettingsMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(CHSET_MENU_COUNT, menuChangeSettings);
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            switch (((MenuSelectEvt*)e)->item_index)
            {
                case 0:
                    return Q_TRAN(&PSConsole::MainMenu);
//                case 1:  // Aperture
//                      return Q_TRAN(&PSConsole::ChangeModeMenu);
                case 2:  // Aperture
                      return Q_TRAN(&PSConsole::ChangeApertureMenu);
                case 3:  // Shutter Speed
                      return Q_TRAN(&PSConsole::ChangeShutterSpeedMenu);
                case 4:  // White Balance
                      return Q_TRAN(&PSConsole::ChangeWBMenu);
                case 5:  // ISO
                      return Q_TRAN(&PSConsole::ChangeIsoMenu);
                case 6:  // Exposure Compensation
                      return Q_TRAN(&PSConsole::ChangeExpCompMenu);
                case 7:  // CompressionSetting
                      return Q_TRAN(&PSConsole::ChangeCompressionMenu);
                case 8:  // FocusMode
                      return Q_TRAN(&PSConsole::ChangeFocusModeMenu);
                case 9:  // FocusMode
                      return Q_TRAN(&PSConsole::ChangeFocusAreaMenu);
            } // switch
        }
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeApertureMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintAperture();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            uint8_t new_value;
            
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            case 2:
                StepUp<uint16_t>((PTP*)&Nk, PTP_DPC_FNumber);
                PrintAperture();
                return Q_HANDLED();
            case 1:
                StepDown<uint16_t>((PTP*)&Nk, PTP_DPC_FNumber);
                PrintAperture();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeShutterSpeedMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintShutterSpeed();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            uint8_t new_value;
            
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            case 2:
                StepUp<VT_SHSPEED>((PTP*)&Nk, PTP_DPC_ExposureTime);
                PrintShutterSpeed();
                return Q_HANDLED();
            case 1:
                StepDown<VT_SHSPEED>((PTP*)&Nk, PTP_DPC_ExposureTime);
                PrintShutterSpeed();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeWBMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintWB();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            uint8_t new_value;
            
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            
            case 2:
                StepUp<VT_WB>((PTP*)&Nk, PTP_DPC_WhiteBalance);
                PrintWB();
                return Q_HANDLED();
            case 1:
                StepDown<VT_WB>((PTP*)&Nk, PTP_DPC_WhiteBalance);
                PrintWB();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeIsoMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintIso();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            uint8_t new_value;
            
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            case 2:
                StepUp<VT_ISO>((PTP*)&Nk, PTP_DPC_ExposureIndex);
                PrintIso();
                return Q_HANDLED();
            case 1:
                StepDown<VT_ISO>((PTP*)&Nk, PTP_DPC_ExposureIndex);
                PrintIso();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeExpCompMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintExpCompensation();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            uint8_t new_value;
            
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            case 2:
                StepUp<VT_EXPCOMP>((PTP*)&Nk, PTP_DPC_ExposureBiasCompensation);
                PrintExpCompensation();
                return Q_HANDLED();
            case 1:
                StepDown<VT_EXPCOMP>((PTP*)&Nk, PTP_DPC_ExposureBiasCompensation);
                PrintExpCompensation();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

//QState PSConsole::ChangeCamOutputMenu(PSConsole *me, QEvent const *e) 
//{
//    switch (e->sig) 
//    {
//        case Q_ENTRY_SIG:
//            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
//            PrintCamOutput();
//            return Q_HANDLED();
//        case MENU_SELECT_SIG: 
//        {
//            switch (((MenuSelectEvt*)e)->item_index)
//            {
//            case 0:
//                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
//            case 2:
//                StepUp<uint8_t>((PTP*)&Nk, PS_DPC_CameraOutput);
//                PrintCamOutput();
//                return Q_HANDLED();
//            case 1:
//                StepDown<uint8_t>((PTP*)&Nk, PS_DPC_CameraOutput);
//                PrintCamOutput();
//                return Q_HANDLED();
//            } // switch (((MenuSelectEvt*)e)->item_index)
//        } // case MENU_SELECT_SIG:
//    }
//    return Q_SUPER(&PSConsole::Active);
//}

QState PSConsole::ChangeCompressionMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintCompression();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            case 1:
                StepUp<VT_COMPRESSION>((PTP*)&Nk, PTP_DPC_CompressionSetting);
                PrintCompression();
                return Q_HANDLED();
            case 2:
                StepDown<VT_COMPRESSION>((PTP*)&Nk, PTP_DPC_CompressionSetting);
                PrintCompression();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeFocusModeMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintFocusMode();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            case 2:
                StepUp<VT_FOCUSMODE>((PTP*)&Nk, NK_DPC_AutofocusMode);
                PrintFocusMode();
                return Q_HANDLED();
            case 1:
                StepDown<VT_FOCUSMODE>((PTP*)&Nk, NK_DPC_AutofocusMode);
                PrintFocusMode();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

QState PSConsole::ChangeFocusAreaMenu(PSConsole *me, QEvent const *e) 
{
    switch (e->sig) 
    {
        case Q_ENTRY_SIG:
            PrintMenuTitles(UPDWN_MENU_COUNT, menuUpDown);
            PrintFocusArea();
            return Q_HANDLED();
        case MENU_SELECT_SIG: 
        {
            switch (((MenuSelectEvt*)e)->item_index)
            {
            case 0:
                return Q_TRAN(&PSConsole::ChangeSettingsMenu);
            case 2:
                StepUp<VT_FOCUSAREA>((PTP*)&Nk, NK_DPC_AutofocusArea);
                PrintFocusArea();
                return Q_HANDLED();
            case 1:
                StepDown<VT_FOCUSAREA>((PTP*)&Nk, NK_DPC_AutofocusArea);
                PrintFocusArea();
                return Q_HANDLED();
            } // switch (((MenuSelectEvt*)e)->item_index)
        } // case MENU_SELECT_SIG:
    }
    return Q_SUPER(&PSConsole::Active);
}

static TickEvt     tick_evt;

int8_t PSConsole::MenuSelect()
{
    if( !Serial.available()) 
        return -1;
      
    uint8_t  char_count = 0;
    uint8_t  index = 0;
          
    while (Serial.available() > 0 && char_count < 2)
    {
        uint8_t key = Serial.read();
        key -= '0';
                  
        if (index)
        {
	    uint8_t tmp = index;
	    // index *= 10;
	    index <<= 3;
	    index += tmp;
	    index += tmp;
        }
        index += key;
        char_count ++;
    }
    return (char_count) ? (int8_t)index : (int8_t)-1;
}

