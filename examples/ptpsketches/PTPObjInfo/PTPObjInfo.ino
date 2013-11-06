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

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
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
        uint32_t  objs[] = {0x00020001, 0x00020002, 0x00020003, 0x00020004, 0x00020005, 0x00020006, 0x00020007, 0x00020008, 0x00020009 /*0x30040000, 0x00000003, 0x00000004, 0x00000005, 0x00000006*/};

        HexDump              dmp;
        PTPObjInfoParser     prs;

        for (uint8_t i=0; i<9; i++)
        {
            E_Notify(PSTR("\r\n"), 0x80);
            PrintHex<uint32_t>(objs[i], 0x80);
            E_Notify(PSTR("\r\n----------------------\r\n"), 0x80);

            dmp.Initialize();
            ptp->GetObjectInfo(objs[i], &dmp);
            E_Notify(PSTR("\r\n\r\n"), 0x80);

            ptp->GetObjectInfo(objs[i], &prs);
            E_Notify(PSTR("\r\n"), 0x80);
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

