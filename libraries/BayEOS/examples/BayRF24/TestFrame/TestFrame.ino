#include <BayRF24.h>

#define RF24ADDRESS 0x45e431ae12LL
#define RF24CHANNEL 0x5e
#define WITH_CHECKSUM 1

BayRF24 client = BayRF24(9, 10);

void setup(void)
{
  Serial.begin(9600);
  client.init(RF24ADDRESS, RF24CHANNEL);
}

long package_count = 1;
void loop(void)
{
  // Construct DataFrame
  client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
  client.addChannelValue((float)millis() / 1000);
  client.addChannelValue(package_count);

  // TODO: Add further channels e.g sensor data
  // Keep in mind that RF24 frames are limited to 32 byte
  // Overhead is 11 byte (with checksum + delayed frames)
  // When you have more than 5 channels you have to send the data in
  // several frames or send data as int16 
  // (value range -31768 ... 31767, e.g send 23.74°C as 2374)
  // client.startDataFrame(BayEOS_Int16le, WITH_CHECKSUM);

#if WITH_CHECKSUM
  client.addChecksum();
#endif
  Serial.print("Sending package #");
  Serial.print(package_count);
  package_count++;
  Serial.print(" ... ");
  if (client.sendPayload())
    Serial.println("failed");
  else
    Serial.println("ok");

  delay(5000);
}
