#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 5
#define BAT_DIVIDER (3.3 * (100 + 100) / 100 / 1023)
#define BAT_WARNING 3500
#define RTC_SECONDS_CORRECT -25000

#include <HX711Array.h>
uint8_t dout[] = {A3, A2, A1, A0};
uint8_t sck = A4;
long adc[4];
HX711Array scale;

#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>
#include <math.h>

#define CONNECTED_PIN 9
uint8_t connected = 0;

BaySerial client(Serial);
SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>

//Add your sensor measurements here!
float values[NUMBER_OF_CHANNELS ];
uint16_t count;
unsigned long last_measurement, warmup_start;

uint16_t try_count;
uint8_t warmup;
uint8_t measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
      values[i] = 0;
    }
  }

  //Add your sensor measurements here!
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(POWER_PIN, HIGH);
  scale.power_up();
  scale.read_average(adc);
  values[1] += adc[0];
  values[2] += adc[1];
  values[3] += adc[2];
  values[4] += adc[3];

  analogReference(DEFAULT);
  myLogger._bat = (BAT_DIVIDER * analogRead(A7)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  scale.power_down();
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(POWER_PIN, LOW);

  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }
  return 0;
}



void setup() {
  pinMode(POWER_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, SAMPLING_INT, BAT_WARNING);
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  initLCB(); //init time2
  scale.begin(dout, 4, sck); //start HX711Array with 4 ADCs
  scale.set_gain(32);
  scale.power_down();
}


void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;

  if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                       (myRTC.get() - last_measurement) >= SAMPLING_INT)) {
    if(connected){ //wait for the transmission to complete!
      Serial.flush();
      delay(50);
    }
    measure();
    last_measurement = myRTC.get();

  }

  myLogger.run();

  if (! connected && myLogger._logging_disabled) {
    digitalWrite(LED_BUILTIN, HIGH);
    delayLCB(200);
    digitalWrite(LED_BUILTIN, LOW);
    delayLCB(800);
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
