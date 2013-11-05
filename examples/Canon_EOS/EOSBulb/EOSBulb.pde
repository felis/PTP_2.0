#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>

#define SHUTTER_SPEED_BULB       0x0c

class CamStateHandlers : public EOSStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
} CamStates;

USB                 Usb;
USBHub              Hub1(&Usb);
CanonEOS            Eos(&Usb, &CamStates);

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

         uint16_t rc = ((CanonEOS*)ptp)->SetProperty(EOS_DPC_ShutterSpeed,SHUTTER_SPEED_BULB);

        if (rc != PTP_RC_OK)
            ErrorMessage<uint16_t>("Error", rc);
    }

    ((CanonEOS*)ptp)->StartBulb();
    delay(6000);
    ((CanonEOS*)ptp)->StopBulb();
}

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
