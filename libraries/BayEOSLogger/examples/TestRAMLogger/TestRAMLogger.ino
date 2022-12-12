/*
 * Simple Logger Sketch using
 * RAM Buffer and Millis_RTC
 *  
 */


#include <BaySerial.h>
#include <BayEOSBufferRAM.h>
#include <BayEOSLogger.h>
#include <RTClib.h>


#define BAUD_RATE 38400

RTC_Millis myRTC;
BaySerial client = BaySerial(Serial);
BayEOSBufferRAM  myBuffer;
BayEOSLogger myLogger;


void setup() {
  client.begin(BAUD_RATE);

  myBuffer = BayEOSBufferRAM(1000);
  //register RTC in buffer
  myBuffer.setRTC(myRTC);

  //register Buffer in bayeos-client
  client.setBuffer(myBuffer);

  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC,30,3500); //min sampling 30, battery warning 3500 mV
}

void loop() {

  //store values in BayEOSClient
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis());
  client.addChannelValue(myBuffer.readPos());
  client.addChannelValue(myBuffer.writePos());
  client.addChannelValue(myBuffer.endPos());
  client.addChannelValue(myBuffer.available());
  myLogger._bat=3300; //Store current voltage values in _bat

  //if it is time to save the frame - BayEOSLogger will do it in run()
  myLogger.run();

}




