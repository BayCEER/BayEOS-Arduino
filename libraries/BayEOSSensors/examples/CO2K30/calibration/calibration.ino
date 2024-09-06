#include <CO2K30.h>

CO2K30 sensor;
#define POWER_ON_PIN 7

void setup() {
  Serial.begin(9600);
  Serial.println("Starting K30 Sensor");
  Serial.print("Switching on PIN");
  Serial.println(POWER_ON_PIN);
  digitalWrite(SDA, HIGH);
  digitalWrite(SCL, HIGH);
  pinMode(POWER_ON_PIN, OUTPUT);
  digitalWrite(POWER_ON_PIN, HIGH);
  sensor.begin();

}

void loop() {

  Serial.print("CO2:");
  Serial.println(sensor.readCO2());
  Serial.println("Send Z to start zero - or B for background calibration");
  delay(3000);
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'Z') {
      Serial.println("Starting Zero Calibration");
      uint8_t res = sensor.zeroCalibration();
      Serial.print("res=");
      Serial.println(res);
    }
    if (c == 'B') {
      Serial.println("Starting Background Calibration");
      uint8_t res = sensor.backgroundCalibration();
      Serial.print("res=");
      Serial.println(res);

    }
  }

  }

