/******************************************************

   Sample Logger Sketch for Seeduino 2.2 with SD-Card

 *****************************************************/
#include <RTClib.h>
#include <BayEOSBufferSDFat.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>

#define CONNECTED_PIN 5
#define SAMPLING_INT 30

uint8_t connected = 0;

DS3231 myRTC; //Seduino 2.2

BaySerial client(Serial); //Note giving the Serial Object is mandatory!!
BayEOSBufferSDFat myBuffer;
BayEOSLogger myLogger;


//Add your sensor measurements here!
void measure() {
  client.startDataFrame(BayEOS_Int16le);
  client.addChannelValue(millis());
}



void setup() {
  Sleep.setupWatchdog(5); //init watchdog timer to 0.5 sec
  pinMode(CONNECTED_PIN, INPUT);
  digitalWrite(CONNECTED_PIN, HIGH);
  myBuffer = BayEOSBufferSDFat(200000000, 1);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60); //min_sampling_int = 60
  //disable logging as RTC has to be set first!!
  myBuffer = BayEOSBufferSDFat(200000000, 1);
}

void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.now().get() > 315360000L)
    myLogger._logging_disabled = 0;

  measure();
  myLogger.run();


  //sleep until timer2 will wake us up...
  if (! connected) {
    myLogger._mode = 0;
    Sleep.sleep();
  }

  //check if still connected
  if (connected)
    connected++;

  if (connected > 100 && digitalRead(CONNECTED_PIN)) {
    client.flush();
    client.end();
    connected = 0;
  }

  //Connected pin is pulled to GND
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    client.begin(38400);
  }

}

