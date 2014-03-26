#include <BayEOS.h>
#include <SoftwareSerial.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>

BayGPRS client=BayGPRS(Serial2,46);

void setup(void){
//  Serial.begin(9600);
//  client.softSwitch();
  client.readConfigFromStringPGM(
  PSTR("192.168.0.1|80|gateway/frame/saveFlat|admin|xbee|TestGPRS|pinternet.interkom.de|||PIN"));
  client.begin(38400);
  
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame(BayEOS_Float32le);
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   
   
    
  delay(5000);
   
}

