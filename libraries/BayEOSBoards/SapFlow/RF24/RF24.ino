#include <Wire.h>
#include <MCP342x.h>
#include <math.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>

#define SEND_RF24 1

#if SEND_RF24

#include <BayRF24.h>
#define NRF24_PIPE 0
#define RF24CHANNEL 0x33
#define CHECKSUM_FRAMES 1
#define SAMPLING_INT 32
unsigned long last_sent;

#if NRF24_PIPE == 0
#define RF24ADDRESS 0x45c431ae12LL
#elif NRF24_PIPE == 1
#define RF24ADDRESS 0x45c431ae24LL
#elif NRF24_PIPE == 2
#define RF24ADDRESS 0x45c431ae48LL
#elif NRF24_PIPE == 3
#define RF24ADDRESS 0x45c431ae96LL
#elif NRF24_PIPE == 4
#define RF24ADDRESS 0x45c431aeabLL
#elif NRF24_PIPE == 5
#define RF24ADDRESS 0x45c431aebfLL
#endif

BayRF24 client(9, 10);


#else

#include <EEPROM.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>

BaySerial client(Serial);
BayEOSLogger myLogger;
#define BAUD_RATE 38400
#define CONNECTED_PIN 9
uint8_t connected = 0;

#endif


//Configuration
const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
#define MCP_POWER_PIN 3
#define HEAT_PIN 6
#define TARGET_DT 3.0
#define HEAT_RESISTANCE 63.67
float Kp = 80; // Einheit: 255/C
float Ki = 1; // Einheit: 255/C*S
float Kd = 100; // Einheit: 255/C/s
const uint16_t R[] = { 14293, 14296 };
const float t1_t2_offset = 0;

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
#define DELTA_T 3

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
  blinkLED(1);
  mcp342x.setConf(addr, 1, 0, mode, rate, gain);
  delay(100);
  float I = mcp342x.getData(addr) / (float) R[0];
  mcp342x.setConf(addr, 1, 1, mode, rate, gain);
  delay(100);
  float R_mess = mcp342x.getData(addr) / I;
  t_ref = ntc10_R2T(R_mess);

  mcp342x.setConf(addr, 1, 2, mode, rate, gain);
  delay(100);
  I = mcp342x.getData(addr) / (float) R[1];
  mcp342x.setConf(addr, 1, 3, mode, rate, gain);
  delay(100);
  R_mess = mcp342x.getData(addr) / I;
  t_heat = ntc10_R2T(R_mess) - t1_t2_offset;
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

  analogReference(DEFAULT);
  values[0] += 3300.0 * 5.7 / 1023 * analogRead(A1);
  values[1] += t_ref;
  values[2] += t_heat;
  values[3] += error;
  values[4] += heat_rate;
  values[5] += values[0] / 1000 / count * values[0] / 1000 / count / HEAT_RESISTANCE * values[4] / 255.0 / count;
  values[6] += pTerm;
  values[7] += iTerm;
  values[8] += dTerm;

#if SEND_RF24
  client.startDataFrame(BayEOS_Int16le, CHECKSUM_FRAMES);
#else
  client.startDataFrame();
#endif
  client.addChannelValue(values[0] / count);
  client.addChannelValue(values[1] / count * 100);
  client.addChannelValue(values[2] / count * 100);
  client.addChannelValue(values[3] / count * 1000); //error
  client.addChannelValue(values[4] / count * 100); //heat rate
  client.addChannelValue(values[5] / count * 1000); //heat rate in W
  client.addChannelValue(values[6] / count * 100);
  client.addChannelValue(values[7] / count * 100);
  client.addChannelValue(values[8] / count * 100);
#if SEND_RF24
#if CHECKSUM_FRAMES
  client.addChecksum();
#endif
#endif
}

void setup()
{
  initLCB();
  Wire.begin();
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  startLCB();
#if SEND_RF24
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash, 10); //This will restore old pointers
  myBuffer.reset(); //This will set all pointers to zero
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 100); //use skip!
#else
  client.begin(BAUD_RATE);
  myBuffer.init(flash, 10); //This will restore old pointers
  myBuffer.setRTC(myRTC, 1); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  myLogger.init(client, myBuffer, myRTC, 30, 2500); //min sampling 30, battery warning 2500 mV
  pinMode(CONNECTED_PIN, INPUT);
  digitalWrite(CONNECTED_PIN, HIGH);
  myLogger._logging_disabled = 1;
#endif
}

void loop() {
  measure();
#if SEND_RF24
  if (ISSET_ACTION(0) && count > 0) {
    UNSET_ACTION(0);
    sendOrBufferLCB();
    count = 0;
  }
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
#else
  if (myLogger._logging_disabled && myRTC.now().get() > 315360000L)
    myLogger._logging_disabled = 0;
  myLogger.run();
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
  }
  if (! connected && myLogger._logging_disabled) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(800);
    pinMode(LED_BUILTIN, INPUT);
  }

  if (! connected) {
    myLogger._mode = 0;
  }
  if (connected && digitalRead(CONNECTED_PIN)) {
    connected++;
    if (connected > 5) {
      client.flush();
      client.end();
      connected = 0;
    }
  }
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    adjust_OSCCAL();
    client.begin(38400);
  }
#endif
}

