/*
This is a sketch for a BayEOS-Logger device
receiving data over XBee, logging to SD and 
communicating to a logger software via SoftSerial


ATTENTION!
This file will only complile replacing the original 
 arduino-1.0/hardware/arduino/cores/arduino/HardwareSerial.cpp
 arduino-1.0/hardware/arduino/cores/arduino/HardwareSerial.h 
by the version distributed with the BayEOS arduino libraries

 
*/

#include <XBee.h>
#include <BayXBee.h>
#include <BayEOS.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <BaySoftwareSerial.h>
#include <BayEOSLogger.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <Wire.h>
#include <RTClib.h>
#include <EEPROM.h>

#define RX_SERIAL Serial


/* Use SoftwareSerial to read out data... */
BaySoftwareSerial client=BaySoftwareSerial(8,9);
DS3231 myRTC; //Seduino 2.2
BayEOSLogger myLogger; 

//BayDebug client;
XBee xbee_rx = XBee();
BayEOSBufferSDFat myBuffer;
Rx16Response rx16 = Rx16Response();


uint16_t rx_panid;
	
/*
 * Create a huge ring buffer to store incoming RX Packages while
 * arduino is busy with GPRS...
 */
#define RX_BUFFER_SIZE 192
unsigned char buffer[RX_BUFFER_SIZE];

unsigned long last_alive;

void setup(void){
   Wire.begin();
   myRTC.begin();
  pinMode(10, OUTPUT);
  
 
  
  client.begin(9600);
 //Use 10 for Stalker
  if (!SD.begin(10)) {
  	client.sendError("No SD.");
  }
  myBuffer = BayEOSBufferSDFat(200000000,1);
  //register RTC in buffer
  myBuffer.setRTC(myRTC);

  //register Buffer in bayeos-client
  client.setBuffer(myBuffer);

  //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC);

 //Replace the RX ring buffer with the larger one...
  RX_SERIAL.setRxBuffer(buffer,RX_BUFFER_SIZE); 
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);

 /* while(! rx_panid){
    rx_panid=getPANID(xbee_rx);
  }
  */
  client.sendMessage("Setup ok");
   
}

void loop(void){
    if(millis()-last_alive>5000){
      last_alive=millis();
      client.sendMessage("up");
      client.startDataFrame(BayEOS_Float32le);
      client.addChannelValue(millis()/1000);
      client.writeToBuffer();
    }
    xbee_rx.readPacket();
   
    if (xbee_rx.getResponse().isAvailable()) {
      // got something
      if (xbee_rx.getResponse().getApiId() == RX_16_RESPONSE ) {
        // got a rx16 packet
        xbee_rx.getResponse().getRx16Response(rx16);
        if (xbee_rx.getResponse().isError()) {
		// get the error code
			client.startFrame(BayEOS_ErrorMessage);
			client.addToPayload("RX-ERROR:"+
(xbee_rx.getResponse().getErrorCode()+'0'));
	    } else {
   //       client.startRoutedFrame(rx16.getRemoteAddress16(),rx_panid,rx16.getRssi());
         client.startRoutedFrame(rx16.getRemoteAddress16(),rx_panid);
     	  for(uint8_t i=0; i<rx16.getDataLength();i++){
			client.addToPayload(rx16.getData(i));
		  }
		}
		client.writeToBuffer();
      }
    }
     
     //Handle Command 
     myLogger.handleCommand();
     //Send Data over SoftwareSerial
     myLogger.sendData();
 
}
