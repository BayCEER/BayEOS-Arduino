#include <OneWire.h>
#include <EEPROM.h>
#include <DS18B20.h>
#include <MCP3422.h>
#include <Wire.h>

/* Print All Channels of Board AHMT
   Oliver Archner
   26.02.2012
   Channel 1: Photosynthetic Photon Flux Density (PPFD)
   */

const int pinMois = A0;    
const int pinTemp = A1;    

float mois ;  
float temp ;
float adc ;
float lPar;

char str_buf[50];

MCP3422 mcp3422 = MCP3422();

void setup() {
  Serial.begin(9600);
}

void loop() {
  delay(2*1000); 

  Serial.println("-----------------------");  

  // Licor 
  mcp3422.setConf(1,0,1,3,3);
  delay(100);
  adc = mcp3422.getData();
  Serial.print("LI-190 PAR(V):"); 
  dtostrf(adc, 12, 8, str_buf);  
  Serial.println(str_buf);
  
  mcp3422.setConf(1,0,1,3,0);
  delay(100);
  adc = mcp3422.getData();
  Serial.print("LI-190 PAR(V):"); 
  dtostrf(adc, 12, 8, str_buf);  
  Serial.println(str_buf);



}
