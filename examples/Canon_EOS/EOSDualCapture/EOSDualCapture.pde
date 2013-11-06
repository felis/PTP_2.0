#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>

class EOSCamStateHandlers : public PTPStateHandlers
{
      bool stateConnected;

public:
      EOSCamStateHandlers() : stateConnected(false) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
} EOSStates;

class PTPCamStateHandlers : public PTPStateHandlers
{
      bool stateConnected;

public:
      PTPCamStateHandlers() : stateConnected(false) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
} PTPStates;

USB         Usb;
USBHub      Hub1(&Usb);
CanonEOS    Eos(&Usb, &EOSStates);
PTP         Ptp(&Usb, &PTPStates);

void EOSCamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected)
    {
        stateConnected = false;
        E_Notify(PSTR("Camera at: "),0x80);
        Serial.print(ptp->GetAddress(),HEX);
        E_Notify(PSTR(" disconnected\r\n"),0x80);
    }
}

void PTPCamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected)
    {
        stateConnected = false;
        E_Notify(PSTR("Camera at: "),0x80);
        Serial.print(ptp->GetAddress(),HEX);
        E_Notify(PSTR(" disconnected\r\n"),0x80);
    }
}

void EOSCamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    static uint32_t next_time = 0;

    if (!stateConnected)
    {
        stateConnected = true;
        E_Notify(PSTR("Camera at: "),0x80);
        Serial.print(ptp->GetAddress(),HEX);
        E_Notify(PSTR(" connected\r\n"),0x80);
    }

    uint32_t  time_now = millis();

    if (time_now > next_time)
    {
        next_time = time_now + 5000;

        uint16_t rc = Eos.Capture();

        if (rc != PTP_RC_OK)
            ErrorMessage<uint16_t>("Error", rc);
    }
}

void PTPCamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    static uint32_t next_time = 0;

    if (!stateConnected)
    {
        stateConnected = true;
        E_Notify(PSTR("Camera at: "),0x80);
        Serial.print(ptp->GetAddress(),HEX);
        E_Notify(PSTR(" connected\r\n"),0x80);
    }

    uint32_t  time_now = millis();

    if (time_now > next_time)
    {
        next_time = time_now + 10000;

        Serial.println("CaptureImage");

        uint16_t rc = ptp->CaptureImage();

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

