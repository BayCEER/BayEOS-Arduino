#include <CO2K33.h>

CO2K33 sensor;

void setup() {
  Serial.begin(9600);
  sensor.begin();
  digitalWrite(SDA,HIGH);
  digitalWrite(SCL,HIGH);
  /*
   * Performe a Backgroud Calibration on Setup
   * Assumes that sensor is exposed to 400ppm CO2
   */
  Serial.println("Starting Background Calibration");
  uint8_t res=sensor.backgroundCalibration();
  Serial.print("res=");
  Serial.println(res);
  
  res=sensor.triggerMeasurement();
  Serial.print("res=");
  Serial.println(res);
  delay(25000);
}

void loop() {
  sensor.triggerMeasurement();
  delay(25000);
  
  Serial.print("CO2:");
  Serial.println(sensor.readCO2());
  Serial.print("TEMP:");
  Serial.println(sensor.readTemperature());
  Serial.print("HUM:");
  Serial.println(sensor.readHumidity());
  Serial.println();
  delay(5000);
  

}

