/*
   Low Power ADC-Board
   is a variant of BayEOS Low Power Board

   It has 12 screw clamps
   1,7:       GRD
   2,8:       VCC
   3-5,9-11:  A0-A2,A3-A5
   6,12;      MOSFET Power switch via D7

   For EC-5 soil moisture sensors solder a 2.5V LDO voltageregulator on the board.
   Exitation time must be lower then 10ms

   VWC = 0.00119 * mV - 0.400


*/
#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 1
char channel_map[] = "time;bat;ch1;ch2;ch3;ch4;ch5;ch6";
char unit_map[] = "ms;V;%;%;%;%;%;%";

#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>


#define CONNECTED_PIN 9
#define TICKS_PER_SECOND 16
uint8_t connected = 0;

BaySerial client(Serial);
SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>



//Add your sensor measurements here!
float values[NUMBER_OF_CHANNELS + 1];
uint16_t count;
unsigned long last_measurement;

//Add your sensor measurements here!
uint16_t current_tics, last_tics;
void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < NUMBER_OF_CHANNELS + 1; i++) {
      values[i] = 0;
    }
  }

  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  delay(5);
  for (uint8_t ch = 0; ch < NUMBER_OF_CHANNELS; ch++) {
    values[ch + 1] += 0.00119 * 1100.0 / 1023 * analogRead(A0 + ch) - 0.400;
  }
  analogReference(DEFAULT);
  if (digitalRead(CONNECTED_PIN))
    myLogger._bat = (3.3 * (100 + 100) / 100 / 1023 * analogRead(A7)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);


  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < NUMBER_OF_CHANNELS + 1; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }
}



void setup() {
  pinMode(POWER_PIN, OUTPUT);
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 3700); //min_sampling_int = 60, LOW BAT Warning 3700mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);

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
    connected = 0;
    myLogger._mode = 0;
  }
  //Connected pin is pulled to GND
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    adjust_OSCCAL();
    client.begin(38400);
  }

}
