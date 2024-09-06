/**
 * Example Sketch for LCB-Logger with a ESP01LoggerConnector
 *
 * Sketch for ESP01:
 * https://github.com/BayCEER/BayEOS-ESP8266/tree/master/examples/LoggerConnectorESP01
 *
 */
#define SAMPLING_INT 30
#define NUMBER_OF_CHANNELS 3

#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>

#define CONNECTED_PIN 9
#define TICKS_PER_SECOND 16
#define TIME_OUT_CONNECTED 180
#define TIME_OUT_UNCONNECTED 90
#define LCB_BAT_MULTIPLIER 3.3 * (100 + 100) / 100 / 1023
#define LCB_BAT_ADCPIN A7

// channel map and unit map must not exceed 98 characters!
char channel_map[] = "time;bat;CH2;CH3";
char unit_map[] = "ms;V;-;-";

uint8_t connected = 0;
uint8_t forced_unconnect = 0;

BaySerial client(Serial);
SPIFlash flash(8); // CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>

// Add your sensor measurements here!
float values[NUMBER_OF_CHANNELS];
uint16_t count;
unsigned long last_measurement;

uint16_t current_tics, last_tics;

void measure()
{
  if (myLogger._logged_flag)
  {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++)
    {
      values[i] = 0;
    }
  }

  digitalWrite(POWER_PIN, HIGH);
  analogReference(DEFAULT);
  myLogger._bat = (LCB_BAT_MULTIPLIER * analogRead(LCB_BAT_ADCPIN)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  if (!connected)
    digitalWrite(POWER_PIN, LOW);
  // Add your sensor measurements here!
  values[1] += (float)myLogger.secondsSinceLastCommunication();

  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < NUMBER_OF_CHANNELS; i++)
  {
    client.addChannelValue(values[i] / count, i + 2);
  }
}

void setup()
{
  pinMode(POWER_PIN, OUTPUT);
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); // Nutze RTC absolut!
  client.setBuffer(myBuffer);
  // register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 3700); // min_sampling_int = 60, LOW BAT Warning 3700mV
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);

  // disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  initLCB(); // init time2
}

void loop()
{
  // Enable logging if RTC give a time later than 2010-01-01
  if (myLogger._logging_disabled && myRTC.get() > 315360000L)
    myLogger._logging_disabled = 0;

  if (!myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                      (myRTC.get() - last_measurement) >= SAMPLING_INT))
  {
    last_measurement = myRTC.get();
    measure();
  }
  myLogger.run();

  if (!connected && myLogger._logging_disabled)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delayLCB(200);
    digitalWrite(LED_BUILTIN, LOW);
    delayLCB(800);
  }

  // sleep until timer2 will wake us up...
  if (!connected)
  {
    myLogger._mode = 0;
    sleepLCB();
  }

  // check if still connected
  if (connected)
  {
    int sec = myLogger.secondsSinceLastCommunication();
    if (digitalRead(CONNECTED_PIN) && sec > TIME_OUT_UNCONNECTED)
      connected = 0;
    if (!digitalRead(CONNECTED_PIN) && sec > TIME_OUT_CONNECTED)
    {
      connected = 0;
      forced_unconnect = 1;
    }
    if (!connected)
    {
      client.flush();
      client.end();
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(POWER_PIN, LOW);
    }
  }
  if (forced_unconnect && digitalRead(CONNECTED_PIN))
    forced_unconnect = 0; // User released connected pin

  // Connected pin is pulled to GND
  if (!connected && !digitalRead(CONNECTED_PIN) && !forced_unconnect)
  {
    connected = 1;
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(POWER_PIN, HIGH);
    adjust_OSCCAL();
    client.begin(38400);
    myLogger._last_communication = myRTC.get();
  }
}
