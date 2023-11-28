
/*  A modification of Sapflow-Boards for Dendrometer-Measurements

    For dendrometer measurements remove the two
    resistors and replace one by a bridge

    This sketch collects a number (SEND_COUNT) of measurements and
    sends the data via GPRS to a bayeos gateway

    wiring:
    1 - GND
    2 - common PW
    3 - GND
    4 - SIG Sensor1
    5 - GND
    6 - SIG Sensor2
    ...

*/



// Divider resistors for battery voltage
#define BOARD_ORIGIN "De2"
#define SENSOR1_ORIGIN "P07"
#define SENSOR2_ORIGIN "P08"
#define SENSOR3_ORIGIN "P09"
//#define SENSOR4_ORIGIN "P21"
//#define SENSOR5_ORIGIN "P22"
//#define SENSOR6_ORIGIN "P23"
#define RF24CHANNEL 0x66
#define RF24ADDRESS 0x45c4f1a424
#define BAT_DIVIDER (100.0 + 100.0) / 100.0
#define SENSOR_LENGTH 10
#define SAMPLING_INT 64


#include <Wire.h>
#include <MCP342x.h>

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);


//Configuration
const byte addr = 0;
const uint8_t gain = 0;  //0-3: x1, x2, x4, x8
const uint8_t mode = 0;  //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 3;  //18bit
#define MCP_POWER_PIN A0

MCP342x mcp342x(addr);

#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);              //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer;  //BayEOS Buffer

#include <LowCurrentBoard.h>


void setup() {
  initLCB();
  Wire.begin();
  pinMode(POWER_PIN, OUTPUT);
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash);  //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip();                               //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);  //Nutze RTC relativ!
  client.setBuffer(myBuffer,120);
  startLCB();
}


float measureAndSend(uint8_t ch,char* origin) {
  digitalWrite(A1, ch & 0x4);
  digitalWrite(A2, ch & 0x2);
  digitalWrite(A3, ch & 0x1);
  float v1, v2;
  mcp342x.runADC(0);
  delayLCB(mcp342x.getADCTime());
  v1 = mcp342x.getData();
  mcp342x.runADC(1);
  delayLCB(mcp342x.getADCTime());
  v2 = mcp342x.getData();
  client.startDataFrameWithOrigin(BayEOS_Float32le, origin, 1, 1);
  client.addChannelValue(v2 / (v1 + v2) * SENSOR_LENGTH * 1000);
  client.addChecksum();
  client.sendOrBuffer();
}

void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    digitalWrite(MCP_POWER_PIN, HIGH);
    delay(2);
#ifdef SENSOR1_ORIGIN
    measureAndSend(0,SENSOR1_ORIGIN);
#endif
#ifdef SENSOR2_ORIGIN
    measureAndSend(1,SENSOR2_ORIGIN);
#endif
#ifdef SENSOR3_ORIGIN
    measureAndSend(2,SENSOR3_ORIGIN);
#endif
#ifdef SENSOR4_ORIGIN
    measureAndSend(3,SENSOR4_ORIGIN);
#endif
#ifdef SENSOR5_ORIGIN
    measureAndSend(4,SENSOR5_ORIGIN);
#endif
#ifdef SENSOR6_ORIGIN
    measureAndSend(5,SENSOR6_ORIGIN);
#endif
    digitalWrite(MCP_POWER_PIN, LOW);

    digitalWrite(POWER_PIN, HIGH);
    analogReference(DEFAULT);
    delayLCB(100);
    float bat_voltage = 3.3 * BAT_DIVIDER / 1023 * analogRead(A7);
    digitalWrite(POWER_PIN, LOW);
    client.startDataFrameWithOrigin(BayEOS_Float32le, BOARD_ORIGIN, 1, 1);
    client.addChannelValue(millis());
    client.addChannelValue(bat_voltage);
    client.addChecksum();
    sendOrBufferLCB();
  }
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();
}
