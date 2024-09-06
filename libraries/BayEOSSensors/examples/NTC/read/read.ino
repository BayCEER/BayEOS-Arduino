/*
 * NTC 10k Sensor directly connected to Arduino ADC
 * with preresistor 20k.
 * A2 is used as output to switch on and off the sensor
 *
 * A2 --[ 20 k ] --A1-- [ NTC ] --- GND
 */

#include <NTC.h>

NTC_ADC ntc(A2, A1, 20000.0, 10.0);

void setup(void)
{
  Serial.begin(9600);
}

void loop(void)
{
  unsigned long start = micros();
  float temp = ntc.getTemp();
  Serial.print(micros() - start);
  Serial.print("\t");
  Serial.println(temp);
  delay(1000);
}
