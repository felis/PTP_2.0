#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>

class CamStateHandlers : public EOSStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

CamStateHandlers  CamStates;
USB                 Usb;
USBHub              Hub1(&Usb);
CanonEOS            Eos(&Usb, &CamStates);

void setup()
{
    Serial.begin( 115200 );
    Serial.println("Start");

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay( 200 );
}

void loop()
{
    Usb.Task();
}

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        stateConnected = stDisconnected;
        E_Notify(PSTR("Camera disconnected.\r\n"),0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;

        // Switch LiveView on
        ((CanonEOS*)ptp)->SwitchLiveView(true);
        delay(50);

        for (uint8_t i=0; i<10; i++)
        {
            if (i > 4)
                ((CanonEOS*)ptp)->MoveFocus(3);
            else
                ((CanonEOS*)ptp)->MoveFocus(0x8003);

            delay(100);
            ((CanonEOS*)ptp)->Capture();
            delay(1500);
        }

        // Switch LiveView off
        ((CanonEOS*)ptp)->SwitchLiveView(false);
    }
}

