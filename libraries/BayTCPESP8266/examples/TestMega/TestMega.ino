#include <BayTCPESP8266.h>


BayESP8266 client(Serial1,A15);

void setup(void){
  Serial.begin(115200);
  Serial.println("Starting");
  client.readConfigFromStringPGM(
  PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|TestWLAN|@BayernWLAN|||"));
  client.begin(115200);
  
}

void loop(void){
  //Construct DataFrame
   client.startDataFrame();
   client.addChannelValue(millis()/1000);
   Serial.print("Sending ...");     
   uint8_t res=client.sendPayload();
   if(res){
    Serial.println("failed");
    Serial.print("Error code: ");
    Serial.println(res);
   } else {
    Serial.println("ok");
   }
   client.powerDown();
   delay(20000);  
}

