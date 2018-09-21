
/*
   Sketch for SapFlow Board

   Run as Logger
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
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>


#include <BaySerial.h>
#include <BayEOSLogger.h>

BaySerial client(Serial);
BayEOSLogger myLogger;
#define BAUD_RATE 38400
#define CONNECTED_PIN 9
uint8_t connected = 0;



//Configuration
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
#define MCP_POWER_PIN A3
#define HEAT_PIN 6

MCP342x mcp342x = MCP342x();

SPIFlash flash(8); //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer; //BayEOS Buffer
//Configure your resistors on the board!

float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}
#include <LowCurrentBoard.h>


float t_ref, t_heat, bat_voltage;
int heat_rate;
float iTerm, pTerm, dTerm, error, last_error;
unsigned long last_reg;

float values[9];
int count;

void measure(void) {
  unsigned long dt = myRTC.now().get() - last_reg;
  if (dt < DELTA_T) return;
  last_reg = myRTC.now().get();
  digitalWrite(MCP_POWER_PIN, HIGH);
  delay(2);
  if (count == 0) {
    for (uint8_t i = 0; i < 9; i++) {
      values[i] = 0;
    }
  }
  count++;
  mcp342x.runADC(0);
  delay(mcp342x.getADCTime());
  float I = mcp342x.getData() / (float) R[0];

  mcp342x.runADC(1);
  delay(mcp342x.getADCTime());
  float R_mess = mcp342x.getData() / I;
  t_ref = ntc10_R2T(R_mess / NTC10FACTOR_REF);

  mcp342x.runADC(2);
  delay(mcp342x.getADCTime());
  I = mcp342x.getData() / (float) R[1];

  mcp342x.runADC(3);
  delay(mcp342x.getADCTime());
  R_mess = mcp342x.getData() / I;

  t_heat = ntc10_R2T(R_mess / NTC10FACTOR_HEAT) - t1_t2_offset;
  digitalWrite(MCP_POWER_PIN, LOW);

  //PID-regulation
  error = -((t_heat - t_ref) - TARGET_DT);
  iTerm += Ki * error * dt;
  if (iTerm < 0) iTerm = 0;
  if (iTerm > 255) iTerm = 255;

  pTerm = Kp * error;
  dTerm = Kd * (error - last_error) / dt;
  last_error = error;
  if (TARGET_DT > 0)
    heat_rate = pTerm + iTerm + dTerm;
  else
    heat_rate = 0;

  if (heat_rate > 255) heat_rate = 255;
  if (heat_rate < 0) heat_rate = 0;

  analogWrite(HEAT_PIN, heat_rate);

  digitalWrite(POWER_PIN, HIGH);
  analogReference(DEFAULT);
  bat_voltage = 3.3 * BAT_DIVIDER / 1023 * analogRead(A7);
  myLogger._bat = bat_voltage * 1000;
  digitalWrite(POWER_PIN, LOW);
  values[0] += bat_voltage;
  values[1] += t_ref;
  values[2] += t_heat;
  values[3] += error;
  values[4] += heat_rate;
  values[5] += bat_voltage * bat_voltage / HEAT_RESISTANCE * heat_rate / 255.0 ;
  values[6] += pTerm;
  values[7] += iTerm;
  values[8] += dTerm;

  client.startDataFrame(0x41);
  client.addChannelValue(values[0] / count, 1);
  client.addChannelValue(values[1] / count, 2);
  client.addChannelValue(values[2] / count, 3);
  client.addChannelValue(values[3] / count, 4); //error
  client.addChannelValue(values[4] / count / 255 * 100, 5); //heat rate
  client.addChannelValue(values[5] / count * 1000, 6); //heat rate in mW
  client.addChannelValue(values[6] / count, 7);
  client.addChannelValue(values[7] / count, 8);
  client.addChannelValue(values[8] / count, 9);
}

void setup()
{
  initLCB();
  pinMode(POWER_PIN, OUTPUT);
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  Wire.begin();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  client.begin(BAUD_RATE);
  myBuffer.init(flash); //This will restore old pointers
  myBuffer.setRTC(myRTC, 1); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  myLogger.init(client, myBuffer, myRTC, 30, 3500); //min sampling 30, battery warning 2500 mV
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myLogger._logging_disabled = 1;
}

void loop() {
  measure();
  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;
  myLogger.run();
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
  }
  if (! connected && myLogger._logging_disabled) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(800);
  }

  if (! connected) {
    myLogger._mode = 0;
  }
  //check if still connected
  if (connected && digitalRead(CONNECTED_PIN)) {
    client.flush();
    client.end();
  }
  //Connected pin is pulled to GND
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    adjust_OSCCAL();
    client.begin(38400);
  }

}

