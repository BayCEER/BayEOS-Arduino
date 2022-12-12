/*******************************************************
   Sketch for SDP600-Type Sensors
   library default is SDP610-25Pa sensor
   use begin(scaleFactor) to change output

   Wiring Sensor (bottom view):
   SDA (A4) - GND - 3.3V - SCL (A5)

 *******************************************************/
#include <SDP600.h>

SDP600 sensor;
uint8_t w_time;
uint8_t last_loop;

void inline switchSensor(char c) {
  if (c < '2' || c > '5') return;
  switch (c) {
    case '5':
      sensor.setResolution(15); //Default resolution is 12
      w_time = 40; //-> 1000/40=25Hz
      break;
    case '4':
      sensor.setResolution(14); //Default resolution is 12
      w_time = 20; //-> 1000/20=50Hz
      break;
    case '3':
      sensor.setResolution(13); //Default resolution is 12
      w_time = 11; //-> 1000/11=90.9091Hz
      break;
    case '2':
      sensor.setResolution(12); //Default resolution is 12
      w_time = 7 ; //-> 1000/7=142.8571Hz
      break;
  }
}


void setup(void) {
  Serial.begin(38400);
  sensor.begin(); //Scale factor 1200 (1200 == 1 Pa)
  switchSensor('4');
}


void loop(void) {
  while ( (uint8_t) ( ((uint8_t)millis()) - last_loop) < w_time) {
    if (Serial.available()) {
      switchSensor(Serial.read());
    }
  }
  last_loop = millis();
  Serial.print(sensor.read(), 4);
  Serial.print('\n');
}
