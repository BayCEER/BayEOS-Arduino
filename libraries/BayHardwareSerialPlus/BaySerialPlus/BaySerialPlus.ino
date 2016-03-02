#include <HardwareSerialPlus.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <BayHardwareSerialPlus.h>



BaySerialPlus client=BaySerialPlus();


void setup(void){
   client.begin(9600);
}

void loop(void){
   client.startDataFrame();
   client.addChannelValue(millis()/1000);     
   client.addChannelValue(analogRead(A0));     
   client.sendPayload();
  
   client.sendMessage("Just a message ;-)");

   client.sendError("Just a test error message ;-)");
   
  delay(5000);
}
