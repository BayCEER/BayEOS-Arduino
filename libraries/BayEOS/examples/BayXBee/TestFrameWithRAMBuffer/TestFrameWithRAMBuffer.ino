#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>



BayXBee client = BayXBee(Serial);
BayEOSBufferRAM myBuffer;
unsigned long last_data;

void setup(void) {
  client.begin(38400);
  myBuffer = BayEOSBufferRAM(1000);
  client.setBuffer(myBuffer);
}

void loop(void) {
  //Create a BayEOS Frame every five seconds
  //while waiting try to resend buffered frames

  if ((millis() - last_data) > 5000) {

    client.sendFromBuffer();

    last_data = millis();
    client.startDataFrame(0x41);
    for (uint8_t i = 0; i < 5; i++) {
      client.addToPayload(i);
      client.addToPayload((float) i);
    }
    client.sendOrBuffer();
  }

}
