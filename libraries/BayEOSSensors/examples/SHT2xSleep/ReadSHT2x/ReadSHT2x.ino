/****************************************************************
 * ReadSHT2x
 *  An example sketch that reads the sensor and prints the
 *  relative humidity to the PC's serial port
 *
 ***************************************************************/

#include <Wire.h>
#include <Sleep.h>
#include <SHT2xSleep.h>

ISR(TIMER2_OVF_vect)
{
}

void setup()
{
  Sleep.setupTimer2(2); // init timer2 to 0,0625sec
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Starting");
  delay(20);
}

void loop()
{
  unsigned long t = millis();
  float hum = SHT2x.GetHumidity();
  float temp = SHT2x.GetTemperature();
  SHT2x.reset();
  t = millis() - t;

  Serial.print("Humidity(%RH): ");
  Serial.print(hum);
  Serial.print("     Temperature(C): ");
  Serial.println(temp);
  Serial.print("cputime: ");
  Serial.println(t);

  delay(1000);
}
