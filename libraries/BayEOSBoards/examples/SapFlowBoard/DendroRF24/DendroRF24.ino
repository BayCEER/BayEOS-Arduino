
/*  A modification of Sapflow-Boards for Dendrometer-Measurements

    For dendrometer measurements remove the two
    resistors and replace one by a bridge

    wiring:
    1 - GND
    2 - common PW
    3 - GND
    4 - SIG Sensor1
    5 - GND
    6 - SIG Sensor2

Frame: 
5 byte for Delayed Frames
3 byte for Checksum Frames
5 byte for Origin Frame
3 byte for Data Frame

=> 32-16=16 == max. 4 Float Values

*/



#define BOARD_ORIGIN "D123" /* Not more than 4 Byte!! */
#define SENSOR1_LENGTH 11
#define SENSOR2_LENGTH 11
#define RF24CHANNEL 0x2b /* Line 1 */
#define RF24ADDRESS 0x45f437aebf
#define BAT_DIVIDER (100.0 + 100.0) / 100.0
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



void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    digitalWrite(MCP_POWER_PIN, HIGH);
    delay(2);
    float v1, v2;
#ifdef SENSOR1_LENGTH
    mcp342x.runADC(2);
    delayLCB(mcp342x.getADCTime());
    v1 = mcp342x.getData();
    mcp342x.runADC(3);
    delayLCB(mcp342x.getADCTime());
    v2 = mcp342x.getData();
    float sensor1 = v2 / (v1 + v2) * SENSOR1_LENGTH * 1000;
#endif
#ifdef SENSOR2_LENGTH
    mcp342x.runADC(0);
    delayLCB(mcp342x.getADCTime());
    v1 = mcp342x.getData();
    mcp342x.runADC(1);
    delayLCB(mcp342x.getADCTime());
    v2 = mcp342x.getData();
    float sensor2 = v2 / (v1 + v2) * SENSOR2_LENGTH * 1000;
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
#ifdef SENSOR1_LENGTH
    client.addChannelValue(sensor1);
#endif
#ifdef SENSOR2_LENGTH
    client.addChannelValue(sensor2);
#endif
    client.addChecksum();
    sendOrBufferLCB();
  }
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();
}
