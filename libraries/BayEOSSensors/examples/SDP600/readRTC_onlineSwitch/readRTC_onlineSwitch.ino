/*******************************************************
   Sketch for SDP600-Type Sensors
   library default is SDP610-25Pa sensor
   use begin(scaleFactor) to change output

   The sketch uses a RTC-crystal connected to atmega328

   Wiring Sensor (bottom view):
   SDA (A4) - GND - 3.3V - SCL (A5)

 *******************************************************/
#include <SDP600.h>

volatile uint8_t ticks;
ISR(TIMER2_COMPA_vect) {
  TCNT2 = 0;
  ticks++;
}

SDP600 sensor;


void inline switchSensor(char c) {
  if(c<'2' || c>'5') return;
  switch (c) {
    case '5':
      sensor.setResolution(15); //Default resolution is 12
      OCR2A = 157; //-> 4096/(157+1)=25.92405Hz
      break;
    case '4':
      sensor.setResolution(14); //Default resolution is 12
      OCR2A = 78; //-> 4096/(78+1)=51.8481Hz
      break;
    case '3':
      sensor.setResolution(13); //Default resolution is 12
      OCR2A = 44; //-> 4096/(44+1)=91.0222Hz
      break;
    case '2':
      sensor.setResolution(12); //Default resolution is 12
      OCR2A = 26; //-> 4096/(26+1)=151.7037Hz
      break;
  }
  TCNT2 = 0;
}


void setup(void) {
  Serial.begin(38400);
  sensor.begin(); //Scale factor 1200 (1200 == 1 Pa)
  TCCR2A = 0x00;
  TCCR2B = 2; //-> 4096Hz
  ASSR = (1 << AS2); //Enable asynchronous operation
  TIMSK2 = (1 << OCF2A); //Enable timer2 compare_a interrupt
  switchSensor('4');
}

uint8_t last_ticks;
void loop(void) {
  while (ticks == last_ticks) {
    if(Serial.available()){
      switchSensor(Serial.read());
    }
  }
  last_ticks = ticks;
  Serial.print(sensor.read(), 4);
  Serial.print('\n');
}
