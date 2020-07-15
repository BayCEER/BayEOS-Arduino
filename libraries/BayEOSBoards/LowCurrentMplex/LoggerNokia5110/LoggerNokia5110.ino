/*
 * This is a logger with display
 * 
 * Due to storage limitations the
 * logger only supports 1.5 and no live mode!
 * 
 * 
 */

#define SAMPLING_INT 16
#define PRE_RESISTOR 14300
/* Factor to NTC10 - e.g. 0.5 for NTC5, 0.3 for NTC3 ...*/
#define NTC10FACTOR 0.5
#define MCPPOWER_PIN 6
#define LIPO 1

#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>
#include <Wire.h>
#include <MCP342x.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
//Hardware SPI+10,9 (RF24)+4
// pin 10 - Data/Command select (D/C)
// pin 3 - LCD chip select (CS)
// pin 4 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(10, 3, 4);


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
      myLogger.handleCommand1_5();
      myLogger.sendBinaryDump();
    }
  } else {
    delayLCB(d);
  }
}


//Add your sensor measurements here!
float values[9];
uint16_t count;
unsigned long last_measurement;

//Add your sensor measurements here!
uint16_t current_tics, last_tics;
void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < 9; i++) {
      values[i] = 0;
    }
  }


  digitalWrite(MCPPOWER_PIN, HIGH);
  delayLogger(20);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.print(myRTC.now().hour());
  display.print(":");
  if(myRTC.now().minute()<10) 
    display.print("0");
  display.print(myRTC.now().minute());
  display.print(" GMT");
  display.display();


  for (uint8_t ch = 0; ch < 8; ch++) {
    digitalWrite(A1, ch & 0x4);
    digitalWrite(A2, ch & 0x2);
    digitalWrite(A3, ch & 0x1);
    delayLogger(10);
    mcp342x.runADC(0);
    delayLogger(mcp342x.getADCTime());
    float I = mcp342x.getData() / PRE_RESISTOR;

    mcp342x.runADC(1);
    delayLogger(mcp342x.getADCTime());
    float R_mess = mcp342x.getData() / I / NTC10FACTOR;
    float t = ntc10_R2T(R_mess);
    display.setCursor((ch % 2) * 42,8 + (ch / 2) * 8);
    display.print(t);
    display.display();
    values[ch + 1] += t;
  }
  digitalWrite(MCPPOWER_PIN, LOW);

  digitalWrite(POWER_PIN, HIGH);
#if LIPO
  analogReference(DEFAULT);
  myLogger._bat = (3.3 * 200 / 100 / 1023 * analogRead(A7)) * 1000;
#else
  analogReference(INTERNAL);
  if (digitalRead(CONNECTED_PIN))
    myLogger._bat = (1.1 * 320 / 100 / 1023 * analogRead(A0)) * 1000;
#endif
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);
  
  display.setCursor(0,40);
  if(myLogger._bat>3700){
    display.print("BAT: ");
    display.print((float) myLogger._bat/1000);
  } else 
    display.print("Low Bat!");
  display.display();

  count++;

  client.startDataFrame(0x41);
  client.addToPayload((uint8_t) 1);
  client.addToPayload((float) millis());
  //client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < 9; i++) {
    client.addToPayload(i+2);
    client.addToPayload(values[i]);
    //client.addChannelValue(values[i] / count, i + 2);
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
#if LIPO
  myLogger.init(client, myBuffer, myRTC, 60, 3700); //min_sampling_int = 60
#else
  myLogger.init(client, myBuffer, myRTC, 60, 2500); //min_sampling_int = 60
#endif
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  display.begin();
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(15, 20);
  display.print("Set Clock");
  display.display();


  Wire.begin();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  initLCB(); //init time2
}

void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;

  if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                       (myRTC.get() - last_measurement) >= SAMPLING_INT)) {
    last_measurement = myRTC.get();
    measure();
  }
  myLogger.logData();
  if(connected){
    myLogger.sendBinaryDump();
    myLogger.handleCommand1_5();
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
