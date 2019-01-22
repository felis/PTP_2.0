// #include <usbhub.h>

#include <ptp.h>
#include <ptpdebug.h>
#include "ptpeventparser.h"

class PTPEventPrintFilename : public PTPEventHandlers
{
public:
	virtual void OnEvent(PTPUSBEventContainer* evt);
};

void PTPEventPrintFilename::OnEvent(PTPUSBEventContainer* evt)
{
    switch (evt->code) {
    // case PTP_EC_DevicePropChanged:
    //    //Fifo.Push(evt->wParam1);
    //    PrintHex<uint16_t>(evt->wParam1, 0x80);
    //    Serial.println("");
    //    break;
        case PTP_EC_ObjectAdded:
            Serial.println("Obj.added\r\n");
            {
//            PTPEventPrintFilename pepf;
//            ptpcam.GetObjectInfo(evt->dwParam, &pepf);
            }
        break;
    };
}

class CamStateHandlers : public PTPStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial){};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

class PTPdevice : public PTP
{
    uint32_t     nextPollTime;   // Time of the next poll to occure

public:
    bool         bPollEnabled;   // Enables or disables camera poll

    PTPdevice(USB *pusb, PTPStateHandlers *pstates) : PTP(pusb, pstates),
        nextPollTime(0), bPollEnabled(false)
    {
    };

    uint8_t Poll() {
        static bool first_time = false;
        PTP::Poll();

        if (!bPollEnabled)
            return 0;

        if (first_time)
            InitiateCapture();

        uint32_t  current_time = millis();

        if (current_time >= nextPollTime)
        {
            Serial.println("\r\n");

            PTPEventParser prs(this);
            EventCheck(&prs);
            
            nextPollTime = current_time + 300;
        }
        first_time = false;
        return 0;
    };
};

CamStateHandlers    CamStates;
USB                 Usb;
// USBHub              Hub1(&Usb);
PTPdevice           ptpcam(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    PTPTRACE("Disconnected\r\n");
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        ((PTPdevice*)ptp)->bPollEnabled = false;
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
        ((PTPdevice*)ptp)->bPollEnabled = true;
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

