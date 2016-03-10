/*
  Send All Channels of SMTP-Shield

  Same as SendData but uses BayEOSBufferSDFat
  instead of BayEOSBufferSD
  Will decrease sketch size

   Stefan Holzheu
   30.01.2013

  channel 1+2 - CH-Status+U_LiPo
  channel 3+4 - uptime/cpu-time
  channel 5 - RTC-Temperature
  channel 6-11 - EC5
  channel 12 Kippwaage
  channel 13-14 Dallas (Currently one sensor per bus)

*/

#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <MCP342x.h>
#include <Wire.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>

#define CONNECTED_PIN 5
#define SAMPLING_INT 30

uint8_t connected = 0;

DS3231 myRTC; //Seduino 2.2


BaySerial client(Serial);
BayEOSBufferSDFat myBuffer;
BayEOSLogger myLogger;
/*
   Just a bunch of variables and functions to
   handle measurements on SMTPShield

   expects to have a bayeosClient called "client"
   and a RTC called "myRTC"
*/
#include <SMTPShield.h>


void setup() {

  Sleep.setupWatchdog(5); //init watchdog timer to 0.5 sec
  pinMode(CONNECTED_PIN, INPUT);
  Serial.end();

  initShield();
  //Set 4 for EthernetShield, 10 for Stalker
  if (!SD.begin(10)) {
    delay(10000);
    client.sendError("No SD!");
  }

  myBuffer = BayEOSBufferSDFat(2000000000, 1);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!

  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC);
}

void loop() {
  if (myLogger._mode == LOGGER_MODE_LIVE || (myRTC.now().get() - myLogger._last_measurement)
      >= myLogger._sampling_int) {
    measure();
    myLogger.liveData(1000);
    //This will only write Data, when _sampling_interval is reached...
    myLogger.logData();
  }


  //Entprellen des Interrupts
  handleKippevent();
  //wenn wdcount>240 -> reset!
  wdcount = 0;

  myLogger.handleCommand();
  myLogger.sendData();
  myLogger.sendBinaryDump();
  //sleep until watchdog will wake us up...
  if (! connected) {
    myLogger._mode = 0;
    Sleep.sleep();
  }

  //check if still connected
  //  if(! (myLogger._send_data || myLogger._live_data))
  if (connected)
    connected++;

  if (connected > 100) {
    Serial.flush();
    Serial.end();
    pinMode(1, INPUT);
    pinMode(2, INPUT);
    delay(10);
    connected = 0;
  }

  if (!connected && digitalRead(CONNECTED_PIN)) {
    connected = 1;
    client.begin(38400);
  }


}

