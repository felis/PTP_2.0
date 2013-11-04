#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>
#include <qp_port.h>
#include <valuelist.h>
#include <eosvaluetitles.h>
#include <eoseventparser.h>

#include "eoseventhandlers.h"
#include "EOSConsole.h"

#define EEP_APERTURE_LIST_OFFSET    0
#define EEP_APERTURE_LIST_SIZE      32

#define EEP_SHTSPEED_LIST_OFFSET    (EEP_APERTURE_LIST_OFFSET + EEP_APERTURE_LIST_SIZE)
#define EEP_SHTSPEED_LIST_SIZE      64

#define EEP_WBALANCE_LIST_OFFSET    (EEP_SHTSPEED_LIST_OFFSET + EEP_SHTSPEED_LIST_SIZE)
#define EEP_WBALANCE_LIST_SIZE      12

#define EEP_PICSTYLE_LIST_OFFSET    (EEP_WBALANCE_LIST_OFFSET + EEP_WBALANCE_LIST_SIZE)
#define EEP_PICSTYLE_LIST_SIZE      12

#define EEP_EXPOCOR_LIST_OFFSET     (EEP_PICSTYLE_LIST_OFFSET + EEP_PICSTYLE_LIST_SIZE)
#define EEP_EXPOCOR_LIST_SIZE       48

#define EEP_ISO_LIST_OFFSET         (EEP_EXPOCOR_LIST_OFFSET + EEP_EXPOCOR_LIST_SIZE)
#define EEP_ISO_LIST_SIZE           8

EEPROMByteList          vlAperture(EEP_APERTURE_LIST_OFFSET, EEP_APERTURE_LIST_SIZE);
EEPROMByteList          vlShutterSpeed(EEP_SHTSPEED_LIST_OFFSET, EEP_SHTSPEED_LIST_SIZE);
EEPROMByteList          vlWhiteBalance(EEP_WBALANCE_LIST_OFFSET, EEP_WBALANCE_LIST_SIZE);
EEPROMByteList          vlPictureStyle(EEP_PICSTYLE_LIST_OFFSET, EEP_PICSTYLE_LIST_SIZE);
EEPROMByteList          vlIso(EEP_ISO_LIST_OFFSET, EEP_ISO_LIST_SIZE);
EEPROMByteList          vlExpCompensation(EEP_EXPOCOR_LIST_OFFSET, EEP_EXPOCOR_LIST_SIZE);

class CamStateHandlers : public EOSStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial)
      {
      };

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

CamStateHandlers    CamStates;
USB                 Usb;
USBHub              Hub1(&Usb);
CanonEOS            Eos(&Usb, &CamStates);

QEvent            evtTick, evtAbort;
EOSConsole        eosConsole;

uint8_t  dpMode         = 0;
uint8_t  dpAperture     = 0;
uint8_t  dpShutterSpeed = 0;
uint8_t  dpWb           = 0;
uint8_t  dpPStyle       = 0;
uint8_t  dpIso          = 0;
uint8_t  dpExpComp      = 0;

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        stateConnected = stDisconnected;
        //PTPPollTimer.Disable();
        E_Notify(PSTR("Camera disconnected.\r\n"),0x80);

        if (stateConnected == stConnected)
            eosConsole.dispatch(&evtTick);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    static uint32_t  next_time = 0;

    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;
        eosConsole.dispatch(&evtTick);
    }
    uint32_t time_now = millis();

    if (time_now >= next_time)
    {
        next_time = time_now + 300;

        EosEventHandlers  hnd;
        EOSEventParser    prs(&hnd);
        Eos.EventCheck(&prs);

        int8_t  index = eosConsole.MenuSelect();

        if (index >= 0)
        {
            MenuSelectEvt     menu_sel_evt;
            menu_sel_evt.sig         = MENU_SELECT_SIG;
            menu_sel_evt.item_index  = index;
            eosConsole.dispatch(&menu_sel_evt);      // dispatch the event
        }
    }
}

void setup()
{
    Serial.begin(115200);

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay( 200 );

    evtTick.sig = TICK_SIG;
    eosConsole.init();

    Serial.println("Start");
}

void loop()
{
    Usb.Task();
}

