/****************************************************************
 * SHT21 Sleep Current Tests
 
 *
 * Sleep Current with ATMEGA328 + 2 EEPROMS + SHT21 was
 * 
 * 3.0µA
 ***************************************************************/

#include <Wire.h>
#include <Sleep.h>
#include <SHT2xSleep.h>

volatile uint8_t ticks;
SIGNAL(TIMER2_OVF_vect){
  ticks++;
}

void setup()
{
  Sleep.setupTimer2(2); //init timer2 to 0,0625sec -> 16 ticks per second
  Wire.begin();
  Serial.begin(9600);
  Serial.println("Starting");
  delay(20);
}

void loop()
{
  if(! ticks){
    unsigned long t=millis();
    float hum=SHT2x.GetHumidity();
    float temp=SHT2x.GetTemperature();
    t=millis()-t;
    
    Serial.print("Humidity(%RH): ");
    Serial.print(hum);
    Serial.print("     Temperature(C): ");
    Serial.println(temp);
    Serial.print("cputime: ");
    Serial.println(t);
    
    delay(200);
  }
  Sleep.sleep(TIMER2_ON,SLEEP_MODE_PWR_SAVE);
}

