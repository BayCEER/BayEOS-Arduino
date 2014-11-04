/**************************************************

Example for Seduino Stalker with SD-Card

All data is written to SD-card before sending via XBee

**************************************************/


#include <Wire.h>
#include <MB7364.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <BayXBee.h>

#define SAMPLING_INT 30


unsigned long last_data;
DS3231 myRTC; //Seduino 2.2
BayEOSBufferSDFat myBuffer;
BayXBee client=BayXBee(); 
MB7364 sonar(8,9);


void setup(){
  Wire.begin();
  myRTC.begin();
  sonar.begin();
  client.begin(38400);
  pinMode(10, OUTPUT);
  while(!SD.begin(10)) {
    delay(10000);
    client.sendError("No SD!");
    delay(10000);
  }
  myBuffer=BayEOSBufferSDFat(200000000,1);
  myBuffer.setRTC(myRTC,false); //Nutze RTC jedoch relativ!
  client.setBuffer(myBuffer); 
}

int counter = 0;

void loop() {
  if((myRTC.now().get()-last_data)>=SAMPLING_INT){
    client.startDataFrame(BayEOS_ChannelInt32le); 
    int value = sonar.range();  
    client.addChannelValue(++counter,1);
    if ((value <= 500) || (value > 4999)){
  	client.addChannelValue(value,3);
    } else {
         client.addChannelValue(value,2);
    } 
    last_data=myRTC.now().get();
    client.writeToBuffer();   
  }
  //Send data
  client.sendFromBuffer();


  delay(500);
}

