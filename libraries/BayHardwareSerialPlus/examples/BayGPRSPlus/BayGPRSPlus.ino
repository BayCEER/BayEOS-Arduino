#include <HardwareSerialPlus.h>
#include <BayEOS.h>
#include <SoftwareSerial.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>
#include <BayHardwareSerialPlus.h>

//Define Serial and powerPin - Depends on the Board
//BayGPRSPlus client=BayGPRSPlus(SerialPlus3,46);
BayGPRSPlus client=BayGPRSPlus();

void setup(void){
//  client.softSwitch();
  client.readConfigFromStringPGM(
  PSTR("132.180.112.128|80|gateway/frame/saveFlat|import|import|TestGPRS|pinternet.interkom.de|||1802|"));
  client.begin(38400);
  
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame();
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   delay(5000);  
}

