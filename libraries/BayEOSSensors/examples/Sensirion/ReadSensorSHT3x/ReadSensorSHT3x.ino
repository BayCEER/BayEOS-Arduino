#include <SHT3x.h>

#define WITH_SLEEP 1

// SDA, SCL
SHT3x sht(A4, A5);

float temperature;
float humidity;
int ret;

ISR(TIMER2_OVF_vect)
{
}

void setup()
{
  // put your setup code here, to run once:
  Sleep.setupTimer2(2); // init timer2

  Serial.begin(9600);
  Serial.println("started");
  Serial.flush();
}

void loop()
{
  // put your main code here, to run repeatedly:
  long start = millis();
#if WITH_SLEEP
  ret = sht.measureSleep(&temperature, &humidity);
#else
  ret = sht.measure(&temperature, &humidity);
#endif

  start = millis() - start;
  if (!ret)
  {
    Serial.print("CPU-Time = ");
    Serial.print(millis());
    Serial.print("ms, ");
    Serial.print("Call = ");
    Serial.print(start);
    Serial.print("ms, ");
    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.print("oC, Humidity = ");
    Serial.print(humidity);
    Serial.println("%");
  }
  else
  {
    Serial.print("Error: ");
    Serial.println(ret);
  }
  Serial.flush();
  // Sleep for 2 seconds
  for (uint8_t i = 0; i < 64; i++)
  {
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
  }
}
