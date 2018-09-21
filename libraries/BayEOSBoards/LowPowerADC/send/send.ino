#define SAMPLING_INT 32
#define RF24CHANNEL 0x7e
#define RF24ADDRESS 0x45c431ae48LL
#define WITH_CHECKSUM 1
#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

void setup()
{
  pinMode(7, OUTPUT);
  analogReference(DEFAULT);

  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 120); //use skip!
  initLCB(); //init time2
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    client.startDataFrame(BayEOS_Int16le, WITH_CHECKSUM);
    client.addChannelValue(millis());
    digitalWrite(7, HIGH);
    delayLCB(100); //Add a delay to allow sensors to power up - Board sleeps at this point
    client.addChannelValue(3300.0 * analogRead(A7) / 1023 * ((100.0 + 100.0) / 100.0)); //Battery Voltage with a 100k:100k Divider
    for (uint8_t i = 0; i < 6; i++) {
      client.addChannelValue(analogRead(A0 + i));
    }
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}



