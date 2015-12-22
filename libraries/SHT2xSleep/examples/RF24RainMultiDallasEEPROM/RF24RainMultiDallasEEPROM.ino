/****************************************************************
 * 
 * Sketch for simple and cheap weather station
 * 
 * with
 * air temperature - air moisture (SHT21)
 * rain gauge
 * soil temperature (Dallas)
 * Allows multiple Dallas on the same bus.
 * Search and delete is done in setup
 *
 * Sends Dallas values as separate frame due to payload length limitation (32 byte) of RF24
 * 
 * Wiring:
 * Dallas: DATAPIN -> A1, GND-> GND, VCC -> VCC, 4,7k Pullup
 * Rain Gauge: INT0 == D2
 *
 ***************************************************************/

// 16 ticks per second!
#define RAINGAUGE_LAGTICKS 12
#define SAMPLING_INTTICKS 512
#define WITHDALLAS 1
#define WITHRAINGAUGE 1
#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
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

volatile uint16_t ticks; //16 ticks per second
volatile uint8_t action;
#define TCONV_MASK 0x1
#define TSEND_MASK 0x2
#define SEND_MASK 0x4
#define RESEND_MASK 0x8

RTC_Timer2 myRTC;
ISR(TIMER2_OVF_vect){
  ticks++;
  if((ticks % 16)==0){
    myRTC._seconds += 1; 
    int tick_mod=ticks%SAMPLING_INTTICKS;
    switch(tick_mod){
      case (SAMPLING_INTTICKS-32):
        action|=TCONV_MASK;
        break;
      case (SAMPLING_INTTICKS-16):
        action|=TSEND_MASK;
        break;
      case 0:
        action|=SEND_MASK;
        break;
      default:
        action|=RESEND_MASK;
    }
  }
}
BayRF24 client=BayRF24(9,10);
BayEOSBufferEEPROM myBuffer;

float temp, hum, bat;


#if WITHRAINGAUGE
float rain_count=0;
volatile uint8_t rain_event=0;
volatile uint16_t rain_event_ticks;
void rain_isr(void){
  rain_event=1;
  rain_event_ticks=ticks;
}
#endif

#if WITHDALLAS
uint8_t channel;
const byte* new_addr;

DS18B20 ds=DS18B20(A1,10,4); //Allow four sensors on the bus - channel 11-14
#endif

/* Read battery voltage */
void readVoltage(){
    analogReference(INTERNAL);
    pinMode(A3,OUTPUT);
    digitalWrite(A3,HIGH);
    bat=1.1*320/100/1023*analogRead(A0);
    digitalWrite(A3,LOW);
    pinMode(A3,INPUT);
    analogReference(DEFAULT);
}

void setup()
{
  #if SERIALDEBUG
  Serial.begin(9600);
  Serial.println("Starting");
  delay(20);
  #endif
  
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  Wire.begin();
  client.init(RF24ADDRESS,RF24CHANNEL);
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer,100); //use skip!
  
  #if WITHRAINGAUGE
  digitalWrite(2,HIGH); //Enable Pullup on Pin 2 == INT0
  attachInterrupt(0,rain_isr,FALLING);
  rain_count=0;
  rain_event=0;
  #endif
  
  #if WITHDALLAS
  ds.setAllAddrFromEEPROM();
  // Search and Delete
  while(channel=ds.checkSensors()){
    new_addr=ds.getChannelAddress(channel);
    client.sendMessage(String("DS:")+channel+"-"+ds.addr2String(new_addr));
    ds.deleteChannel(new_addr);
  }
  while(new_addr=ds.search()){
    if(channel=ds.getNextFreeChannel()){
      ds.addSensor(new_addr,channel);
      client.sendMessage(String("DS:")+channel+"+"+ds.addr2String(new_addr));
    }
  }
  ticks=SAMPLING_INTTICKS-34; //set ticks to a value to make sure that conversion is bevore sampling!
  #endif
  readVoltage();
}

void loop()
{
  #if WITHDALLAS
  //Do conversion 32 ticks (2sec) bevor sampling!
  if(action & TCONV_MASK){
    action&= ~TCONV_MASK;
    ds.t_conversion();
   #if SERIALDEBUG
   Serial.println("t-conv");
  delay(20);
  #endif
  }
  
  
  //Send dallas temps 16 ticks (1sec) bevor sampling!
  if(action & TSEND_MASK){
    action&= ~TSEND_MASK;
    
    client.startDataFrame(BayEOS_ChannelFloat32le);
    #if BUFFERDEBUG
    client.addChannelValue(myBuffer.writePos(),10);
    #endif
    while(channel=ds.getNextChannel()){
     #if SERIALDEBUG
   Serial.println(channel);
  delay(20);
  #endif
      if(! ds.readChannel(channel,&temp)){
         client.addChannelValue(temp,channel);
    #if SERIALDEBUG
   Serial.println(temp);
  delay(20);
  #endif
      }
    }
     #if SERIALDEBUG
   Serial.println("send t");
  delay(20);
  #endif
   client.sendOrBuffer();
   readVoltage();
  }
  
  #endif
  
  #if WITHRAINGAUGE
  if(rain_event){
      detachInterrupt(0);
  }
  if(rain_event && ((ticks-rain_event_ticks)>RAINGAUGE_LAGTICKS)){
    attachInterrupt(0,rain_isr,FALLING);
    rain_count++;
    rain_event=0;
    #if SERIALDEBUG
   Serial.println("rain");
  delay(20);
  #endif
 }
  #endif
  
  
  // Measure and send 
  if(action & SEND_MASK){
    action&= ~SEND_MASK;
    client.startDataFrame(BayEOS_ChannelFloat32le);
    client.addChannelValue(millis(),1);
    client.addChannelValue(bat,2);
    hum=SHT2x.GetHumidity();
    temp=SHT2x.GetTemperature();
    SHT2x.reset();
    client.addChannelValue(temp,3);
    client.addChannelValue(hum,4);
    #if WITHRAINGAUGE
    client.addChannelValue(rain_count,5);
    #endif
    client.sendOrBuffer();
    //Read battery voltage _after_ long uptime!!!
    readVoltage();
   
  } 
  
  // Resend from Buffer
  if(action & RESEND_MASK){
    action&= ~RESEND_MASK;
    
    #if SERIALDEBUG
    Serial.println(myBuffer.available());
    delay(20);
    #endif
    
    client.sendFromBuffer();
  }

  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}


