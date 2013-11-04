/*

Yanis - Android-Arduino Wireless EOS Controller


File: YAWEOS.pde
version: v.1.0 28 August 2011

Author: Manishi Barosee (manis404)

Includes code derivatives of Sandro Benigno(ArducamOSD)

(USB host and PTP library from Oleg Mazurov - circuitsathome.com)
(PTP 2.0 adaptation - Oleg Mazurov - circuitsathome.com


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.

*/

#include <usbhub.h>

#include <ptp.h>
#include <canoneos.h>


//#define SerPri  Serial.print
//#define SerPriln Serial.println
//#define SerAva  Serial.available
//#define SerRea  Serial.read

uint16_t x;

class CamStateHandlers : public EOSStateHandlers
{
      //enum CamStates { stInitial, stDisconnected, stConnected };
      //CamStates stateConnected;
      bool stateConnected;

public:
      CamStateHandlers() : stateConnected(false) {};

      virtual void OnDeviceDisconnectedState(PTP *ptp);
      virtual void OnDeviceInitializedState(PTP *ptp);
} CamStates;

//CamStateHandlers    CamStates;
USB                 Usb;
CanonEOS            Eos(&Usb, &CamStates);

void CamStateHandlers::OnDeviceDisconnectedState(PTP *ptp)
{
    //if (stateConnected == stConnected || stateConnected == stInitial) {
    //    stateConnected = stDisconnected;
    if (stateConnected)
    {
        stateConnected = false;
        E_Notify(PSTR("Camera disconnected\r\n"),0x80);
    }
}

void CamStateHandlers::OnDeviceInitializedState(PTP *ptp)
{
      if (!stateConnected)
        stateConnected = true;

    //if (stateConnected == stDisconnected || stateConnected == stInitial) {
    //    stateConnected = stConnected;
    //}

    while( stateConnected == true ){
      //Serial.println(".");
      //delay(1000);
      readSerialCommand();
    }

   // if (rc != PTP_RC_OK)
     //   Message(PSTR("Error: "), rc);

    delay(5000);
}

void setup()
{
  Serial.begin( 9600 );
  Serial.println("Start");
  if (Usb.Init() == -1) {
    Serial.println("OSC did not start.");
  }
  //Eos.Setup();
  delay( 200 );
}

void loop()
{
    Usb.Task();

}

void readSerialCommand() {
  char queryType;
  if ( Serial.available()) {
    queryType = Serial.read();
    switch (queryType) {

    case 'C': //Capture!!!
      Serial.println("Capture!");
      Eos.Capture();
      delay(500);
      break;


    case 'O': //ViewFinder Output. 1 = LCD. 2 = AV.
      //Eos.SetDevicePropValue(EOS_DPC_CameraOutput, (uint16_t)readFloatSerial());
      delay(1000);
      break;

    case 'V': //Liveview ON/OFF
      if((uint16_t)readFloatSerial() == 0){
        //((CanonEOS*)ptp)->SwitchLiveView(false);
        Eos.SwitchLiveView(false);
        Serial.println("Live View OFF!");
      }
      else {
        //((CanonEOS*)ptp)->SwitchLiveView(true);
        Eos.SwitchLiveView(true);
        Serial.println("Live View ON!");
      }
      delay(1000);
      break;
    case 'I': //Set Iso
      Eos.SetProperty(EOS_DPC_Iso, (uint16_t)readFloatSerial());
      delay(1000);
       Serial.println("ISO Changed!");
      break;
    case 'S': //Set ShutterSpeed
      Eos.SetProperty(EOS_DPC_ShutterSpeed, (uint16_t)readFloatSerial());
      delay(1000);
       Serial.println("Shutter Speed Changed!");
      break;
    case 'W': //Set Whitebalance
      Eos.SetProperty(EOS_DPC_WhiteBalance, (uint16_t)readFloatSerial());
      delay(1000);
      Serial.println("White Balance Changed!");
      break;
    case 'A': //Set Aperture
      Eos.SetProperty(EOS_DPC_Aperture, (uint16_t)readFloatSerial());
      delay(1000);
       Serial.println("Aperture value Changed!");
      break;



      case 'F': //MoveFocus

      Eos.MoveFocus(3);

      break;

      case 'B': //MoveFocus


      Eos.MoveFocus(0x8003);

      break;

    }
  }
}


float readFloatSerial() {
  byte index = 0;
  byte timeout = 0;
  char data[128] = "";

  do {
    if (Serial.available() == 0) {
      delay(10);
      timeout++;
    }
    else {
      data[index] = Serial.read();
      timeout = 0;
      index++;
    }
  }
  while ((data[constrain(index-1, 0, 128)] != ';') && (timeout < 5) && (index < 128));
  return atof(data);
}

