/*
This sketch uses SERIAL1 (D18/D19) for logger communication 
and Serial for Debug

Only runs on Arduino Mega
*/


#include <EEPROM.h> 
#include <Wire.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>



RTC_Millis myRTC;
BaySerial client=BaySerial(Serial1); 
BayEOSBufferRAM myBuffer;
BayEOSLogger myLogger;


//Add your sensor measurements here!
void measure(){
   client.startDataFrame();
   client.addChannelValue(millis());
   client.addChannelValue(myBuffer.readPos());
   client.addChannelValue(myBuffer.writePos());
   client.addChannelValue(myBuffer.endPos());
   client.addChannelValue(myBuffer.available());
}



void setup() {
  myRTC.begin();
  client.begin(38400);
  Serial.begin(9600);
  myBuffer=BayEOSBufferRAM(300);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer); 
  //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC,2); //min_sampling_int = 2
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled=1; 
}

void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if(myLogger._logging_disabled && myRTC.now().get()>315360000L)
      myLogger._logging_disabled = 0;
   
   measure();
   myLogger.run();
 /* Serial.print(myBuffer.readPos());
  Serial.print("\t");
  Serial.print(myBuffer.writePos());
  Serial.print("\t");
  Serial.print(myBuffer.endPos());
  Serial.print("\t");
  Serial.println(myBuffer.available());
   if(! myLogger._mode)
     delay(1000);
 */
  
}

