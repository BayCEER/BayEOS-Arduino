/*
  GBoardPro-Test
*/
#include <BayTCPSim900.h>
#include <BayEOSBufferSDFat.h>

#define SD_CSPIN 4
#define GPRS_PIN 46
#define GPRS_RESET 47
#define TX_SERIAL Serial2

BayGPRS client = BayGPRS(TX_SERIAL, GPRS_PIN, GPRS_RESET);


void setup(void) {
  Serial.begin(9600);
  Serial.println("Starting");
  while (!SD.begin(SD_CSPIN)) {
    Serial.println("No SD");
    delay(2000);
  }
  client.readConfigFromFile("GPRS.TXT");

  Serial.print("Starting GPRS error=");
  unsigned long start = millis();
  uint8_t res = client.begin(38400);
  Serial.println(res);
  Serial.print("runtime in ms: ");
  Serial.println(millis() - start);

  Serial.print("Sending Message error=");
  start = millis();
  res = client.sendMessage("GPRS TestMSG1");
  Serial.println(res);
  Serial.print("runtime in ms: ");
  Serial.println(millis() - start);

  Serial.println("Calling softReset");
  client.softReset();
  Serial.print("Sending Message error=");
  start = millis();
  res = client.sendMessage("GPRS TestMSG2");
  Serial.println(res);
  Serial.print("runtime in ms: ");
  Serial.println(millis() - start);

  Serial.println("Calling softSwitch");
  client.softSwitch();
  Serial.print("Sending Message error=");
  start = millis();
  res = client.sendMessage("GPRS TestMSG3");
  Serial.println(res);
  Serial.print("runtime in ms: ");
  Serial.println(millis() - start);

  Serial.println("Calling softReset + softSwitch");
  client.softReset();
  client.softSwitch();
  Serial.print("Sending Message error=");
  start = millis();
  res = client.sendMessage("GPRS TestMSG4");
  Serial.println(res);
  Serial.print("runtime in ms: ");
  Serial.println(millis() - start);

  Serial.println("Calling softSwitch + softReset");
  client.softSwitch();
  client.softReset();
  Serial.print("Sending Message error=");
  start = millis();
  res = client.sendMessage("GPRS TestMSG5");
  Serial.println(res);
  Serial.print("runtime in ms: ");
  Serial.println(millis() - start);

  Serial.println("sending 10 Messages");
  for (uint8_t i = 0; i < 10; i++) {
    Serial.print(i);
    Serial.print(". runtime in ms: ");
    start = millis();
    client.startFrame(BayEOS_Message);
    client.addToPayload("Test-Message NR");
    client.addToPayload('0' + i);
    res = client.sendPayload();
    Serial.println(millis() - start);


  }

  Serial.println("Test finished");
}

void loop(void) {

}



