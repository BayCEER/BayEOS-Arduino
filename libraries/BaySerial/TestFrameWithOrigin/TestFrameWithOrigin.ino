#include <BayEOS.h>
#include <BaySerial.h>



BaySerial client=BaySerial(Serial);


void setup(void){
   client.begin(38400);
   client.sendMessage("Started");
}

void loop(void){
  client.startDataFrameWithOrigin(0x1,"TestSerial");
  for(uint8_t i=0;i<5;i++){
    client.addChannelValue(i);
   }
   
   client.sendPayload();
   
  delay(1000);
}
