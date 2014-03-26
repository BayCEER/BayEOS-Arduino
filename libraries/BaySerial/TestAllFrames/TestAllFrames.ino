/*
* Test-Sketch sending all types of BayEOS-Frames...
*/

#include <BayEOS.h>
#include <BaySerial.h>



BaySerial client=BaySerial();


void setup(void){
   client.begin(38400);
}

void loop(void){
  client.startDataFrame(0x1);
  client.addChannelValue(1.0);
  client.addChannelValue(millis());
  client.addChannelValue(random(1,100));
  client.sendPayload();
   
  client.sendMessage("Just a message ;-)");

  client.sendError("Just a test error message ;-)");
  
  client.startCommand(0x1);
  client.addToPayload("command 0x1");
  client.sendPayload();
  
  client.startCommandResponse(0x1);
  client.addToPayload("response 0x1");
  client.sendPayload();
  
  client.startDelayedFrame(5000);
  client.addToPayload((uint8_t) 0x4);
  client.addToPayload("Delayed Message");
  client.sendPayload();
 
  client.startTimestampFrame(5000);
  client.addToPayload((uint8_t) 0x4);
  client.addToPayload("Timestamped  Message");
  client.sendPayload();
 
  client.startRoutedFrame(2,10002);
  client.addToPayload((uint8_t) 0x4);
  client.addToPayload("Routed Message");
  client.sendPayload();

  client.startRoutedFrame(2,10002,70);
  client.addToPayload((uint8_t) 0x4);
  client.addToPayload("Routed Message RSSI");
  client.sendPayload();
 
  client.startFrame(BayEOS_OriginFrame);
  client.addToPayload((uint8_t) 8);
  client.addToPayload("TEST-dev");
  client.addToPayload((uint8_t) 0x4);
  client.addToPayload("Origin-Message");
  client.sendPayload();
  
  //etwas kompliziertes...
  client.startFrame(BayEOS_OriginFrame);
  client.addToPayload((uint8_t) 8);
  client.addToPayload("TEST-dev");
  
  client.addToPayload((uint8_t) BayEOS_RoutedFrameRSSI);
  client.addToPayload((uint16_t) 3);
  client.addToPayload((uint16_t) 10004);
  client.addToPayload((uint8_t) 75);
  
  client.addToPayload((uint8_t) BayEOS_DelayedFrame);
  client.addToPayload((unsigned long) 10000);
  
  client.addToPayload((uint8_t) 0x4);
  client.addToPayload("Origin-Routed-Delayed-Message");
  client.sendPayload();
 
 
  client.startFrame(BayEOS_BinaryFrame);
  client.addToPayload((unsigned long) 4567);
  client.addToPayload("Binary chunk...");
  client.sendPayload();
  
   
  delay(5000);
}
