/****************************************************************

  Logger Sketchf or Read-Out via RF24
  use BaySerialRF24/LoggerConnector as receiver

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


***************************************************************/

#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 6
#define NRF24_TRYINT 60
#define BLINK_ON_LOGGING_DISABLED 1
const uint8_t channel = 0x33;
const uint8_t adr[] = {0x96, 0xf0, 0x3f, 0xc4, 0x45};
//channel map and unit map must not exceed 98 characters!
char channel_map[] = "time;bat;ch1;ch2;ch3;ch4;ch5;ch6";
char unit_map[] = "ms;V;%;%;%;%;%;%";


#include <BayEOSBufferSPIFlash.h>
#include <BaySerialRF24.h>
#include <BayEOSLogger.h>
#include <Sleep.h>
#include <NTC.h>

RF24 radio(9, 10);
BaySerialRF24 client(radio, 100, 3); //wait maximum 100ms for ACK


SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#define TICKS_PER_SECOND 16
#include <LowCurrentBoard.h>



//Add your sensor measurements here!
float values[NUMBER_OF_CHANNELS + 1];
uint16_t count;
unsigned long last_measurement;

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
  client.init(channel, adr);
  radio.powerDown();
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 3700); //min_sampling_int = 60, LOW BAT Warning 3000mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);

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
