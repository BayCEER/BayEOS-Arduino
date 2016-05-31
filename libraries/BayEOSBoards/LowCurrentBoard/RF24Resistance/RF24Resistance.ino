/****************************************************************
 * 
 * Sketch for read out of resistance sensors
 * 
 * 
 *
 ***************************************************************/
#define DALLAS_PIN 4
#define POWER_PIN 7
#define POWER_GND_PIN 8
#define DIVIDER_RESISTANCE 3.3
#define LED_PIN 5

#define TICKS_PER_SECOND 16
#define RAINGAUGE_LAGTICKS 12
#define SAMPLING_INT 32
#define WITHDALLAS 0
#define WITHRAINGAUGE 0
#define NRF24_PIPE 0
#define RF24CHANNEL 0x66

#if NRF24_PIPE == 0
#define RF24ADDRESS 0x45c431ae12LL
#elif NRF24_PIPE == 1
#define RF24ADDRESS 0x45c431ae24LL
#elif NRF24_PIPE == 2
#define RF24ADDRESS 0x45c431ae48LL
#elif NRF24_PIPE == 3
#define RF24ADDRESS 0x45c431ae96LL
#elif NRF24_PIPE == 4
#define RF24ADDRESS 0x45c431aeabLL
#elif NRF24_PIPE == 5
#define RF24ADDRESS 0x45c431aebfLL
#endif

//Set this to 1 to get BayDebug Output!
#define SKETCH_DEBUG 1


#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <BayEOSBuffer.h>
#include <Wire.h>
#include <RTClib.h>
#include <BayEOSBufferRAM.h>
#include <Sleep.h>
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
BayEOSBufferRAM myBuffer;

float temp;
//include some functions for low current board
//expects BayEOS-Client to be called "client"

#include <LowCurrentBoard.h>


void setup()
{
  
  Wire.begin();
#if SKETCH_DEBUG
  client.begin(9600,1);
#else
  client.init(RF24ADDRESS,RF24CHANNEL);
#endif
  myBuffer = BayEOSBufferRAM(1000);
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer,20); //use skip!
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
    client.startDataFrame(BayEOS_Int16le);
    client.addChannelValue(millis());
    client.addChannelValue(1000*batLCB);
    pinMode(POWER_GND_PIN,OUTPUT);
    pinMode(POWER_PIN,OUTPUT);
    digitalWrite(POWER_GND_PIN,0);
    digitalWrite(POWER_PIN,1);
    unsigned long start=millis();
    for(uint8_t i=0;i<4;i++){
      client.addChannelValue(analogRead(A1+i));
    }
    //reverse voltage to avoid polarization
    digitalWrite(POWER_GND_PIN,1);
    digitalWrite(POWER_PIN,0);
    delay(millis()-start);
    digitalWrite(POWER_GND_PIN,0);
    pinMode(POWER_GND_PIN,INPUT);
    
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
#if SKETCH_DEBUG
  delay(50);
#else
  sleepLCB();
#endif
}


