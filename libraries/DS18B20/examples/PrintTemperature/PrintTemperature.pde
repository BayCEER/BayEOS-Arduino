#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>



/*
Create a DS18B20 instance on pin 10, offset 0 and a
capacity of 4 Sensors
*/
DS18B20 ds=DS18B20(10,0,4);



uint8_t startup=1;
const byte* new_addr;
uint8_t channel;
float temp;



void setup(void){
  Serial.begin(9600);
}

void loop(void){
  if(startup){
    startup=0;
    Serial.print("Arduino startup: DS18B20 Channels: ");
    Serial.print(ds.getNumberOfChannels(),DEC);
    Serial.print(" - registered ");
    Serial.print(ds.setAllAddrFromEEPROM(),DEC);
    Serial.print(" Sensors from EEPROM");
    Serial.println();
  }
  ds.t_conversion();
  delay(1000);
   Serial.print("Reading sensors...");
   Serial.println();
   while(channel=ds.getNextChannel()){
       if(! ds.readChannel(channel,&temp)){
         Serial.print("Channel ");
         Serial.print(channel,DEC);
         Serial.print(": ");
         Serial.print(temp);
         Serial.print("C");
         Serial.println();
       }
   }
   Serial.println();
   
// Search and Delete

    Serial.print("Searching for non responding sensors");
    Serial.println();

    while(channel=ds.checkSensors()){
      new_addr=ds.getChannelAddress(channel);

      Serial.print("Channel ");
      Serial.print(channel,DEC);
      Serial.print(" with ROM ");
      Serial.print(ds.addr2String(new_addr));
      Serial.print(" is not responding. Deleting ...");
      Serial.println();
      if(! ds.deleteChannel(new_addr)){
        Serial.print("Failed to delete Sensor on Channel ");
        Serial.print(channel, DEC);
        Serial.println();
     }
   }
  
  Serial.print("Searching for new Sensors");
  Serial.println();
  while(new_addr=ds.search()){
    if(channel=ds.getNextFreeChannel()){
      ds.addSensor(new_addr,channel);
      Serial.print("Registered new Sensor with ROM ");
      Serial.print(ds.addr2String(new_addr));
      Serial.print(" on channel ");
      Serial.print(channel,DEC);
      Serial.println();
      } else {
        Serial.print("Failed to add new Sensor with ROM ");
        Serial.print(ds.addr2String(new_addr));
        Serial.print(": No channel left!!");
        Serial.println();
        break;
      }
  }
}

