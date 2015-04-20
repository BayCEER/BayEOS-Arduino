/**************************************************

Example for Seduino Stalker with SD-Card
All data is written to SD-card before sending via XBee
Please set the clock and initialize the card with the following sketch:
https://github.com/BayCEER/BayEOS-Arduino/blob/master/libraries/SMTPShield/SetStalker/SetStalker.ino

**************************************************/


#include <Wire.h>
#include <PWD11.h>
#include <SoftwareSerial.h>
#include <XBee.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <BayXBee.h>

#define SAMPLING_INT 60

unsigned long last_data;
DS3231 myRTC; //Seduino 2.2
BayEOSBufferSDFat myBuffer;
BayXBee client=BayXBee(); 
PWD11 pwd11(8,9);


void setup(){
  Wire.begin();
  myRTC.begin();
  pwd11.begin();
  client.begin(38400);
  pinMode(10,OUTPUT);
    
  while(!SD.begin(10)) {
    delay(10000);
    client.sendError("No SD!");
    delay(10000);
  }
  myBuffer=BayEOSBufferSDFat(200000000,1);
  myBuffer.setRTC(myRTC,false); 
  client.setBuffer(myBuffer); 
}

int counter = 0;

void loop() {
  if((myRTC.now().get()-last_data)>=SAMPLING_INT){
    client.startDataFrame(BayEOS_WithoutOffsetFloat32le); 	
	float* value;
	value = pwd11.readMessage2();	
	for(int i=0;i<10;i++){
	  client.addToPayload(value[i]);
    }      
    last_data=myRTC.now().get();
    client.writeToBuffer();   
  }
  //Send data
  client.sendFromBuffer();
  delay(500);
}
