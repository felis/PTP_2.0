#include <inttypes.h>
#include <avr/pgmspace.h>

#include <avrpins.h>
#include <max3421e.h>
#include <usbhost.h>
#include <usb_ch9.h>
#include <Usb.h>
#include <usbhub.h>
#include <address.h>

#include <printhex.h>
#include <message.h>
#include <hexdump.h>
#include <parsetools.h>

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
        Notify(PSTR("Camera disconnected\r\n"));
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;
        Notify(PSTR("Camera connected\r\n"));
        
        uint8_t    data[48];
        
        if (ptp->GetStorageIDs(48, data) == PTP_RC_OK)
        {
              uint32_t    cntdn = *((uint32_t*)(data+12)), *p = (uint32_t*)(data+16);
              Serial.println(cntdn, DEC);
              
              for (; cntdn; cntdn--, p++)
              {
                    Notify(PSTR("Storage ID:\t"));
                    PrintHex<uint32_t>(*p);
                    Notify(PSTR("\n------------------------\n"));
                    
                    HexDump  hex;
                    ptp->GetStorageInfo(*p, &hex);
                    
                    PTPStorageInfoParser  stiParser;
                    ptp->GetStorageInfo(*p, &stiParser);
                    Notify(PSTR("\n"));
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

