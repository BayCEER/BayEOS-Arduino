
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



// Divider resistors for battery voltage
#define BAT_DIVIDER (100.0 + 100.0) / 100.0
#define SENSOR1_LENGTH 10
#define SENSOR2_LENGTH 10
#define SAMPLING_INT 64


#include <Wire.h>
#include <MCP342x.h>
#include <BayEOSBufferSPIFlash.h>
unsigned long last_sent;

#include <BayDebug.h>
BayDebug client(Serial);


//Configuration
const byte addr = 0;
const uint8_t gain = 0;  //0-3: x1, x2, x4, x8
const uint8_t mode = 0;  //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 3;  //18bit
#define MCP_POWER_PIN A3

MCP342x mcp342x(addr);

SPIFlash flash(8);              //Standard SPIFlash Instanz
BayEOSBufferSPIFlash myBuffer;  //BayEOS Buffer
//Configure your resistors on the board!

#include <LowCurrentBoard.h>


void setup() {
  initLCB();
  Wire.begin();
  pinMode(POWER_PIN, OUTPUT);
  pinMode(MCP_POWER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  myBuffer.init(flash);                          //This will restore old pointers
  myBuffer.skip();                               //This will skip unsent frames
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);  //Nutze RTC relativ!
  client.setBuffer(myBuffer);
  client.begin(9600);
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
    client.startDataFrame();
    client.addChannelValue(millis());
    client.addChannelValue(bat_voltage);
#ifdef SENSOR1_LENGTH
    client.addChannelValue(sensor1);
#endif
#ifdef SENSOR2_LENGTH
    client.addChannelValue(sensor2);
#endif
    sendOrBufferLCB();
    Serial.flush();
    digitalWrite(POWER_PIN, LOW);
  }
  if(ISSET_ACTION(7)){
    UNSET_ACTION(7);
    client.sendFromBuffer();
    Serial.flush();
  }
  sleepLCB();
}
