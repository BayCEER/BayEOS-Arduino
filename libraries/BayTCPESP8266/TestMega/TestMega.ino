#include <BayTCPESP8266.h>


BayESP8266 client=BayESP8266(Serial1,A15);

void setup(void){
  Serial.begin(115200);
  Serial.println("Starting");
  client.readConfigFromStringPGM(
  PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|TestWLAN|@BayernWLAN|||"));
  client.begin(115200);
  
}

void loop(void){
   client.powerUp();
  //Construct DataFrame
   client.startDataFrame();
   client.addChannelValue(millis()/1000);     
   client.sendPayload();
   client.powerDown();
   delay(20000);  
}

