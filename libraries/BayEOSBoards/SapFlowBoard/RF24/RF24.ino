
/*
   Sketch for SapFlow Shield

   Run as Logger or RF24
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
#define NTC10FACTOR 0.5
//Some constants for reguation
//decrease if regulation reacts to fast
const float Kp = 80; // Einheit: 255/C
const float Ki = 1; // Einheit: 255/C*S
const float Kd = 100; // Einheit: 255/C/s
//Configure your resistors on the board!
const uint16_t R[] = { 14300, 14300 };
const float t1_t2_offset = 0;

// 470k - 100k Divider + Default Reference
#define BAT_MULTIPLIER 3.3*570/100/1023

#define RF24ADDRESS 0x45c431ae24LL
#define RF24CHANNEL 0x33
#define CHECKSUM_FRAMES 1
#define SAMPLING_INT 32

//END user configruation
//**********************************************


#include <Wire.h>
#include <MCP342x.h>
#include <math.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BayRF24.h>
unsigned long last_sent;

BayRF24 client(9, 10);


//Configuration
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 2; //0-3: 12bit ... 18bit
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


float t_ref, t_heat;
int heat_rate;
float iTerm, pTerm, dTerm, error, last_error;
unsigned long last_reg;

float values[9];
int count;

void measure(void) {
  int dt = myRTC.now().get() - last_reg;
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
  mcp342x.setConf(addr, 1, 0, mode, rate, gain);
  delay(100);
  float I = mcp342x.getData(addr) / (float) R[0];
  mcp342x.setConf(addr, 1, 1, mode, rate, gain);
  delay(100);
  float R_mess = mcp342x.getData(addr) / I;
  t_ref = ntc10_R2T(R_mess / NTC10FACTOR);

  mcp342x.setConf(addr, 1, 2, mode, rate, gain);
  delay(100);
  I = mcp342x.getData(addr) / (float) R[1];
  mcp342x.setConf(addr, 1, 3, mode, rate, gain);
  delay(100);
  R_mess = mcp342x.getData(addr) / I;
  t_heat = ntc10_R2T(R_mess / NTC10FACTOR) - t1_t2_offset;
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
  values[0] += BAT_MULTIPLIER * analogRead(A7);
  digitalWrite(POWER_PIN, LOW);
  values[1] += t_ref;
  values[2] += t_heat;
  values[3] += error;
  values[4] += heat_rate;
  values[5] += values[0] / 1000 / count * values[0] / 1000 / count / HEAT_RESISTANCE * values[4] / 255.0 / count;
  values[6] += pTerm;
  values[7] += iTerm;
  values[8] += dTerm;

  client.startDataFrame(BayEOS_Int16le, CHECKSUM_FRAMES);
  client.addChannelValue(values[0] / count);
  client.addChannelValue(values[1] / count * 100);
  client.addChannelValue(values[2] / count * 100);
  client.addChannelValue(values[3] / count * 1000); //error
  client.addChannelValue(values[4] / count * 100); //heat rate
  client.addChannelValue(values[5] / count * 1000); //heat rate in W
  client.addChannelValue(values[6] / count * 100);
  client.addChannelValue(values[7] / count * 100);
  client.addChannelValue(values[8] / count * 100);
#if CHECKSUM_FRAMES
  client.addChecksum();
#endif
}

void setup()
{
  initLCB();
  Wire.begin();
  pinMode(POWER_PIN, OUTPUT);
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  startLCB();
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  myBuffer.skip(); //This will skip unsent frames
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 100); //use skip!
}

void loop() {
  measure();
  if (ISSET_ACTION(0) && count > 0) {
    UNSET_ACTION(0);
    sendOrBufferLCB();
    count = 0;
  }
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
}

