/****************************************************************
 * 
 * Sketch for simple and cheap weather station
 * based on FROGGIT5300
 * 
 * with
 * air temperature - air moisture (SHT21)
 * rain gauge
 * wind direction
 * wind speed
 * soil temperature (Dallas)
 * Allows multiple Dallas on the same bus.
 * Search and delete is done in setup
 *
 * Sends Dallas values as separate frame due to payload length limitation (32 byte) of RF24
 * 
 * Wiring:
 * Dallas: DATAPIN -> A1, GND-> GND, VCC -> VCC, 4,7k Pullup
 * Rain Gauge: INT0 == D2
 * WINDDirection: A2 with 10k Divider to A3
 * WINDSpeed:  INT1 == D3
 *
 ***************************************************************/



// 128= ticks per second!
#define TICKS_PER_SECOND 128
#define RAINGAUGE_LAGTICKS 64
#define POWER_PIN A3
#define SAMPLING_INT 30
#define WITHDALLAS 0
#define WITHRAINGAUGE 1
#define WITHWIND 1
//#define RF24ADDRESS 0x45c431aeabLL
#define RF24ADDRESS 0x45c431ae12LL
#define RF24CHANNEL 0x61
#define SERIALDEBUG 0
#define BUFFERDEBUG 0

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
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>

BayRF24 client=BayRF24(9,10);
BayEOSBufferEEPROM myBuffer;
//include some functions for low current board
//expects BayEOS-Client to be called "client"
#include <LowCurrentBoard.h>


float temp, hum;

void setup()
{
  Wire.begin();
  client.init(RF24ADDRESS,RF24CHANNEL);
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer,100); //use skip!
  
  initLCB(); //init time2   
  readBatLCB(); 
  startLCB();
}

void loop()
{
  //ISR-Stuff
 #if WITHWIND
 if(wind_event){
    wind_event=0;
    readWindDirectionLCB();
 }
 #endif
  
  #if WITHRAINGAUGE
  handleRainEventLCB();
  #endif


  //ACTIONS 
  if(action){
  //Do conversion (2sec) bevor sampling!
  if(ISSET_ACTION(0)){
    UNSET_ACTION(0);
    SHT2x.reset();
  #if WITHDALLAS
    ds.t_conversion();
  #endif
  }
  
  #if WITHDALLAS 
  //Send dallas (1sec) bevor sampling!
  if(ISSET_ACTION(1)){
    UNSET_ACTION(1);
    readAndSendDallasLCB();
    readBatLCB();
  }
  
  #endif
  
 
 #if WITHWIND
  if(ISSET_ACTION(4)){
    UNSET_ACTION(4);
  #if SERIALDEBUG
    Serial.println('w');
    Serial.println(((float)wind_count)/SAMPLING_INT);
    delay(10);
  #endif
    client.startDataFrame(BayEOS_ChannelFloat32le);
    client.addChannelValue(((float)wind_count)/SAMPLING_INT,31);
    client.addChannelValue((wind_count?((float)TICKS_PER_SECOND)/min_wind_ticks:0),32);
    if(wind_direction_count){
      client.addChannelValue(((float)windn)/wind_direction_count/10000,33);
      client.addChannelValue(((float)windo)/wind_direction_count/10000,34);
    }
    client.sendOrBuffer();
    wind_count=0;
    min_wind_ticks=65535;
    wind_direction_count=0;
    windn=0;
    windo=0;
    
    
 }
 #endif
  
 
  // Measure and send 
  if(ISSET_ACTION(2)){
    UNSET_ACTION(2);
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB);
    hum=SHT2x.GetHumidity();
    temp=SHT2x.GetTemperature();
//    SHT2x.reset();
    client.addChannelValue(temp);
    client.addChannelValue(hum);
    #if WITHRAINGAUGE
    client.addChannelValue(rain_count);
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
  }
  sleepLCB();
}

