#include <BayEOSBufferSPIFlash.h>
#include <BaySIM800.h>

// SIM800-Config string.
// Gateway-url|login|password|Origin (== Board unique identifier)|apn of sim-card|apn-user|apn-pw|PIN
#define SIM800_CONFIG "http://132.180.112.128/gateway/frame/saveFlat|import@Workshop|ChangeME|MyGPRS-Board|iot.1nce.net||||"

BaySIM800 client = BaySIM800(Serial);
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

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

void setup(void)
{
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);
  client.readConfigFromStringPGM(PSTR(SIM800_CONFIG));
  myBuffer.init(flash); // This will restore old pointers
  // myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); // This will move read pointer to write pointer
  client.setBuffer(myBuffer);
  blink(client.begin(38400) + 1);
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
    blink(client.sendMultiFromBuffer() + 1);

  delay(5000);
}
