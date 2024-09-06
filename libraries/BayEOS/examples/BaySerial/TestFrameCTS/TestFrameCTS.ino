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

// Client with timeout 300ms, 38400 baud and CTS-pin 9
BaySerial client(Serial,300,38400,9);


void setup(void){
  pinMode(LED_BUILTIN,OUTPUT);
//Do not call client.begin(...) here
//client.begin and client.end is automatically called by sendPayload()
}

void loop(void){
   client.startDataFrameWithOrigin(BayEOS_ChannelFloat32le,"Test-Board",0,1);
   client.addChannelValue(millis()/1000);     
   client.addChannelValue(analogRead(A0));     
   client.sendPayload();
    
  delay(5000);
}
