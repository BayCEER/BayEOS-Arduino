#include <Wire.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>
#include <Sleep.h>

#define XBEE_SLEEP_PIN 5

BayXBee client=BayXBee(XBEE_SLEEP_PIN,15,1000); //Sleep-Pin - Wakeuptime, timeout
DS3231 myRTC; //Seduino 2.2

BayEOSBufferRAM myBuffer;
unsigned long last_data;

float count=0;
uint8_t skip_count=10;
//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
// You have to set this! Otherwise it will call reset!
ISR(WDT_vect) {
}


void setup(void){
  Wire.begin();
  myRTC.begin();
  Sleep.setupWatchdog(6); //init watchdog timer to 1 sec
  pinMode(XBEE_SLEEP_PIN, OUTPUT);
  client.begin(38400);
  myBuffer=BayEOSBufferRAM(2000);
  myBuffer.setRTC(myRTC,false); //Nutze RTC jedoch relativ!
  client.setBuffer(myBuffer,20);
}

void loop(void){

  //Resend buffered frames
  //one every second
  //sending to frequently my make xbee operationable (channel overload)
   client.sendFromBuffer();
  skip_count++;
  if(skip_count>=10){
    skip_count=0;
	  
 	  //Construct DataFrame
      count++;
      client.startDataFrame(BayEOS_Float32le);
      client.addChannelValue(count);
      analogReference(INTERNAL);
      client.addChannelValue((1.1 / 1024)*analogRead(A7)*(10+2)/2);
      client.writeToBuffer();
  }                                                                                                                                               
    Sleep.sleep();

}
