/*******************************************************
 * Test-Sketch for SDP600-Type Sensors
 * library default is SDP610-25Pa sensor
 * use begin(scaleFactor) to change output
 * 
 * Wiring Sensor (bottom view):
 * SDA (A4) - GND - 3.3V - SCL (A5)
 * 
 *******************************************************/
#include <SDP600.h>

SDP600 sensor;

void setup(void){
  Serial.begin(38400);
  sensor.begin(); //Scale factor 1200 (1200 == 1 Pa)
  sensor.setResolution(14); //Default resolution is 12
}

unsigned long last_loop;
void loop(void){
  while((millis()-last_loop)<20){}
  last_loop=millis();
  Serial.print(" ");
  Serial.print(sensor.read(),4);
  Serial.println(" ");
}
