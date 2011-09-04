#include <inttypes.h>
#include <avr/pgmspace.h>

#include <avrpins.h>
#include <max3421e.h>
#include <usbhost.h>
#include <usb_ch9.h>
#include <Usb.h>
#include <usbhub.h>
#include <address.h>

#include <message.h>
#include <parsetools.h>

#include <ptp.h>
#include <ptpdebug.h>
#include <canonps.h>
#include <ptpdpparser.h>
#include <psvaluetitles.h>
#include "devpropparser.h"

class CamStateHandlers : public PTPStateHandlers
{
      enum CamStates { stInitial, stDisconnected, stConnected };
      CamStates stateConnected;
    
public:
      CamStateHandlers() : stateConnected(stInitial) {};
      
      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
};

CamStateHandlers  CamStates;
USB                 Usb;
USBHub              Hub1(&Usb);
PTP                 Ptp(&Usb, &CamStates);

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
        
        uint16_t    prefix[3] = { 0x5000, 0xD000, 0xD100 };
               
        for (uint8_t i=0; i<3; i++)
        {
            for (uint8_t j=0; j<128; j++)
            {
                HexDump          dmp;
                
                if (Ptp.GetDevicePropDesc((prefix[i] | j), &dmp) == PTP_RC_OK)
                {
                    Notify(PSTR("\r\n"));
                    
                    DevPropParser    prs;
                    
            	    if (Ptp.GetDevicePropDesc((prefix[i] | j), &prs) == PTP_RC_OK)
                        Notify(PSTR("\r\n"));
                }
            }
    
            Notify(PSTR("\r\n"));

        } // for (uint8_t i=0; i<2; i++)

    }  // if (stateConnected == stDisconnected || stateConnected == stInitial)
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

