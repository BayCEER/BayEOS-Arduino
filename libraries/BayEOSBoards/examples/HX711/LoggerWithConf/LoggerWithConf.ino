/*
   This is the logger sketch for a BayEOS HX711 Board (2 HX711) run as continuous scale

   The sketch logs 8 Channels
   - CPU Time [ms]
   - Battery Voltage [V]
   - Raw adc reading 1 [counts]
   - Raw adc reading 2 [counts]
   - temperature 1 [C]
   - temperature 2 [C]
   - calculated weight 1 [g] //depends on unit of calibration weight -- see below
   - caluclated weight 2 [g]

   For calibration
   1. run the board as logger for one day without weight and one day with known weight. Temperature variations
   should be in the rage of desired measurements.
   2. Calculate a linear regression out of the adc and temp values. One with and one without weight.
   3. Calculate the predicted adc values for 10 and 20°C
   4. Put all values in the calibration section of this sketch, set INIT_CAL to 1
      and upload the sketch to the board.
   5. Connect with the logger software and check for reasonable readings (Channel 7+8)

   The calibration data is stored in EEPROM. In case of software updates you can reflash the sketch
   to the board. But make sure the set INIT_CAL to 0 before. With INIT_CAL 0 the calibration data
   in the sketch is ignored and calibration data from EEPROM is used.
*/


#define SAMPLING_INT 60
#define NUMBER_OF_CHANNELS 7
#define BAT_DIVIDER (3.3 * (100 + 100) / 100 / 1023)
#define BAT_WARNING 3500

#include <HX711Array.h>
uint8_t dout[] = {6, 4};
uint8_t sck = 3;
long adc[2];
HX711Array scale;

#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <NTC.h>

#define CONNECTED_PIN 9
uint8_t connected = 0;

NTC_HX711 ntc(scale, A3, 2 * 470000, 3.0); //Adjust resistor values
Scale4PointCal cal0;
Scale4PointCal cal1(28);

#define INIT_CAL 0
#if INIT_CAL
// only for first run
float sw0 = 1467.0; //Calibration weight of first weight cell in gramm
float sw1 = 1463.0; //Calibration weight of second weight in gramm
float t[] = {10.0, 20.0}; //Calibration temperatures
long adc0[] = {331519L, 332595L, 1255903L, 1255912L}; //Calibration Values: adc[zero,t0], adc[zero,t1], ...
long adc1[] = {331519L, 332595L, 1255903L, 1255912L}; //Calibration Values: adc[zero,t0], adc[zero,t1], ...
#endif

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
  ntc.readResistance();
  float temp0 = ntc.getTemp(0);
  float temp1 = ntc.getTemp(1);

  scale.power_up();
  scale.set_gain(128);
  scale.read_average(adc, 1); //dummy reading
  scale.read_average_with_filter(adc);
  scale.power_down();
  values[1] += adc[0];
  values[2] += adc[1];
  values[3] += temp0;
  values[4] += temp1;
  values[5] += cal0.getWeight(adc[0], temp0);
  values[6] += cal1.getWeight(adc[1], temp1);

  analogReference(DEFAULT);
  myLogger._bat = (BAT_DIVIDER * analogRead(A7)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
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
  scale.begin(dout, 2, sck); //start HX711Array with 2 ADCs
#if INIT_CAL
  cal0.setConf(sw0, t, adc0);
  cal1.setConf(sw1, t, adc1);
  cal0.saveConf();
  cal1.saveConf();
#endif
  cal0.readConf();
  cal1.readConf();

}


void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;

  if (! myLogger._logging_disabled &&
      (myLogger._mode == LOGGER_MODE_LIVE || (myRTC.get() - last_measurement) >= SAMPLING_INT) &&
      myLogger._mode != LOGGER_MODE_DUMP
     ) {
    if (connected) { //wait for the transmission to complete!
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
    myLogger._mode = 0;
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
  }

  //check if still connected
  if (connected && digitalRead(CONNECTED_PIN)) {
    client.flush();
    client.end();
    connected = 0;
  }
  //Connected pin is pulled to GND
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    adjust_OSCCAL();
    client.begin(38400);
  }

}
