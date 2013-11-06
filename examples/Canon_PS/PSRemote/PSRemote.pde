#include <usbhub.h>

#include <ptp.h>
#include <canonps.h>
#include "qp_port.h"
#include <valuelist.h>
#include <psvaluetitles.h>

#include "ptpdpparser.h"
#include "ptpobjinfoparser.h"
#include "pseventparser.h"
#include "psconsole.h"

using namespace QP;

//............................................................................
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    QF_INT_DISABLE();                                // disable all interrupts
//    USER_LED_ON();                                  // User LED permanently ON
    asm volatile ("jmp 0x0000");    // perform a software reset of the Arduino
}


class CamStateHandlers : public PSStateHandlers
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
//USBHub              Hub1(&Usb);
CanonPS             Ps(&Usb, &CamStates);

QEvent            evtTick; //, evtAbort;
PSConsole         psConsole;

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

        PSEventParser  prs;
        Ps.EventCheck(&prs);

        if (uint32_t handle = prs.GetObjHandle())
        {
                    PTPObjInfoParser     inf;
                    Ps.GetObjectInfo(handle, &inf);
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
//    evtAbort.sig = ABORT_SIG;
    psConsole.init();

    Serial.println("Start");
}

void loop()
{
    Usb.Task();
}

