#include <BayEOS.h>
#include <BaySerial.h>
#include <SoftwareSerial.h>
#include <BaySoftwareSerial.h>


BaySoftwareSerial client=BaySoftwareSerial(8,9);


void setup(void){
   client.begin(9600);
}

void loop(void){
  client.startDataFrame(0x41);
  for(uint8_t i=0;i<5;i++){
    client.addToPayload(i);
    client.addToPayload((float) 1.0000);
   }
   
   client.sendPayload();
   
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
}

