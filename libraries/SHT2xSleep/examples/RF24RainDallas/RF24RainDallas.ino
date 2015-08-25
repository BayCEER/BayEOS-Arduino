/****************************************************************
 * 
 * Sketch for simple and cheap weather station
 * 
 * with
 * air temperature - air moisture (SHT21)
 * rain gauge
 * soil temperature (Dallas)
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
#define RF24ADDRESS 0x45c431ae48LL
#define RF24CHANNEL 0x71

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
DS18B20 ds=DS18B20(A1,0,1); //Allow only one sensor on the bus
#endif

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec
  Wire.begin();
  client.init(RF24ADDRESS,RF24CHANNEL);
  startFrameReadVoltage();
  
  #if WITHRAINGAUGE
  digitalWrite(2,HIGH); //Enable Pullup on Pin 2 == INT0
  attachInterrupt(0,rain_isr,LOW);
  #endif
  
  #if WITHDALLAS
  ds.setAllAddr();
  ticks-=17; //set ticks to a value to make sure that conversion is bevore sampling!
  #endif

}

void loop()
{
  #if WITHDALLAS
  //Do conversion 16 ticks (1sec) bevor sampling!
  if((ticks%SAMPLING_INTTICKS)==(SAMPLING_INTTICKS-16)){
    ds.t_conversion();
  }
  #endif
  
  #if WITHRAINGAUGE
  if(rain_event && ((ticks-rain_event_ticks)>RAINGAUGE_LAGTICKS)){
    rain_count++;
    rain_event=0;
    attachInterrupt(0,rain_isr,LOW);
  }
  #endif
  if((ticks%SAMPLING_INTTICKS)==0){ //
    unsigned long t=millis();
    float hum=SHT2x.GetHumidity();
    float temp=SHT2x.GetTemperature();
    client.addChannelValue(temp);
    client.addChannelValue(hum);
    //Read battery voltage _after_ long uptime!!!
    #if WITHRAINGAUGE
    client.addChannelValue(rain_count);
    #endif
    
    #if WITHDALLAS
    while(channel=ds.getNextChannel()){
       if(! ds.readChannel(channel,&temp))
         client.addChannelValue(temp);
    }
    #endif
    client.sendPayload();
    startFrameReadVoltage();
  }  
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}


/* Read battery voltage */
void startFrameReadVoltage(void){
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis());
  analogReference(INTERNAL);
  pinMode(A3,OUTPUT);
  digitalWrite(A3,HIGH);
  client.addChannelValue(1.1*320/100/1023*analogRead(A0));
  digitalWrite(A3,LOW);
  pinMode(A3,INPUT);
  analogReference(DEFAULT);
}
