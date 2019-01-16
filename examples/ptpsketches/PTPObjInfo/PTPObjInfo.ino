#include <usbhub.h>

#include <ptp.h>
#include <ptpcallback.h>
#include <ptpdebug.h>
#include "ptpobjinfoparser.h"

class CamStateHandlers : public PTPStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;

public:
      CamStateHandlers() : stateConnected(stInitial) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
} CamStates;

USB      Usb;
USBHub   Hub1(&Usb);
PTP      Ptp(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp
    __attribute__((unused)))
{
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        stateConnected = stDisconnected;
        E_Notify(PSTR("Camera disconnected\r\n"), 0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;
        E_Notify(PSTR("Camera connected\r\n"), 0x80);

        // Before running the sketch make sure you set the obj to the handle value of existing object.
//        uint32_t  objs[] = { 0x00000002, 0x00000003, 0x00000004, 0x00000005, 0x00000006, 0x00000007, 0x00000008, 0x00000009, 0x0000000A, 0x0000000B };

        uint32_t objs[] = {0x50000000, 0x51900000, 0x5190F702, 0x5190F712, 0x5190F722, 0x5190F732, 0x5190F742, 0x5190F752, 0x5190F762, 0x5190F772, 0x5190F782, 0x5190F792, 0x5190F7A2, 0x5190F7B2, 0x5190F7C2, 0x60080000};


        HexDump              dmp;
        PTPObjInfoParser     prs;

        for (uint8_t i=0; i<16; i++)
        {

            delay(100);

            E_Notify(PSTR("\r\n"), 0x80);
            PrintHex<uint32_t>(objs[i], 0x80);
            E_Notify(PSTR("\r\n----------------------\r\n"), 0x80);

            delay(100);

            dmp.Initialize();
            ptp->GetObjectInfo(objs[i], &dmp);
            E_Notify(PSTR("\r\n\r\n"), 0x80);


            delay(100);    

            ptp->GetObjectInfo(objs[i], &prs);
            E_Notify(PSTR("\r\n"), 0x80);

            delay(100);
        }
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

