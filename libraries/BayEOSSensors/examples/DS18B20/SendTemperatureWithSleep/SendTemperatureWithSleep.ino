#include <OneWire.h>
#include <XBee.h>
#include <EEPROM.h>

#include <DS18B20.h>


#include <BayEOS.h>
#include <BayXBee.h>
#include <WString.h>

#include <Sleep.h>


#define OW_PIN 10
#define OW_PULLUP_PIN 11

#define XBEE_SLEEP_PIN 5



DS18B20 ds=DS18B20(OW_PIN,0,4);
//XBee-WakeUp-Time 15ms
//WaitTimeForResponse 500ms 
//Setting to zero does not work 
BayXBee xbee=BayXBee(Serial,XBEE_SLEEP_PIN,15,500);


const byte* new_addr;
uint8_t channel;
float temp;

void switch_off(void){
 pinMode(OW_PIN, INPUT);
 pinMode(OW_PULLUP_PIN, INPUT);
 digitalWrite(OW_PULLUP_PIN,LOW);
 pinMode(XBEE_SLEEP_PIN, INPUT);
 digitalWrite(XBEE_SLEEP_PIN,HIGH);
}


void switch_on(void){
 pinMode(OW_PULLUP_PIN, OUTPUT);
 digitalWrite(OW_PULLUP_PIN,HIGH);
 pinMode(XBEE_SLEEP_PIN, OUTPUT);
 digitalWrite(XBEE_SLEEP_PIN,HIGH);
}


//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
volatile boolean watchdog_flag=1;
ISR(WDT_vect) {
  watchdog_flag=1;  // set global flag
}
int watchdog_count=0;


void setup(void){
  Sleep.setupWatchdog(9); //init watchdog timer to 8 sec
  xbee.begin(38400);
//  Serial.println("starting");
  switch_on();
  delay(5000);
  ds.setAllAddrFromEEPROM();
  ds.setAllAddr(); //Sucht nach neuen Sensoren und registriert diese
}

void loop(void){
  if(watchdog_flag){
    watchdog_count++;
    watchdog_flag=0;
//    Serial.println("watchdog_flag ");
  }
  if(watchdog_count==2){
//  Serial.println("watchdog_count ");
    watchdog_count=0;
    switch_on();
    delay(50);
    ds.t_conversion();
    delay(700);
    xbee.startDataFrame(0x41);
    while(channel=ds.getNextChannel()){
       if(! ds.readChannel(channel,&temp)){
         xbee.addToPayload(channel);
         xbee.addToPayload(temp);
       }
     }
   xbee.sendPayload();
  }
  switch_off();
  Sleep.sleep();
}
