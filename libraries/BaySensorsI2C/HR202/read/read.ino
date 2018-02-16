/*
 * HR202 Sensor directly connected to Arduino ADC
 * with preresistor 100k (constant of library). 
 * A5 and A4 are used as output to switch on and off the sensor
 * and to revert the current in order to avoid polarization.
 * 
 * A5 --[ 100 k ] --A3-- [ HR202 ] --- A4
 * 
 * We need a additional temperature sensor (NTC) as
 * Humidity calibration is temperatur dependet.
 */


#include <NTC.h>
#include <HR202.h>


NTC_ADC ntc(A2,A1,20000.0,10.0);
HR202 hr(A5,A4,A3);

void setup(void){
  Serial.begin(9600);
}

void loop(void){
  unsigned long start=micros();
  float temp=ntc.getTemp();
  float hum=hr.getHumidity(temp);
  Serial.print(micros()-start);
  Serial.print("\t");
  Serial.print(temp);
  Serial.print("\t");
  Serial.println(hum);
  delay(1000);
  
}

