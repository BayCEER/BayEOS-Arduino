/****************************************************************

   Example Sketch for running BayEOS-LCB with SPI-flash as logger

***************************************************************/
#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 4
#define NRF24_TRYINT 60
#define BLINK_ON_LOGGING_DISABLED 1
const uint8_t channel = 0x70;
const uint8_t adr[] = {0x46, 0xf0, 0xe3, 0xc4, 0x45};
//channel map and unit map must not exceed 98 characters!
char channel_map[] = "time;bat;temp;hum;pre";
char unit_map[] = "ms;V;C;%;Pa";

#include <BayEOSBufferSPIFlash.h>
#include <BaySerialRF24.h>

RF24 radio(9, 10);
BaySerialRF24 client(radio, 100, 3); //wait maximum 100ms for ACK

#include <BayEOSLogger.h>
#include <BME280SoftI2C.h>
BME280SoftI2C bme(A4, A5); // I2C


#define TICKS_PER_SECOND 16

SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
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

  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  myLogger._bat = (1.1 * (220 + 100) / 100 / 1023 * analogRead(A7)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);
  bme.triggerMeasurement();
  delayLCB(40);
  values[1] += bme.readTemperature();
  values[3] += bme.readPressure();
  values[2] += bme.readHumidity();

  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }
}



void setup() {
  client.init(channel, adr);
  radio.powerDown();
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 2700); //min_sampling_int = 60, LOW BAT Warning 3700mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);
  initLCB(); //init time2
  while (!bme.begin(0x76)) {};

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
