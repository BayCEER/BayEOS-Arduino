/*
   Test-Sketch from ADC-Board

*/
#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayDebug.h>
BayDebug client(Serial);

#define ACTION_COUNT 1
#define SAMPLING_INT 4
#include <LowCurrentBoard.h>

void setup()
{
  pinMode(POWER_PIN, OUTPUT);
  client.begin(9600);
  Serial.println("Starting Test");
  myBuffer.init(flash); //This will restore old pointers
  Serial.print("Flash: ");
  Serial.println(flash.getCapacity());
  Serial.flush();
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); //use skip!
  initLCB(); //init time2
  Serial.println("Test done");
  Serial.flush();
  startLCB();

}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement

    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
 
    client.addChannelValue(analogRead(A0));
    client.addChannelValue(analogRead(A1));
    client.addChannelValue(analogRead(A2));
    client.addChannelValue(analogRead(A3));
    client.addChannelValue(analogRead(A4));
    client.addChannelValue(analogRead(A5));
    sendOrBufferLCB();

    Serial.flush();
  }
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
    Serial.flush();
  }
  sleepLCB();

}



