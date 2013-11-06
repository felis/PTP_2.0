#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>
#include <qp_port.h>

#include "hdrcapture.h"

class CamStateHandlers : public EOSStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

CamStateHandlers    CamStates;
USB                 Usb;
USBHub              Hub1(&Usb);
CanonEOS            Eos(&Usb, &CamStates);
HDRCapture          hdrCapture(Eos);
QEvent              msTick;
QEvent              secTick;

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        stateConnected = stDisconnected;
        E_Notify(PSTR("Camera disconnected\r\n"),0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;

        E_Notify(PSTR("Camera connected\r\n"),0x80);

        SetEvt         setEvt;
        setEvt.sig     = RUN_SIG;

        hdrCapture.PostEvent(&setEvt);
    }
    hdrCapture.Run();
    hdrCapture.PostEvent(&msTick);                   // post TICK_MILLIS_SIG event
}

void setup()
{
    Serial.begin( 115200 );
    Serial.println("Start");

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay( 200 );

    hdrCapture.init();

    SetEvt  setEvt;

    setEvt.sig     = SET_FRAMES_SIG;
    setEvt.value   = 3;

    hdrCapture.dispatch(&setEvt);

    setEvt.sig     = SET_FRAME_TIMEOUT_SIG;
    setEvt.value   = 5;

    hdrCapture.dispatch(&setEvt);

    setEvt.sig     = SET_SELF_TIMEOUT_SIG;
    setEvt.value   = 3;

    hdrCapture.dispatch(&setEvt);

    SetBktEvt          setBktEvt;
    setBktEvt.sig       = SET_BRACKETING_SIG;
    setBktEvt.step      = 2;
    setBktEvt.negative  = 3;
    setBktEvt.positive  = 9;

    hdrCapture.dispatch(&setBktEvt);
    msTick.sig = TICK_MILLIS_SIG;
}

void loop()
{
    Usb.Task();
    delay(1);
}

