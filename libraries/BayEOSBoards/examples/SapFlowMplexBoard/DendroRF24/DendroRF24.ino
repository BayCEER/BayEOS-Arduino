
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


#define BOARD_ORIGIN "D123" /* Not more than 4 Byte!! */
#define SENSOR1_LENGTH 11
#define SENSOR2_LENGTH 11
#define SENSOR3_LENGTH 11
//#define SENSOR4_LENGTH 11
//#define SENSOR5_LENGTH 11
//#define SENSOR6_LENGTH 11
#define RF24CHANNEL 0x2b 
#define RF24ADDRESS 0x45f437aebf
#define SAMPLING_INT 64
#define BAT_DIVIDER (100.0 + 100.0) / 100.0

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


float measure(uint8_t ch,float sensor_length) {
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
  return (v2 / (v1 + v2) * sensor_length * 1000);
}

void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    digitalWrite(MCP_POWER_PIN, HIGH);
    client.startDataFrameWithOrigin(BayEOS_Float32le, BOARD_ORIGIN, 1, 1);
    delay(2);
    uint8_t count=0;
#ifdef SENSOR1_LENGTH
    client.addChannelValue(measure(0,SENSOR1_LENGTH),0+3);
    count++;
#endif
#ifdef SENSOR2_LENGTH
    client.addChannelValue(measure(1,SENSOR2_LENGTH),1+3);
    count++;
#endif
#ifdef SENSOR3_LENGTH
    client.addChannelValue(measure(2,SENSOR3_LENGTH),2+3);
    count++;
#endif
    if(count>0){
      client.addChecksum();
      client.sendOrBuffer();
      client.startDataFrameWithOrigin(BayEOS_Float32le, BOARD_ORIGIN, 1, 1);
      count=0;
    }
#ifdef SENSOR4_LENGTH
    client.addChannelValue(measure(3,SENSOR4_LENGTH),3+3);
    count++;
#endif
#ifdef SENSOR5_LENGTH
    client.addChannelValue(measure(4,SENSOR5_LENGTH),4+3);
    count++;
#endif
#ifdef SENSOR6_LENGTH
    client.addChannelValue(measure(5,SENSOR6_LENGTH),5+3);
    count++;
#endif
    if(count>0){
      client.addChecksum();
      client.sendOrBuffer();
    }
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
