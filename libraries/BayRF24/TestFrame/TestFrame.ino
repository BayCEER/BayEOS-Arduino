#include <BayRF24.h>

#define RF24ADDRESS 0x45e431ae12LL
#define RF24CHANNEL 0x5e
#define WITH_CHECKSUM 1



BayRF24 client = BayRF24(9, 10);

uint8_t number_of_channels = 1;

void setup(void) {
  Serial.begin(9600);
  client.init(RF24ADDRESS, RF24CHANNEL);
}

void loop(void) {
  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
  for (uint8_t i = 0; i < number_of_channels; i++) {
    client.addChannelValue(millis() / 1000);
  }
 
#if WITH_CHECKSUM
  client.addChecksum();
#endif
  Serial.print("Sending ");
  Serial.print(number_of_channels);
  Serial.print(" Channels ");
  if (client.sendPayload())
    Serial.println("failed");
  else
    Serial.println("ok");

  number_of_channels++;
  if(number_of_channels>10) number_of_channels=1;


  delay(2000);

}
