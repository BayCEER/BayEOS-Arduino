/*******************************************************
 * Test-Sketch for SDP600-Type Sensors
 * library default is SDP610-25Pa sensor
 * use begin(scaleFactor) to change output
 * 
 * The sketch uses a RTC-crystal connected to atmega328 
 * to have a constant frequency 128/3=42.6666666 Hz
 * 
 * 
 * Wiring Sensor (bottom view):
 * SDA (A4) - GND - 3.3V - SCL (A5)
 * 
 *******************************************************/
#include <SDP600.h>
#include <Sleep.h>

volatile uint8_t ticks;
ISR(TIMER2_OVF_vect) {
  ticks++;
}


SDP600 sensor;

void setup(void){
  Sleep.setupTimer2(1); //128 ticks/sec 
  Serial.begin(38400);
  sensor.begin(); //Scale factor 1200 (1200 == 1 Pa)
  sensor.setResolution(14); //Default resolution is 12
}

uint8_t last_ticks;
void loop(void){
  while((uint8_t)(ticks-last_ticks)<(uint8_t)3){}
  last_ticks=ticks;
  Serial.print(sensor.read(),4);
  Serial.print('\n');
}
