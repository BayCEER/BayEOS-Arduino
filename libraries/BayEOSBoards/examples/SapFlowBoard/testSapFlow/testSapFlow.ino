/*
 * Test Sketch for SapFlow Shield 
 */


//*********************************************
// User Configruation
//
// T-Difference between heated and non-heated [°C]
#define TARGET_DT 3.0
// Regulation time step [s]
#define DELTA_T 3
// Resistance of heating coil
#define HEAT_RESISTANCE 63.67
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR_REF 0.5
#define NTC10FACTOR_HEAT 1
// Divider resistors for battery voltage
#define BAT_DIVIDER (470.0+100.0)/100.0
//Some constants for reguation
//decrease if regulation reacts to fast
const float Kp = 80; // Einheit: 255/C
const float Ki = 1; // Einheit: 255/C*S
const float Kd = 100; // Einheit: 255/C/s
//Configure your resistors on the board!
const uint16_t R[] = { 14300, 14300 };
const float t1_t2_offset = 0;

//resolution ADC for temperature measurement
const uint8_t rate = 2; //0-3: 12bit ... 18bit

//END user configruation
//**********************************************

#include <Wire.h>
#include <MCP342x.h>
#include <math.h>
#include <BayEOSBuffer.h>
#include <BayDebug.h>


const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode

#define MCP_POWER_PIN A3
#define HEAT_PIN 6

MCP342x mcp342x(addr);

BayDebug client(Serial);
float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}
#include <LowCurrentBoard.h>


float t_ref, t_heat;
float heat_rate;
float iTerm, pTerm, dTerm, error, last_error;
unsigned long last_reg;

void measure(void) {
  int dt = myRTC.now().get() - last_reg;
  if (dt < DELTA_T) return;
  last_reg = myRTC.now().get();
  digitalWrite(MCP_POWER_PIN, HIGH);
  delay(2);
  mcp342x.runADC(0);
  delay(mcp342x.getADCTime());
  float I = mcp342x.getData() / (float) R[0];
  
  mcp342x.runADC(1);
  delay(mcp342x.getADCTime());
  float R_mess = mcp342x.getData() / I;
  t_ref = ntc10_R2T(R_mess/NTC10FACTOR_REF);

  mcp342x.runADC(2);
  delay(mcp342x.getADCTime());
  I = mcp342x.getData() / (float) R[1];
  
  mcp342x.runADC(3);
  delay(mcp342x.getADCTime());
  R_mess = mcp342x.getData() / I;
  t_heat = ntc10_R2T(R_mess/NTC10FACTOR_HEAT);
  digitalWrite(MCP_POWER_PIN, LOW);

  //PID-regulation
  error = -((t_heat - t_ref) - TARGET_DT);
  iTerm += Ki * error * dt;
  if (iTerm < 0) iTerm = 0;
  if (iTerm > 255) iTerm = 255;

  pTerm = Kp * error;
  dTerm = Kd * (error - last_error) / dt;
  last_error = error;
  heat_rate = pTerm + iTerm + dTerm;
  if (heat_rate > 255) heat_rate = 255;
  if (heat_rate < 0) heat_rate = 0;
  analogWrite(HEAT_PIN, heat_rate);

  pinMode(POWER_PIN,OUTPUT);
  digitalWrite(POWER_PIN,HIGH);
  float vbat=3.3 * BAT_DIVIDER / 1023 * analogRead(A7);
  digitalWrite(POWER_PIN,LOW);

  Serial.print(vbat);
  Serial.print("\t");
  Serial.print(t_ref);
  Serial.print("\t");
  Serial.print(t_heat);
  Serial.print("\t");
  Serial.print(error);
  Serial.print("\t");
  Serial.print(100.0*heat_rate/255);
  Serial.print("\t");
  Serial.print(vbat  * vbat / HEAT_RESISTANCE * heat_rate / 255.0);
  Serial.print("\t");
  Serial.print(pTerm);
  Serial.print("\t");
  Serial.print(iTerm);
  Serial.print("\t");
  Serial.print(dTerm);
  Serial.print("\n");
}

void setup()
{
  initLCB();
  client.begin(9600);
  Wire.begin();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  startLCB();
  Serial.print("Bat\tRef\tHeated\tError\tRate\tPower\tpTerm\tiTerm\tdTerm\n");
  delay(1000);
}

void loop() {
  digitalWrite(5, HIGH);
  measure();
  digitalWrite(5, LOW);
  delay(2000);
}

