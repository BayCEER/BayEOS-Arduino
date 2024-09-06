/**
 * Example Sketch for BayEOS logger
 *
 * This logger is always active.
 *
 * For examples with sleep-mode and low power consumption
 * look into the board examples.
 *
 *
 */

#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <BayEOSBufferSPIFlash.h>
#include <RTClib.h>

#define BAUD_RATE 38400

RTC_Millis myRTC;

BaySerial client = BaySerial(Serial);
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;

// channel map and unit map must not exceed 98 characters!
char channel_map[] = "millis;RTC;Bat";
char unit_map[] = "ms;s;V";

void setup()
{
  myRTC.begin();
  myBuffer.init(flash); // This will restore old pointers
  client.begin(BAUD_RATE);

  // register RTC in buffer
  myBuffer.setRTC(myRTC);

  // register Buffer in bayeos-client
  client.setBuffer(myBuffer);

  // register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 30, 3500); // min sampling 30, battery warning 3500 mV
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);

  delay(1000);
}

void loop()
{

  // Create DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis());          // milliseconds
  client.addChannelValue(myRTC.now().get()); // seconds
  float V = 3.3 / 1024 * analogRead(A7) * (100 + 100) / 100;
  client.addChannelValue(V); // Voltage
  myLogger._bat = 1000 * V;  // Store current voltage values in _bat

  // Run the logger
  // this handels all commands and saves the dataframe to the
  // flash storage if logging is due
  myLogger.run();
}
