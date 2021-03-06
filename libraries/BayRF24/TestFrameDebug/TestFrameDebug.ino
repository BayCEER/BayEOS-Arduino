#include <BayRF24.h>
#include "printf.h"

//#define RF24ADDRESS 0x45c431ae12LL
//#define RF24ADDRESS 0x45c431ae24LL
//#define RF24ADDRESS 0x45c431ae48LL
#define RF24ADDRESS 0x45e431ae96LL
//#define RF24ADDRESS 0x45c431aeabLL
//#define RF24ADDRESS 0x45c431aebfLL
#define RF24CHANNEL 0x5e

#define WITH_CHECKSUM 1

/* ce,csn pins - adjust to your layout*/
BayRF24 client = BayRF24(9, 10);
//BayRF24 client=BayRF24(8,9); //GBoard


void setup(void) {
  Serial.begin(9600);
  delay(300);
  client.init(RF24ADDRESS, RF24CHANNEL,RF24_PA_MAX);
  printf_begin();
  client.printDetails();

  client.createMessage("TestFrameDebug",WITH_CHECKSUM);
  client.sendPayload();
}

void loop(void) {
  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
  client.addChannelValue(millis() / 1000);
#if WITH_CHECKSUM
  client.addChecksum();
#endif
  if (client.sendPayload())
    Serial.println("failed");
  else
    Serial.println("ok");
  delay(5000);
  client.printDetails();


}
