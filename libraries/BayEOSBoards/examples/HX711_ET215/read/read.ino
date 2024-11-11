/*
 * Example Sketch for BayEOS-HX711-Board
 * 
 * The Board has two HX711 modules. 
 * 
 * 
 */

#include <HX711Array.h>
#include <NTC.h>
#include <Sleep.h>
uint8_t dout[] = {6, 4};
uint8_t sck = 3;
long adc[2];

HX711Array scale;
NTC_HX711 ntc(scale, A3, 2*470000, 3.0); //Adjust resistor values

volatile uint8_t tics;
ISR(TIMER2_OVF_vect) {
  tics++;
}


void setup(void) {
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  Serial.begin(9600);
  Serial.println("Starting ...");
  Serial.flush();
  scale.begin(dout, 2, sck); //start HX711Array with 2 ADCs
}


void loop(void) {
  delay(1000);
  ntc.readResistance();
  float temp0 = ntc.getTemp(0);
  float temp1 = ntc.getTemp(1);

  scale.power_up();
  scale.read(); //dummy read after reading the temperature
  scale.read_average(adc);
  scale.power_down();
  
  Serial.print("ADC0: ");
  Serial.print(temp0);
  Serial.print("\t");
  Serial.print(temp1);
  Serial.print("\t");
  Serial.print(adc[0]);
  Serial.print("\t");
  Serial.println(adc[1]);
  Serial.flush();

}


