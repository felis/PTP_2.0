#include <usbhub.h>
#include <eoseventdump.h>

#include <ptp.h>
#include <ptpdebug.h>
#include <nikon.h>

class CamStateHandlers : public PTPStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial){};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

class Nikon : public NikonDSLR
{
    uint32_t     nextPollTime;   // Time of the next poll to occure

public:
    bool         bPollEnabled;   // Enables or disables camera poll

    Nikon(USB *pusb, PTPStateHandlers *pstates) : NikonDSLR(pusb, pstates), nextPollTime(0), bPollEnabled(false)
    {
    };

    virtual uint8_t Poll()
    {
        static bool first_time = true;
        PTP::Poll();

        if (!bPollEnabled)
            return 0;

        if (first_time)
            InitiateCapture();

        uint32_t  current_time = millis();

        if (current_time >= nextPollTime)
        {
            Serial.println("\r\n");

            HexDump  hex;
            EventCheck(&hex);

            nextPollTime = current_time + 300;
        }
        first_time = false;
        return 0;
    };
};

CamStateHandlers    CamStates;
USB                 Usb;
USBHub              Hub1(&Usb);
Nikon               Nik(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    PTPTRACE("Disconnected\r\n");
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        ((Nikon*)ptp)->bPollEnabled = false;
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
        ((Nikon*)ptp)->bPollEnabled = true;
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

