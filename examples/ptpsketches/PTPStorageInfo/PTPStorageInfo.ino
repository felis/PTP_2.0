#include <usbhub.h>

#include <ptp.h>
#include <ptpdebug.h>
#include "stinfoparser.h"

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

        uint8_t    data[48];

        if (ptp->GetStorageIDs(48, data) == PTP_RC_OK)
        {
              uint32_t    cntdn = *((uint32_t*)(data+12)), *p = (uint32_t*)(data+16);
              Serial.println(cntdn, DEC);

              for (; cntdn; cntdn--, p++)
              {
                    E_Notify(PSTR("Storage ID:\t"), 0x80);
                    PrintHex<uint32_t>(*p, 0x80);
                    E_Notify(PSTR("\n------------------------\n"), 0x80);

                    HexDump  hex;
                    ptp->GetStorageInfo(*p, &hex);

                    PTPStorageInfoParser  stiParser;
                    ptp->GetStorageInfo(*p, &stiParser);
                    E_Notify(PSTR("\n"), 0x80);
              }
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

