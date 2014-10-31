/*
  Send distance of HRXL MaxSonar 7364 with XBEE
  Oliver Archner
  30.10.2014 Created 
  
*/

#include <Wire.h>
#include <MB7364.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include <BayEOS.h>
#include <BayXBee.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h> 

MB7364 sonar(10,11);
BayXBee xbee=BayXBee();
BayEOSBufferRAM myBuffer; 

void setup(void){
  xbee.begin(38400);      
  sonar.begin();
  myBuffer=BayEOSBufferRAM(1000);
  xbee.setBuffer(myBuffer);  
  delay(250);   
}

int counter = 0;
  
void loop(void){ 
  xbee.sendFromBuffer();
  xbee.startDataFrame(BayEOS_ChannelInt32le); 
  int value = sonar.range();  
  xbee.addChannelValue(++counter,1);
  if ((value <= 500) || (value > 4999)){
	xbee.addChannelValue(value,3);
  } else {
       xbee.addChannelValue(value,2);
  }  
  
  xbee.sendOrBuffer();      
  delay(10000); 
} 