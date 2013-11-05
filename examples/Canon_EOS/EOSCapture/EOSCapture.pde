#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>

class CamStateHandlers : public PTPStateHandlers
{
      bool stateConnected;

public:
      CamStateHandlers() : stateConnected(false) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
} CamStates;

USB         Usb;
USBHub      Hub1(&Usb);
CanonEOS    Eos(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected)
    {
        stateConnected = false;
        E_Notify(PSTR("Camera disconnected\r\n"),0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    static uint32_t next_time = 0;

    if (!stateConnected)
        stateConnected = true;

    uint32_t  time_now = millis();

    if (time_now > next_time)
    {
        next_time = time_now + 5000;

        uint16_t rc = Eos.Capture();

        if (rc != PTP_RC_OK)
            ErrorMessage<uint16_t>("Error", rc);
    }
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

