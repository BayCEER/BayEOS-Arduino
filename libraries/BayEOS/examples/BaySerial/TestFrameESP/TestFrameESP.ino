/*
 * Example Sketch for sending data via ESP01
 *
 * Please program ESP01 with
 * SerialRouterWiFiManagerWebserver-Sketch from
 * BayEOS-ESP8266-Library
 *
 * Configuration of the Board is done via the webinterface of the esp
 * in AP-Mode
 *
 *
 */
#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>

void blink(uint8_t times)
{
  while (times)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    times--;
  }
}

BaySerialESP client(Serial, 7);
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

void setup(void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  myBuffer.init(flash); // This will restore old pointers
  // myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); // This will move read pointer to write pointer
  client.setBuffer(myBuffer);
  client.begin(38400);
  blink(3);
  client.powerUp();
  // Check if ESP01 is already attached to a WIFI AP
  // IF not ESP01 starts in AP-Mode and can be configured
  // via WIFI (SSID: BayEOS-Serial-Router, PW: bayeos24)
  while (client.isReady())
  {
    blink(2);
    delay(500);
  }
  client.powerDown();
  delay(1000);
  blink(4);
}

long package_count = 1;
void loop(void)
{
  // Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(millis() / 1000);
  client.addChannelValue(package_count);
  client.writeToBuffer();
  package_count++;

  if ((package_count % 5) == 1)
  {
    client.powerUp();
    blink(client.sendMultiFromBuffer() + 1);
    client.powerDown();
  }

  delay(5000);
}
