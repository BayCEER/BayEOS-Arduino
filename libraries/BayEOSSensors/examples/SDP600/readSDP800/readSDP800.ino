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

SDP800 sensor;

void setup(void){
  Serial.begin(38400);
  sensor.begin(); 
}

uint8_t last_loop;
void loop(void){
  while( (uint8_t)((uint8_t)millis()-last_loop)<20){} 
  // This is a sampling frequency of 1000/20=50.0000Hz
  // Tests with arduino mini pro clones showed that the frequency is not very accurate (got 48.6Hz)
  last_loop=millis();
  Serial.print(sensor.read(),4);
  Serial.print('\n');
}
