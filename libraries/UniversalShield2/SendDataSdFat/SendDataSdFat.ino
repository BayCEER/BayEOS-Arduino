/* 
Send All Channels of UNIVERSAL-Shield2
   
*/

#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <MCP342x.h>
#include <Wire.h>
#include <XBee.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <BayXBee.h>
#include <BayDebug.h>
#include <Sleep.h>

#define XBEE_SLEEP_PIN 5
#define SAMPLING_INT 5

DS3231 myRTC; //Seduino 2.2


//BayXBee client=BayXBee(XBEE_SLEEP_PIN,15,1000); //Sleep-Pin - Wakeuptime, timeout
BayDebug client=BayDebug(); 
BayEOSBufferSDFat myBuffer;

/*
 * Just a bunch of variables and functions to
 * handle measurements on SMTPShield
 *
 * expects to have a bayeosClient called "client"
 * and a RTC called "myRTC"
 */
#include <UniversalShield2.h>


void setup() {
  
  Sleep.setupWatchdog(5); //init watchdog timer to 0.5 sec
  pinMode(XBEE_SLEEP_PIN, OUTPUT);
   
  client.begin(38400,1); 
  initShield();
  //Set 4 for EthernetShield, 10 for Stalker
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    delay(10000);
    client.sendError("No SD!");
  }
  
  myBuffer=BayEOSBufferSDFat(200000000,1);
  myBuffer.setRTC(myRTC,false); //Nutze RTC jedoch relativ!

  client.setBuffer(myBuffer,100); //max skip=100!!
  //Switch on Reference Voltage
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  digitalWrite(3,HIGH);
  digitalWrite(4,HIGH);

}

void loop() {
  if((myRTC.now().get()-last_data)>=SAMPLING_INT){
     measure();
     client.writeToBuffer();	  
  }
  //Send data
  client.sendFromBuffer();

//wenn wdcount>240 -> reset!
  wdcount=0;

//Entprellen des Interrupts
  handleKippevent();
  //sleep until watchdog will wake us up...
  //Sleep.sleep();

  
}

