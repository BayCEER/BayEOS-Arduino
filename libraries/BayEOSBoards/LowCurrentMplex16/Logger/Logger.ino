#define SAMPLING_INT 30
#define PRERESISTOR 10000.0
//When inividual preresistorvalues are given PRERESITOR is ignored
//#define PRERESISTORS {9955.0, 9964.0, 9956.0, 9966.0, 9955.0, 9972.0, 9975.0, 9972.0, 9945.0, 9962.0, 9988.0, 9957.0, 9957.0, 9964.0, 9950.0, 9954.0 }
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR 0.5
#define MCPPOWER_PIN 6
char channel_map[] = "time;bat;T1;T2;T3;T4;T5;T6;T7;T8;T9;T10;T11;T12;T13,T14;T15;T16";
char unit_map[] = "ms;V;C;C;C;C;C;C;C;C;C;C;C;C;C;C;C;C";

#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
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

#define CONNECTED_PIN 9
#define TICKS_PER_SECOND 16
uint8_t connected = 0;


BaySerial client(Serial);
SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>

void delayLogger(unsigned long d) {
  if (connected) {
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
  if (digitalRead(CONNECTED_PIN))
    myLogger._bat = (1.1 * 320 / 100 / 1023 * analogRead(A0)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);
  digitalWrite(MCPPOWER_PIN, HIGH);
  delayLogger(20);
  for (uint8_t ch = 0; ch < 16; ch++) {
    digitalWrite(A1, ch & 0x8);
    digitalWrite(A0, ch & 0x4);
    digitalWrite(A3, ch & 0x2);
    digitalWrite(A2, ch & 0x1);
    delayLogger(10);
    mcp342x.runADC(0);
    delayLogger(mcp342x.getADCTime());
#ifdef PRERESISTORS
    strom = span / preresistors[ch]; //current in A
#else
    strom = span / PRERESISTOR; //current in A
#endif

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
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 2500); //min_sampling_int = 60
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);
  Wire.begin();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  initLCB(); //init time2
}

void loop() {
  if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                       (myRTC.get() - last_measurement) >= SAMPLING_INT)) {
    last_measurement = myRTC.get();
    measure();
  }
  myLogger.run(connected);

  if (! connected && myLogger._logging_disabled) {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    delayLCB(200);
    digitalWrite(LED_BUILTIN, LOW);
    delayLCB(800);
    pinMode(LED_BUILTIN, INPUT);
  }

  //sleep until timer2 will wake us up...
  if (! connected) {
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
  }

  //check if still connected
  if (connected && digitalRead(CONNECTED_PIN)) {
    client.flush();
    client.end();
    myLogger._mode = 0;
    connected = 0;
  }

  //Connected pin is pulled to GND
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    adjust_OSCCAL();
    client.begin(38400);
  }

}
