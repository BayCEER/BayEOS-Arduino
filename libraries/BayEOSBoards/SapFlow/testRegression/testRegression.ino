/*
 * Test Sketch for SapFlow Shield and pulsed heating 
 */


//*********************************************
// User Configruation
//
// Resistance of heating coil
#define HEAT_RESISTANCE 63.67
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR 0.5
//Configure your resistors on the board!
const uint16_t R[] = { 14300, 14300 };
//END user configruation
//**********************************************

#include <Wire.h>
#include <MCP342x.h>
#include <math.h>
#include <BayEOSBuffer.h>
#include <BayDebug.h>


const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode

#define MCP_POWER_PIN 3
#define HEAT_PIN 6

MCP342x mcp342x = MCP342x();

BayDebug client(Serial);

float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}
#include <LowCurrentBoard.h>


float t_ref, t_heat;
float heat_rate;

void set_heat_and_measure(uint8_t heat_rate, uint16_t wait) {
  analogWrite(HEAT_PIN, heat_rate);
  delay(wait);
  digitalWrite(MCP_POWER_PIN, HIGH);
  delay(2);
  mcp342x.setConf(addr, 1, 0, mode, rate, gain);
  delay(120);
  float I = mcp342x.getData(addr) / (float) R[0];
  mcp342x.setConf(addr, 1, 1, mode, rate, gain);
  delay(120);
  float R_mess = mcp342x.getData(addr) / I;
  t_ref = ntc10_R2T(R_mess/NTC10FACTOR);

  mcp342x.setConf(addr, 1, 2, mode, rate, gain);
  delay(120);
  I = mcp342x.getData(addr) / (float) R[1];
  mcp342x.setConf(addr, 1, 3, mode, rate, gain);
  delay(120);
  R_mess = mcp342x.getData(addr) / I;
  t_heat = ntc10_R2T(R_mess/NTC10FACTOR);
  digitalWrite(MCP_POWER_PIN, LOW);

  float vbat=3.3 * 5.7 / 1023 * analogRead(A1);
  Serial.print(vbat);
  Serial.print("\t");
  Serial.print(t_ref*10);
  Serial.print("\t");
  Serial.print(t_heat*10);
  Serial.print("\n");
}

void setup()
{
  initLCB();
  client.begin(9600);
  Wire.begin();
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  startLCB();
//  Serial.print("Bat\tRef\tHeated\tError\tRate\tPower\tpTerm\tiTerm\tdTerm\n");
  delay(1000);
}

void loop() {
  for(uint8_t j=0;j<255;j++){
    Serial.print((float)millis()/1000);
    Serial.print("\t");
    Serial.print(j);
    Serial.print("\t");
    delay(100);
    digitalWrite(5, HIGH);
    set_heat_and_measure(j,1000);
    digitalWrite(5, LOW);
    delay(900);
  }

    for(uint8_t j=0;j<255;j++){
    Serial.print((float)millis()/1000);
    Serial.print("\t");
    Serial.print(0);
    Serial.print("\t");
    delay(100);
    digitalWrite(5, HIGH);
    set_heat_and_measure(0,1000);
    digitalWrite(5, LOW);
    delay(900);
  }

}

