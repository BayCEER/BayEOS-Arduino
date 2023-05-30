/*
   Logger-Sketch for Read-Out via RF24
   use BaySerialRF24/LoggerConnector as receiver
*/

#define SAMPLING_INT 60
#define PRE_RESISTOR 14300
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR 0.5
#define MCPPOWER_PIN 6
#define NRF24_TRYINT 60
#define BLINK_ON_LOGGING_DISABLED 1
const uint8_t channel = 0x70;
const uint8_t adr[] = {0x12, 0xae, 0x31, 0xc4, 0x45};

#define BAT_WARNING 3800


//channel map and unit map must not exceed 98 characters!
char channel_map[] = "time;bat;T1;T2;T3;T4;T5;T6;T7;T8;T9;T10;T11;T12;T13,T14;T15;T16";
char unit_map[] = "ms;V;C;C;C;C;C;C;C;C;C;C;C;C;C;C;C;C";

#include <BayEOSBufferSPIFlash.h>
#include <BaySerialRF24.h>
#include <BayEOSLogger.h>
#include <Sleep.h>
#include <Wire.h>
#include <MCP342x.h>
#include <math.h>

const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 3; //0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);
//Configure your resistors on the board!

float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}

#define TICKS_PER_SECOND 16


RF24 radio(9, 10);
BaySerialRF24 client(radio, 50, 3); //wait maximum 100ms for ACK
SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>

void delayLogger(unsigned long d) {
  if (client.connected) {
    unsigned long s = millis();
    while ((millis() - s) < d) {
      myLogger.handleCommand();
      myLogger.sendBinaryDump();
    }
  } else {
    delayLCB(d);
  }
}


//Add your sensor measurements here!
float values[17];
uint16_t count;
unsigned long last_measurement;

//Add your sensor measurements here!
uint16_t current_tics, last_tics;
void measure() {
  float span, strom, ntc10r;
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < 17; i++) {
      values[i] = 0;
    }
  }






  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  myLogger._bat = (1.1 * 200 / 100 / 1023 * analogRead(A0)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);
  digitalWrite(MCPPOWER_PIN, HIGH);
  delayLogger(20);
  for (uint8_t ch = 0; ch < 16; ch++) {
    digitalWrite(A0, ch & 0x8);
    digitalWrite(A1, ch & 0x4);
    digitalWrite(A3, ch & 0x2);
    digitalWrite(A2, ch & 0x1);
    delayLogger(10);
    mcp342x.runADC(0);
    delayLogger(mcp342x.getADCTime());
    strom = span / PRE_RESISTOR; //current in A

    mcp342x.runADC(1);
    delayLogger(mcp342x.getADCTime());
    span = mcp342x.getData();
    ntc10r = span / strom / NTC10FACTOR;


    values[ch + 1] += ntc10_R2T(ntc10r);
  }
  digitalWrite(MCPPOWER_PIN, LOW);


  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < 17; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }



}




void setup() {
  pinMode(MCPPOWER_PIN, OUTPUT);
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.init(channel, adr);
  radio.powerDown();
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 120, BAT_WARNING); //min_sampling_int = 120
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);
  Wire.begin();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  initLCB(); //init time2
}


unsigned long last_try = -NRF24_TRYINT;


void loop() {
  if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                       (myRTC.get() - last_measurement) >= SAMPLING_INT)) {
    last_measurement = myRTC.get();
    measure();
  }
  myLogger.run(client.connected);

  if (! client.connected) {
    myLogger._mode = 0;
    //sleep until timer2 will wake us up...
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);

    //check if receiver is present
    if ((myRTC.get() - last_try) > NRF24_TRYINT) {
      blinkLED(0);
      last_try = myRTC.get();
      client.sendTestByte();
#if BLINK_ON_LOGGING_DISABLED
      if (! client.connected && myLogger._logging_disabled) {
        last_try -= (NRF24_TRYINT - 5);
        blinkLED(10);
      }
#endif
    }
  } else if ((millis() - client.last_activity) > 30000) {
    //check if still connected
    last_try = myRTC.get();
    client.sendTestByte();
  }

}
