#include <BayEOS.h>
#include <SoftwareSerial.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPESP8266.h>


BayESP8266 client=BayESP8266(Serial);

void setup(void){
  Serial.begin(9600);
  Serial.println("Starting");
  client.readConfigFromStringPGM(
  PSTR("192.168.2.1|80|gateway/frame/saveFlat|import|import|TestWLAN|SSID|PASSWORD||"));
  client.begin(9600);
  
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame();
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   delay(5000);  
}

