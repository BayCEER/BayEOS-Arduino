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

*/

#define BOARD_ORIGIN "De8"
#define SENSOR1_ORIGIN "P24"
#define SENSOR2_ORIGIN "P25"
#define RF24CHANNEL 0x66
#define RF24ADDRESS 0x45c4f1a424
#define WITH_CHECKSUM 1
#define BAT_DIVIDER (100.0 + 100.0) / 100.0
#define SENSOR_LENGTH 10
#define SAMPLING_INT 64

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);


#include <Wire.h>
#include <MCP342x.h>

//Configuration
const byte addr = 0;
const uint8_t gain = 0;  //0-3: x1, x2, x4, x8
const uint8_t mode = 0;  //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 3;  //18bit
#define MCP_POWER_PIN A3

MCP342x mcp342x(addr);

#include <LowCurrentBoard.h>

void setup() {
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash);  //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip();                               //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);  //use the rtc clock but relative
  client.setBuffer(myBuffer, 120);               //use skip!
  initLCB();                                     //init time2
  pinMode(MCP_POWER_PIN, OUTPUT);
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(POWER_PIN, OUTPUT);
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement

    digitalWrite(MCP_POWER_PIN, HIGH);
    delay(2);
    float v1, v2;
#ifdef SENSOR1_ORIGIN
    mcp342x.runADC(2);
    delayLCB(mcp342x.getADCTime());
    v1 = mcp342x.getData();
    mcp342x.runADC(3);
    delayLCB(mcp342x.getADCTime());
    v2 = mcp342x.getData();
    client.startDataFrameWithOrigin(BayEOS_Float32le, SENSOR1_ORIGIN, WITH_CHECKSUM, 1);
    client.addChannelValue(v2 / (v1 + v2) * SENSOR_LENGTH * 1000);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    client.sendOrBuffer();
#endif
#ifdef SENSOR2_ORIGIN
    mcp342x.runADC(0);
    delayLCB(mcp342x.getADCTime());
    v1 = mcp342x.getData();
    mcp342x.runADC(1);
    delayLCB(mcp342x.getADCTime());
    v2 = mcp342x.getData();
    client.startDataFrameWithOrigin(BayEOS_Float32le, SENSOR2_ORIGIN, WITH_CHECKSUM, 1);
    client.addChannelValue(v2 / (v1 + v2) * SENSOR_LENGTH * 1000);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    client.sendOrBuffer();
#endif

    digitalWrite(MCP_POWER_PIN, LOW);
    digitalWrite(POWER_PIN, HIGH);
    analogReference(DEFAULT);
    delayLCB(1);
    float bat_voltage = 3.3 * BAT_DIVIDER / 1023 * analogRead(A7);
    digitalWrite(POWER_PIN, LOW);

    client.startDataFrameWithOrigin(BayEOS_Float32le, BOARD_ORIGIN, WITH_CHECKSUM, 1);
    client.addChannelValue(millis());
    client.addChannelValue(bat_voltage);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();
}
