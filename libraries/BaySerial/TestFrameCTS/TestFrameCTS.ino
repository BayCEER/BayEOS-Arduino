/*
 * Example sketch for connecting multiple serial senders to one receiver
 * 
 * RX1 -----------
 * TX1 --------- |
 * CTS1 ------ | |
 *           | | |
 * RX2 ------|-|------ TX
 * TX2 ------|-------- RX
 * CTS2 -------------- CTS
 * 
 * Bevor sending CTS is checked. If pulled low another sender is active
 */

#include <BaySerial.h>

// Client with timeout 300ms, 9600 baud and CTS-pin 9
BaySerial client=BaySerial(Serial,300,9600,9);


void setup(void){
//Do not call client.begin(...) here
//client.begin and client.end is automatically called by sendPayload()
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
