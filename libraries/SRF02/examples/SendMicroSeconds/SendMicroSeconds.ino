#include <Wire.h>
#include <SRF02.h>
#include <DS18B20.h>
#include <OneWire.h>
#include <EEPROM.h>
#include <XBee.h>
#include <BayXBee.h>
#include <BayEOS.h>


SRF02 srf=SRF02(0x70, SRF02_MICROSECONDS);
DS18B20 ds=DS18B20(10,0,1); 
BayXBee xbee=BayXBee();

float temp;

void setup()
{
 // open the serial port:
 xbee.begin(38400);
 ds.setAllAddrFromEEPROM(); 
}

void loop()
{   
 delay(9000);
 // Dallas 
 ds.setAllAddr();
 ds.t_conversion(); 
 delay(1000); 
 ds.readChannel(1,&temp);
   
 int msecs = srf.getDistance(); 
 float dist = ((msecs/2) * 331.5 * sqrt(1+(temp/273.15))) * pow(10,-4) ; 
 
 // Send packet
 xbee.startDataFrame(BayEOS_Float32le); 
 xbee.addToPayload((uint8_t) 0); //Offset 0             
 xbee.addToPayload(dist); 
 xbee.addToPayload((float)msecs); 
 xbee.addToPayload(temp); 
 xbee.sendPayload();
 
}

