#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <SRF02.h>
#include <Wire.h>

DS18B20 ds=DS18B20(10,0,1); //Pin 10, Offest 0, Kapazität 1 Sensor
float temp;
int msecs;

SRF02 srf = SRF02(0x70, SRF02_MICROSECONDS);

void setup(void){
  Serial.begin(9600);
  ds.setAllAddrFromEEPROM();
}


void loop(void){
    ds.setAllAddr(); 
    ds.t_conversion(); 
    delay(750); 
    if(! ds.readChannel(1,&temp)){
	   Serial.print("Temperature:");
       Serial.print(temp);
       Serial.println(" °");     
       msecs = srf.getDistance();
       if (msecs > 0) {         
        float dist = ((msecs/2) * 331.5 * sqrt(1+(temp/273.15))) * pow(10,-4) ;  
        Serial.print("Distance:");
        Serial.print(dist);
        Serial.println("cm");       
       } else {
          Serial.println("SRF02-Lesen fehlgeschlagen!");
       }
    } else {
      Serial.println("DS10B20-Lesen fehlgeschlagen!");
    }
}
