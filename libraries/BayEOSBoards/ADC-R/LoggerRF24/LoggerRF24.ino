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
#define ADC_COUNT 20
#define WAIT_TIME_MICROS_POWERUP 500
#define WAIT_TIME_MICROS_BETWEEN 1000
#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 10
#define NRF24_TRYINT 60
const uint8_t channel = 0x67;
const uint8_t adr[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
#define LIPO 0


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


#define CONNECTED_PIN 3
#define TICKS_PER_SECOND 16
uint8_t connected = 0;



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
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 3300); //min_sampling_int = 60, LOW BAT Warning 3000mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  initLCB(); //init time2
}

unsigned long last_try = -NRF24_TRYINT;

void send_test_byte(void) {
  uint8_t test_byte[] = {XOFF};
  uint8_t res = 0;
  last_try = myRTC.get();
  if (! connected) radio.powerUp();
  radio.stopListening();
  res = radio.write(test_byte, 1);
  uint8_t curr_pa = 0;
  while (!res && curr_pa < 4) {
    radio.setPALevel((rf24_pa_dbm_e) curr_pa);
    delayMicroseconds(random(1000));
    res = radio.write(test_byte, 1);
    curr_pa++;
  }
  if (res){
    blinkLED(0);
    client.last_activity = millis();
    radio.startListening();
    digitalWrite(LED_BUILTIN, 1);
    connected=1;
  } else {
    digitalWrite(LED_BUILTIN, 0);
    radio.powerDown();
    connected=0;
  }
}

void loop() {
#if WITHRAINGAUGE
  handleRainEventLCB();
#endif
  //Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;

  if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                       (myRTC.get() - last_measurement) >= SAMPLING_INT)) {
    last_measurement = myRTC.get();
    measure();
  }
  myLogger.run();

  if (! connected) {
    myLogger._mode = 0;
    //sleep until timer2 will wake us up...
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
    
   //check if receiver is present
   if((myRTC.get() - last_try) > NRF24_TRYINT) {
      send_test_byte();
      if (! connected && myLogger._logging_disabled)
        blinkLED(NRF24_TRYINT * 2);
    } 
  } else if((millis() - client.last_activity) > 30000) {
    //check if still connected
    send_test_byte();
  }


}