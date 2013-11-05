#include <usbhub.h>

#include <ptp.h>
#include <nkeventparser.h>
#include <nikon.h>
#include "qp_port.h"
#include <valuelist.h>
#include <nkvaluetitles.h>

#include "ptpdpparser.h"
#include "ptpobjinfoparser.h"
#include "psconsole.h"

class NKEventDump : public NKEventHandlers
{
public:
	virtual void OnEvent(const NKEvent *evt);
};

void NKEventDump::OnEvent(const NKEvent *evt)
{
    switch (evt->eventCode)
    {
    case PTP_EC_DevicePropChanged:
        //Fifo.Push(evt->wParam1);
        PrintHex<uint16_t>(evt->wParam1, 0x80);
        Serial.println("");
        break;
    case PTP_EC_ObjectAdded:
        {
//            PTPObjInfoParser     inf;
//            Nk.GetObjectInfo(evt->dwParam, &inf);
        }
        break;
    };
}

class CamStateHandlers : public PTPStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

      uint32_t nextPollTime;

public:
      CamStateHandlers() : stateConnected(stInitial), nextPollTime(0)
      {
      };

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

CamStateHandlers  CamStates;

USB                 Usb;
NikonDSLR           Nk(&Usb, &CamStates);
NKEventDump         Dmp;

QEvent            evtTick;
PSConsole         psConsole;

void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    (void)file;                                   /* avoid compiler warning */
    (void)line;                                   /* avoid compiler warning */
    QF_INT_DISABLE();             /* make sure that interrupts are disabled */
    for (;;) {
    }
}

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        stateConnected = stDisconnected;
        E_Notify(PSTR("Camera disconnected.\r\n"),0x80);

        if (stateConnected == stConnected)
            psConsole.dispatch(&evtTick);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;
        E_Notify(PSTR("Camera connected.\r\n"),0x80);
        psConsole.dispatch(&evtTick);
        ptp->SetDevicePropValue(0xD1B0, (uint8_t)1);
    }
    int8_t  index = psConsole.MenuSelect();

    if (index >= 0)
    {
        MenuSelectEvt     menu_sel_evt;
        menu_sel_evt.sig         = MENU_SELECT_SIG;
        menu_sel_evt.item_index  = index;
        psConsole.dispatch(&menu_sel_evt);      // dispatch the event
    }
    uint32_t time_now = millis();

    if (time_now >= nextPollTime)
    {
        nextPollTime = time_now + 300;

        NKEventParser  prs(&Dmp);
        Nk.EventCheck(&prs);
    }
}

void setup()
{
    Serial.begin(115200);

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay( 200 );

    evtTick.sig = TICK_SIG;
//    evtAbort.sig = ABORT_SIG;
    psConsole.init();

    Serial.println("Start");
}

void loop()
{
    Usb.Task();
}

