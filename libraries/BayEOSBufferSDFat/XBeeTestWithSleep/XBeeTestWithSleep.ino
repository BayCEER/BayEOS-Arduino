#include <Wire.h>
#include <BayEOSBuffer.h>
#include <RTClib.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>
#include <Sleep.h>

#define XBEE_SLEEP_PIN 5


BayXBee client=BayXBee(Serial,XBEE_SLEEP_PIN,15,1000); 
BayEOSBufferSDFat myBuffer;
unsigned long last_data;
DS3231 myRTC; //Seduino 2.2

//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
// You have to set this! Otherwise it will call reset!
ISR(WDT_vect) {
}

unsigned long startup_time;
uint16_t count=0;

void setup(void){
  Sleep.setupWatchdog(6); //init watchdog timer to 1 sec
   Wire.begin();
   myRTC.begin();
  startup_time=myRTC.now().get();
  client.begin(38400);
  //Set 4 for EthernetShield, 10 for Stalker
  if (!SD.begin(10)) {
    return;
  }
  
  myBuffer = BayEOSBufferSDFat(100000000); 
//  myBuffer.setRTC(myRTC); //absolute time! You have to set the RTC!!
  myBuffer.setRTC(myRTC,false); //relative time! Use RTC instead of millis() to calculate relative time  
  client.setBuffer(myBuffer);
}

void loop(void){

  //Resend buffered frames
  //one every second
  //sending to frequently my make xbee operationable (channel overload)
  client.sendFromBuffer();

  if((myRTC.now().get()-last_data)>9){
  	  last_data=myRTC.now().get();
 	  
      myRTC.convertTemperature();  
 	  //Construct DataFrame
      client.startDataFrame(BayEOS_Float32le);
      client.addChannelValue(count);  
      client.addChannelValue(myRTC.now().get()-startup_time);  
      client.addChannelValue(myRTC.getTemperature());
      analogReference(INTERNAL);
      client.addChannelValue((1.1 / 1024)*analogRead(A7)*(10+2)/2);
      client.writeToBuffer();
      
      //Construct Message
     // client.startFrame(BayEOS_Message);
     // client.addToPayload("Just a message ;-)");
     // client.sendOrBuffer();
  }       
  Sleep.sleep();  

}
