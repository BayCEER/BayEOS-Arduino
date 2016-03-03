#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <BayEOS.h>
#include <BayDebug.h>
#include <XBee.h>
#include <BayXBee.h>


//BayXBee client=BayXBee(Serial);
BayDebug client;
BayEOSBufferSDFat myBuffer;
unsigned long last_data;
unsigned long last_buffered_data;


void setup(void) {
  client.begin(9600);
  client.sendMessage("starting");
  //Set 4 for EthernetShield, 10 for Stalker
  if (!SD.begin(4)) {
    return;
  }

  myBuffer = BayEOSBufferSDFat(100000000);
  client.setBuffer(myBuffer);
}

void loop(void) {

  //Resend buffered frames
  //one every second
  //sending to frequently my make xbee operationable (channel overload)
  if ((millis() - last_buffered_data) > 1000) {
    client.sendFromBuffer();
    last_buffered_data = millis();
  }

  if ((millis() - last_data) > 5000) {
    last_data = millis();


    //Construct DataFrame
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    client.sendOrBuffer();

    //Construct Message
    client.startFrame(BayEOS_Message);
    client.addToPayload("Just a message ;-)");
    client.sendOrBuffer();
  }

}
