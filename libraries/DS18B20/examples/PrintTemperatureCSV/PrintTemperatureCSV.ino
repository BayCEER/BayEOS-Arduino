/*
Example for simple use of DS18B20-Class for csv-Output
*/

#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>

DS18B20 ds=DS18B20(10,0,5); //Pin 10, Offset 0, Capacity 5 sensors
float temperature;  
uint8_t channel;

void setup(void){
  Serial.begin(9600);
  ds.setAllAddrFromEEPROM();
}


void loop(void){
    ds.setAllAddr(); //Search for new or removed sensors
    ds.t_conversion(); //Start T-conversion 
    delay(750); //wait until T-conversion is finished
    channel=1;
    while(channel<=5){
      if(! ds.readChannel(channel,&temperature)){
	     Serial.print(temperature);
	     Serial.print(";");
	  } else
	     Serial.print("NA;"); //No Sensor present on this Channel
         channel++;
    }
    Serial.println();
}