#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
#include "printf.h"

#define RF24ADDRESS 0x45c431ae24LL
#define RF24CHANNEL 0x72
#define WITH_CHECKSUM 1

/* ce,csn pins - adjust to your layout*/
BayRF24 client = BayRF24(9, 10);
#define ACTION_COUNT 1
#define SAMPLING_INT 4
#include <LowCurrentBoard.h>

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting Test");
  myBuffer.init(flash); //This will restore old pointers
  Serial.print("Flash: ");
  Serial.println(flash.getCapacity());
  Serial.flush();
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); //use skip!
  initLCB(); //init time2
  client.init(RF24ADDRESS, RF24CHANNEL, RF24_PA_MAX);
  printf_begin();
  client.printDetails();

  client.createMessage("TestFrameDebug", WITH_CHECKSUM);
  client.sendPayload();
  Serial.println("Test done");
  Serial.flush();
  startLCB();

}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    client.startDataFrame(BayEOS_Float32le, WITH_CHECKSUM);
    client.addChannelValue(millis() / 1000);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    if (client.sendPayload())
      Serial.println("failed");
    else
      Serial.println("ok");
    client.printDetails();
    Serial.flush();
  }
}
