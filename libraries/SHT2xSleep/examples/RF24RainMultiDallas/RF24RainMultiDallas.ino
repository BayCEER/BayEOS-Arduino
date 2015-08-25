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
#define RF24ADDRESS 0x45c431aeabLL
#define RF24CHANNEL 0x71
#define SERIALDEBUG 0

#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <Wire.h>
#include <Sleep.h>
#include <SHT2xSleep.h>
#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayRF24.h>

volatile uint16_t ticks; //16 ticks per second
ISR(TIMER2_OVF_vect){
  ticks++;
}
BayRF24 client=BayRF24(9,10);
float temp, hum;

#if WITHRAINGAUGE
float rain_count=0;
volatile uint8_t rain_event=0;
volatile uint16_t rain_event_ticks;
void rain_isr(void){
  rain_event=1;
  rain_event_ticks=ticks;
  detachInterrupt(0);
}
#endif

#if WITHDALLAS
uint8_t channel;
const byte* new_addr;

DS18B20 ds=DS18B20(A1,10,4); //Allow four sensors on the bus - channel 11-14
#endif

/* Read battery voltage */
void startFrameReadVoltage(uint8_t mode=0){
  client.startDataFrame(BayEOS_ChannelFloat32le);
  if(mode & 0x1){
    client.addChannelValue(millis(),1);
  }
  if(mode & 0x2){
    analogReference(INTERNAL);
    pinMode(A3,OUTPUT);
    digitalWrite(A3,HIGH);
    client.addChannelValue(1.1*320/100/1023*analogRead(A0),2);
    digitalWrite(A3,LOW);
    pinMode(A3,INPUT);
    analogReference(DEFAULT);
  }
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
  
  #if WITHRAINGAUGE
  digitalWrite(2,HIGH); //Enable Pullup on Pin 2 == INT0
  attachInterrupt(0,rain_isr,LOW);
  #endif
  
  #if WITHDALLAS
  ds.setAllAddrFromEEPROM();
  ticks-=33; //set ticks to a value to make sure that conversion is bevore sampling!
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
  #endif
  startFrameReadVoltage(0x2);

}

void loop()
{
  #if WITHDALLAS
  //Do conversion 32 ticks (2sec) bevor sampling!
  if((ticks%SAMPLING_INTTICKS)==(SAMPLING_INTTICKS-32)){
    ds.t_conversion();
   #if SERIALDEBUG
   Serial.println("t-conv");
  delay(20);
  #endif
  }
  
  
  //Send dallas temps 16 ticks (1sec) bevor sampling!
  if((ticks%SAMPLING_INTTICKS)==(SAMPLING_INTTICKS-16)){
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
   client.sendPayload();
    
    startFrameReadVoltage(0x1);
  }
  
  #endif
  
  #if WITHRAINGAUGE
  if(rain_event && ((ticks-rain_event_ticks)>RAINGAUGE_LAGTICKS)){
    rain_count++;
    rain_event=0;
    attachInterrupt(0,rain_isr,LOW);
    #if SERIALDEBUG
   Serial.println("rain");
  delay(20);
  #endif
 }
  #endif
  
  
  if((ticks%SAMPLING_INTTICKS)==0){ 
    hum=SHT2x.GetHumidity();
    temp=SHT2x.GetTemperature();
    client.addChannelValue(temp,3);
    client.addChannelValue(hum,4);
    #if WITHRAINGAUGE
    client.addChannelValue(rain_count,5);
    #endif
    client.sendPayload();
    //Read battery voltage _after_ long uptime!!!
    startFrameReadVoltage(0x2);
   
  }  
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}


