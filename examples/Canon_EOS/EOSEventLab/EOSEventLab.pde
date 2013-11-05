#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>
#include <eoseventdump.h>

class CamStateHandlers : public EOSStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates   stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

class CanonEos : public CanonEOS
{
    uint32_t     nextPollTime;   // Time of the next poll to occure

public:
    bool         bPollEnabled;   // Enables or disables camera poll

    CanonEos(USB *pusb, PTPStateHandlers *pstates) : CanonEOS(pusb, pstates), nextPollTime(0), bPollEnabled(false)
    {
    };

    virtual uint8_t Poll()
    {
        PTP::Poll();

        if (!bPollEnabled)
            return 0;

        uint32_t  current_time = millis();

        if (current_time >= nextPollTime)
        {
            Serial.println("\r\n");

            EOSEventDump  hex;
            EventCheck(&hex);

            nextPollTime = current_time + 500;
        }
        return 0;
    };
};

CamStateHandlers    CamStates;
USB                 Usb;
USBHub              Hub1(&Usb);
CanonEos            Eos(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        ((CanonEos*)ptp)->bPollEnabled = false;
        stateConnected = stDisconnected;
        E_Notify(PSTR("\r\nDevice disconnected.\r\n"),0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;
        E_Notify(PSTR("\r\nDevice connected.\r\n"),0x80);
        ((CanonEos*)ptp)->bPollEnabled = true;
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

