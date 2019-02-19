/*
 * Example sketch for MAX11212
 * 
 * The library relies on a RTC quarz connected to the MCU
 * 
 */


#include <MAX11212.h>
#include <Sleep.h>

uint8_t dout = A4;
uint8_t sck = A5;
MAX11212 adc(sck,dout);

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}

void setup(void)
{
  //  pinMode(10, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting ...");
  delay(20);
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  adc.begin();
}

void loop() {
  long v=adc.read();
  Serial.println(v);
  delay(1000);
}


