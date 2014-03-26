#include <BayEOS.h>
#include <BaySerial.h>



BaySerial client=BaySerial();


void setup(void){
   client.begin(38400);
}

void loop(void){
  client.startDataFrame(0x1);
  for(uint8_t i=0;i<5;i++){
    client.addChannelValue((float) 1.0000);
   }
   
   client.sendPayload();
   
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
}
