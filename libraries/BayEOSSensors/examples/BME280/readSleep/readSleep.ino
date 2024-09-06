#include <BayEOSBuffer.h>
#include <Wire.h>
#include <BME280.h>
#include <RTClib.h>
#include <Sleep.h>
#define SEALEVELPRESSURE_HPA (1013.25)

volatile uint16_t ticks; // 16 ticks per second
RTC_Timer2 myRTC;
ISR(TIMER2_OVF_vect)
{
  ticks++;
  if ((ticks % 16) == 0)
  {
    myRTC._seconds += 1;
  }
}

BME280 bme; // I2C

void setup()
{
  Sleep.setupTimer2(2); // init timer2 to 0,0625sec
  Serial.begin(9600);
  Serial.println(F("BME280 test"));

  if (!bme.begin(0x76))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
}

void loop()
{
  if ((ticks % 64) == 0)
  {
    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");

    Serial.print("Pressure = ");

    Serial.print(bme.readPressure() / 100.0F);
    Serial.println(" hPa");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");

    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    bme.triggerMeasurement();

    Serial.println();
    delay(200);
  }
  Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
}
