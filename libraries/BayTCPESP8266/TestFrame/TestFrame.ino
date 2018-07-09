#include <BayTCPESP8266.h>


BayESP8266 client=BayESP8266(Serial,7);

void setup(void){
  Serial.begin(9600);
  Serial.println("Starting");
  client.readConfigFromStringPGM(
  PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|TestWLAN|SSID|PASSWORD||"));
  client.begin(9600);
  
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame();
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   client.powerDown();
   delay(10000);  
}

