/****************************************************************

  Logger Sketchf or Read-Out via RF24
  use BaySerialRF24/LoggerConnector as receiver

  Connection

  A) ELECTRODES
       |--R_PRE1--A0---E1+ E1- -|
  D6 --|--R_PRE2--A1---E2+ E2- -|-- D4
       |--R_PRE3--A2---E3+ E3- -|

  B) NTC05
       |--R_NRC1--A3--  NTC1 --|
  D3 --|--R_NTC2--A4--  NTC2 --|-- GND
       |--R_NTC3--A5--  NTC3 --|

  C) RAIN

  D2 -- RAIN -- GND

***************************************************************/

#define R_NTC 10000.0
#define R_PRE 100000.0
#define WITHRAINGAUGE 1
#define BLINK_ON_LOGGING_DISABLED 1
#define ADC_COUNT 20
#define WAIT_TIME_MICROS_POWERUP 500
#define WAIT_TIME_MICROS_BETWEEN 1000
#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 10
#define NRF24_TRYINT 60
const uint8_t channel = 0x67;
const uint8_t adr[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
#define LIPO 0
//channel map and unit map must not exceed 98 characters!
char channel_map[] = "time;bat;temp1;temp2;temp3;R1+;R2+;R3+;R1-;R2-;R3-;Rain";
char unit_map[] = "ms;V;C;C;C;Ohm;Ohm;Ohm;Ohm;Ohm;Ohm;Counts";



#include <BayEOSBufferSPIFlash.h>
#include <BaySerialRF24.h>
#include <BayEOSLogger.h>
#include <Sleep.h>
#include <NTC.h>

RF24 radio(9, 10);
BaySerialRF24 client(radio, 100, 3); //wait maximum 100ms for ACK

NTC_ADC ntc1(3, A3, R_NTC, 5.0);
NTC_ADC ntc2(3, A4, R_NTC, 5.0);
NTC_ADC ntc3(3, A5, R_NTC, 5.0);


#define TICKS_PER_SECOND 16



SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#define TICKS_PER_SECOND 16
#include <LowCurrentBoard.h>



//Add your sensor measurements here!
float values[NUMBER_OF_CHANNELS ];
uint16_t count;
unsigned long last_measurement;

uint16_t current_tics, last_tics;


void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
      values[i] = 0;
    }
  }

  //Add your sensor measurements here!

  values[1] += ntc1.getTemp();
  values[2] += ntc2.getTemp();
  values[3] += ntc3.getTemp();

  uint16_t adc1_p = 0;
  uint16_t adc2_p = 0;
  uint16_t adc3_p = 0;
  uint16_t adc1_n = 0;
  uint16_t adc2_n = 0;
  uint16_t adc3_n = 0;

  for (int i = 0; i < ADC_COUNT; i++) {
    digitalWrite(6, HIGH);
#if WAIT_TIME_MICROS_POWERUP
    delayMicroseconds(WAIT_TIME_MICROS_POWERUP);
#endif
    adc1_p += analogRead(A0);
    adc2_p += analogRead(A1);
    adc3_p += analogRead(A2);
    digitalWrite(6, LOW);
#if WAIT_TIME_MICROS_BETWEEN
    delayMicroseconds(WAIT_TIME_MICROS_BETWEEN);
#endif
    digitalWrite(4, HIGH);
#if WAIT_TIME_MICROS_POWERUP
    delayMicroseconds(WAIT_TIME_MICROS_POWERUP);
#endif
    adc1_n += analogRead(A0);
    adc2_n += analogRead(A1);
    adc3_n += analogRead(A2);
    digitalWrite(4, LOW);
#if WAIT_TIME_MICROS_BETWEEN
    delayMicroseconds(WAIT_TIME_MICROS_BETWEEN);
#endif
  }
  values[4] += R_PRE / (1023.0 * ADC_COUNT / adc1_p - 1);
  values[5] += R_PRE / (1023.0 * ADC_COUNT / adc2_p - 1);
  values[6] += R_PRE / (1023.0 * ADC_COUNT / adc3_p - 1);
  values[7] += R_PRE * (1023.0 * ADC_COUNT / adc1_n - 1);
  values[8] += R_PRE * (1023.0 * ADC_COUNT / adc2_n - 1);
  values[9] += R_PRE * (1023.0 * ADC_COUNT / adc3_n - 1);


  digitalWrite(POWER_PIN, HIGH);
#if LIPO
  analogReference(DEFAULT);
  myLogger._bat = (3.3 * (100 + 100) / 100 / 1023 * analogRead(A7)) * 1000;
#else
  analogReference(INTERNAL);
  myLogger._bat = (1.1 * (470 + 100) / 100 / 1023 * analogRead(A7)) * 1000;
#endif
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);
  analogReference(DEFAULT);


  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }

#if WITHRAINGAUGE
  client.addChannelValue(rain_count, NUMBER_OF_CHANNELS + 2);
#endif

}



void setup() {
  client.init(channel, adr);
  radio.powerDown();
  pinMode(POWER_PIN, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 3300); //min_sampling_int = 60, LOW BAT Warning 3000mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);
  initLCB(); //init time2
}

unsigned long last_try = -NRF24_TRYINT;


void loop() {
#if WITHRAINGAUGE
  handleRainEventLCB();
#endif
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
      last_try = myRTC.get();
      blinkLED(0);
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
