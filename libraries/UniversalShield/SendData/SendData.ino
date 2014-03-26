/* 
Send All Channels of SMTP-Shield
   
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
#include <XBee.h>
#include <RTClib.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <BayXBee.h>
#include <BayDebug.h>
#include <Sleep.h>

#define XBEE_SLEEP_PIN 8
#define SAMPLING_INT 10

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
#include <UniversalShield.h>


void setup() {
  
  Sleep.setupWatchdog(6); //init watchdog timer to 1 sec
  pinMode(XBEE_SLEEP_PIN, OUTPUT);
  pinMode(10, OUTPUT);
   
  client.begin(19200); 
  initShield();
  delay(100);
  //Set 4 for EthernetShield, 10 for Stalker
  if (! SD.begin(10)) {
    client.sendError("No SD!");
  }
  
  myBuffer=BayEOSBufferSDFat(100000000);
  myBuffer.setRTC(myRTC,false); //Nutze RTC jedoch relativ!

  client.setBuffer(myBuffer);
  switch_off();
}

void loop() {
  if((myRTC.now().get()-last_data)>=SAMPLING_INT){
    switch_on();
	client.sendFromBuffer();
  	measure();
    //Send data or write to buffer 	  
	client.sendOrBuffer();
    switch_off();
  }


//Entprellen des Interrupts
  handleKippevent();
  //sleep until watchdog will wake us up...
  //Sleep.sleep();

  
}


