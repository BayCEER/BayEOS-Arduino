/****************************************************************
 * 
 * Sketch for simple and cheap weather station
 * 
 * with
 * air temperature - air moisture (SHT21)
 * rain gauge/wind count
  * soil temperature (Dallas)
 * Allows multiple Dallas on the same bus.
 * Search and delete is done in setup
 *
 * Sends Dallas values as separate frame due to payload length limitation (32 byte) of RF24
 *
 ***************************************************************/
#define DALLAS_PIN 4
#define POWER_PIN 7
#define LED_PIN 5

#define TICKS_PER_SECOND 16
#define RAINGAUGE_LAGTICKS 12
#define SAMPLING_INT 32
#define WITHDALLAS 1
#define WITHRAINGAUGE 1
//#define RF24ADDRESS 0x45c431ae12LL
//#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
//#define RF24ADDRESS 0x45c431ae96LL
//#define RF24ADDRESS 0x45c431aeabLL
#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x71

//Set this to 1 to get BayDebug Output!
#define SKETCH_DEBUG 0


#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <BayEOSBuffer.h>
#include <Wire.h>
#include <RTClib.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <Sleep.h>
#include <SHT2xSleep.h>
#include <BayEOS.h>

#if SKETCH_DEBUG
#include <BayDebug.h>
BayDebug client(Serial);
#else
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>
BayRF24 client=BayRF24(9,10);
#endif
BayEOSBufferEEPROM myBuffer;


//include some functions for low current board
//expects BayEOS-Client to be called "client"

#include <LowCurrentBoard.h>

float temp, hum;


void setup()
{
  
  Wire.begin();
#if SKETCH_DEBUG
  client.begin(9600,1);
#else
  client.init(RF24ADDRESS,RF24CHANNEL);
#endif
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer,100); //use skip!
  initLCB(); //init time2   
  readBatLCB(); 
  startLCB();
  
  
}

void loop()
{
  handleRtcLCB();
  
  #if WITHDALLAS
  //Do conversion 32 ticks (2sec) bevor sampling!
  if(ISSET_ACTION(0)){
    UNSET_ACTION(0);
    ds.t_conversion();
  }
  if(ISSET_ACTION(1)){
    UNSET_ACTION(1);
    readAndSendDallasLCB();
    readBatLCB();
  }
  #endif
  
  #if WITHRAINGAUGE
  handleRainEventLCB();
  #endif
  
  
  // Measure and send 
  if(ISSET_ACTION(2)){
    UNSET_ACTION(2);
    client.startDataFrame(BayEOS_ChannelFloat32le);
    client.addChannelValue(millis(),1);
    client.addChannelValue(batLCB,2);
    hum=SHT2x.GetHumidity();
    temp=SHT2x.GetTemperature();
    SHT2x.reset();
    client.addChannelValue(temp,3);
    client.addChannelValue(hum,4);
    #if WITHRAINGAUGE
    client.addChannelValue(rain_count,5);
    #endif
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();
   
  } 
  
  // Resend from Buffer
  if(ISSET_ACTION(7)){
    UNSET_ACTION(7);
    
    #if SERIALDEBUG
    Serial.println(myBuffer.available());
    delay(20);
    #endif
    
    client.sendFromBuffer();
  }
#if SKETCH_DEBUG
  delay(50);
#else
  sleepLCB();
#endif
}


