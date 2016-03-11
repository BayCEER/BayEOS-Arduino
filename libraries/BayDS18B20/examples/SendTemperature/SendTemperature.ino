#include <OneWire.h>
#include <EEPROM.h>

#include <DS18B20.h>


#include <BayEOS.h>

#include <XBee.h>
#include <BayXBee.h>
#include <WString.h>

#define SWITCH_PIN 9





DS18B20 ds=DS18B20(10,0,4);
BayXBee bayeos_sender=BayXBee(Serial,5,15,1000); //Sleep-Request-Pin on 4, 15ms wakeup time

uint8_t startup=1;
const byte* new_addr;
uint8_t channel;
float temp;



void setup(void){
  bayeos_sender.begin(38400);
  pinMode(SWITCH_PIN, INPUT);
  digitalWrite(SWITCH_PIN,HIGH);
}

void loop(void){
  if(startup){
    startup=0;
    bayeos_sender.sendMessage(String("Arduino startup: Registered ")+
    String(ds.setAllAddrFromEEPROM(),10)+String(" Sensors from EEPROM"));
  }
  ds.t_conversion();
  delay(1000);
   //bayeos_sender.sendMessage(String("Reading sensors..."));
   bayeos_sender.startDataFrame(0x41);
   while(channel=ds.getNextChannel()){
       if(! ds.readChannel(channel,&temp)){
         bayeos_sender.addToPayload(channel);
         bayeos_sender.addToPayload(temp);
       } 
   }
   
// Search and Delete nur bei SWITCH_PIN auf Masse
  if(! digitalRead(SWITCH_PIN)){
    bayeos_sender.sendMessage(String("Searching for non responding sensors"));
    while(channel=ds.checkSensors()){
    new_addr=ds.getChannelAddress(channel);
     bayeos_sender.sendMessage(String("Channel ")+String(channel,10)+String(" with ROM ")+
    String(ds.addr2String(new_addr))+String(" is not responding. Deleting ..."));
    if(! ds.deleteChannel(new_addr)){
      bayeos_sender.sendError(String("Failed to delete Sensor on Channel ")+String(channel,10));
    }
  }
  
  
   bayeos_sender.sendMessage(String("Searching for new Sensors"));
  while(new_addr=ds.search()){
    if(channel=ds.getNextFreeChannel()){
      ds.addSensor(new_addr,channel);
      bayeos_sender.sendMessage(String("Registered new Sensor with ROM ")+
      String(ds.addr2String(new_addr))+String(" on channel ")+String(channel,10));
      } else {
        bayeos_sender.sendError(String("Failed to add new Sensor with ROM ")+
      String(ds.addr2String(new_addr))+String(": No channel left!! "));
         break;
      }
  }
  }
}
