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

volatile uint16_t ticks; //128 ticks per second
volatile uint8_t action;
#define TCONV_MASK 0x1
#define TSEND_MASK 0x2
#define SEND_MASK 0x4
#define RESEND_MASK 0x8
#define WINDSEND_MASK 0x10

RTC_Timer2 myRTC;
ISR(TIMER2_OVF_vect){
  ticks++;
  if((ticks % TICKS_PER_SECOND)==0){
    myRTC._seconds += 1; 
    uint8_t tick_mod = myRTC._seconds % SAMPLING_INT; 
    switch(tick_mod){
      case (1):
        action|=WINDSEND_MASK;
        break;
      case (SAMPLING_INT-2):
        action|=TCONV_MASK;
        break;
      case (SAMPLING_INT-1):
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
volatile float rain_count=0;
volatile uint16_t rain_event_ticks;
void rain_isr(void){
  if(! digitalRead(2)){
    if((ticks-rain_event_ticks)>RAINGAUGE_LAGTICKS){
      rain_count++;
      rain_event_ticks=ticks;
    }
  }
}
#endif

#if WITHWIND
volatile uint16_t wind_count=0;
volatile uint8_t wind_event=0;
volatile uint16_t wind_event_ticks;
volatile uint16_t min_wind_ticks=65535;
long windn=0;
long windo=0;
uint16_t wind_direction_count=0;
void wind_isr(void){
  if((ticks-wind_event_ticks)>4){
    if((ticks-wind_event_ticks)<min_wind_ticks)
      min_wind_ticks=ticks-wind_event_ticks;
    wind_count++;
    wind_event=1;
    wind_event_ticks=ticks;
  }
}
#endif



#if WITHDALLAS
uint8_t channel;
const byte* new_addr;

DS18B20 ds=DS18B20(A1,10,4); //Allow four sensors on the bus - channel 11-14
#endif


void setup()
{
  #if SERIALDEBUG
  Serial.begin(9600);
  Serial.println("Starting");
  delay(20);
  #endif
  
  Sleep.setupTimer2(1); //init timer2 to 128 ticks/s
  Wire.begin();
  client.init(RF24ADDRESS,RF24CHANNEL);
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer,100); //use skip!
  
  #if WITHRAINGAUGE
  digitalWrite(2,HIGH); //Enable Pullup on Pin 2 == INT0
  attachInterrupt(0,rain_isr,FALLING);
  rain_count=0;
  #endif
  
  #if WITHWIND
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH); //Enable Pullup on Pin 3 == INT1
  attachInterrupt(1,wind_isr,RISING);
  wind_count=0;
  wind_event=0;
  windn=0;
  windo=0;
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
  myRTC._seconds = SAMPLING_INT-4; //set RTC to a value to make sure that conversion is bevore sampling!
  #endif
  readVoltage();
}

void loop()
{
  //ISR-Stuff
 #if WITHWIND
 if(wind_event){
    wind_event=0;
    windDirection();
 }
 #endif

  //ACTIONS 
  if(action){
  #if WITHDALLAS
  //Do conversion (2sec) bevor sampling!
  if(action & TCONV_MASK){
    action&= ~TCONV_MASK;
    ds.t_conversion();
  }
  
  
  //Send dallas (1sec) bevor sampling!
  if(action & TSEND_MASK){
    action&= ~TSEND_MASK;
    
    client.startDataFrame(BayEOS_ChannelFloat32le);
    #if BUFFERDEBUG
    client.addChannelValue(myBuffer.writePos(),10);
    #endif
    while(channel=ds.getNextChannel()){
      if(! ds.readChannel(channel,&temp)){
         client.addChannelValue(temp,channel);
      }
    }
   client.sendOrBuffer();
   readVoltage();
  }
  
  #endif
  
 
 #if WITHWIND
 if(action & WINDSEND_MASK){
    action&= ~WINDSEND_MASK;
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
  if(action & SEND_MASK){
    action&= ~SEND_MASK;
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    client.addChannelValue(bat);
    hum=SHT2x.GetHumidity();
    temp=SHT2x.GetTemperature();
//    SHT2x.reset();
    client.addChannelValue(temp);
    client.addChannelValue(hum);
    #if WITHRAINGAUGE
    client.addChannelValue(rain_count);
    #endif
    client.sendOrBuffer();
    //Read battery voltage _after_ long uptime!!!
    readVoltage();
    #if SERIALDEBUG
    Serial.println("send");
    delay(20);
    #endif
   
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
  }
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE); 
}



void windDirection(){
  wind_direction_count++;
  pinMode(POWER_PIN,OUTPUT);
  digitalWrite(POWER_PIN,HIGH);
  int adc=analogRead(A2);
  digitalWrite(POWER_PIN,LOW);
  pinMode(POWER_PIN,INPUT);
  
  if(adc<552){
    windn+=7071;
    windo+=7071;
    return;
  }
  if(adc<602){
    windo+=7071;
    windn+=-7071;
    return;
  }
  if(adc<683){
    windo+=10000;
    return;
  }
  if(adc<761){
    windn+=7071;
    windo+=-7071;
    return;
  }
  if(adc<806){
    windn+=10000;
    return;
  }
  if(adc<857){
    windn+=-7071;
    windo+=-7071;
    return;
  }
  if(adc<916){
    windn+=-10000;
    return;
  }
  windo+=-10000;
  return; 
}


/* Read battery voltage */
void readVoltage(){
    analogReference(INTERNAL);
    pinMode(POWER_PIN,OUTPUT);
    digitalWrite(POWER_PIN,HIGH);
    bat=1.1*320/100/1023*analogRead(A0);
    digitalWrite(POWER_PIN,LOW);
    pinMode(POWER_PIN,INPUT);
    analogReference(DEFAULT);
}

