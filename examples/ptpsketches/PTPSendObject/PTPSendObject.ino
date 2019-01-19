// #include <usbhub.h>

#include <ptp.h>
// #include <ptpdebug.h>
#include "ptpobject.h"

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
// USBHub   Hub1(&Usb);
PTP      Ptp(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp
    __attribute__ ((unused)))
{
    if (stateConnected == stConnected || stateConnected == stInitial)
    {
        stateConnected = stDisconnected;
        E_Notify(PSTR("Camera disconnected\r\n"), 0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp
    __attribute__((unused)))
{
    if (stateConnected == stDisconnected || stateConnected == stInitial)
    {
        stateConnected = stConnected;
        E_Notify(PSTR("Camera connected\r\n"), 0x80);

        // Ptp.CaptureImage();
        // delay(100);

        PTPFileInfoSupplier fileinfo(PSTR("TEST.JPG"), 12);
        E_Notify(PSTR("Sending ObjectInfo...\r\n"),0x80);
        // Serial.println(fileinfo.GetDataSize());
        Ptp.SendObjectInfo(0, &fileinfo);
        E_Notify(PSTR("ObjectInfo sent\r\n"),0x80);
        Ptp.CloseSession();
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

