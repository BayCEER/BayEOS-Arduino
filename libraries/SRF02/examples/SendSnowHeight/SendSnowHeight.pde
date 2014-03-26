#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <SRF02.h>
#include <Wire.h>
#include <XBee.h>
#include <BayEOS.h>
#include <BayXBee.h>

DS18B20 ds=DS18B20(10,0,1); //Pin 10, Offest 0, Kapazität 1 Sensor
float temp;
int msecs;

SRF02 srf = SRF02(0x70, SRF02_MICROSECONDS);

BayXBee xbee=BayXBee();


void setup(void){
  xbee.begin(38400); 
  ds.setAllAddrFromEEPROM();
}


void loop(void){
    ds.setAllAddr(); //Sucht nach neuen Sensoren und registriert diese
    ds.t_conversion(); //Befehl zur T-Umwandlung
    delay(1000); //Warten...
    if(! ds.readChannel(1,&temp)){
       msecs = srf.getDistance();
       if (msecs > 0) {         
       float dist = ((msecs/2) * 331.5 * sqrt(1+(temp/273.15))) * pow(10,-4) ;  
       xbee.startDataFrame(BayEOS_Float32le); 
       xbee.addToPayload((uint8_t) 0); //Offset 0 
       xbee.addToPayload(dist); 
       xbee.sendPayload();
       } else {
          xbee.sendError("SRF02-Lesen fehlgeschlagen!");
       }
    } else {
      xbee.sendError("DS10B20-Lesen fehlgeschlagen!");
    }
    delay(9000);
}
