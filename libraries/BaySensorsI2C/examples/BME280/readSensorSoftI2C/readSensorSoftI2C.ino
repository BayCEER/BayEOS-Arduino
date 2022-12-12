#include <Wire.h>
#include <BME280SoftI2C.h>


#define SEALEVELPRESSURE_HPA (1013.25)

BME280SoftI2C bme(A4,A5); //SDA + SCL 
//in this example identical to hardware I2C. Could be any arduino pin

void setup() {
   delay(1000);
 Serial.begin(9600);
  Serial.println(F("BME280 test"));

  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
}

void loop() {
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
    delay(2000);
}
