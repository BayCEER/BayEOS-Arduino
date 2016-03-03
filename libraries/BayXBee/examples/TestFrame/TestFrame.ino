#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>


BayXBee client = BayXBee(Serial);


void setup(void) {
  client.begin(38400);

}

void loop(void) {
  client.sendMessage("Just a message ;-)");

  //Construct DataFrame
  client.startDataFrame();
  client.addChannelValue(millis() / 1000);
  client.addChannelValue(analogRead(A0));
  client.sendPayload();


  client.sendError("Just a test error message ;-)");

  delay(5000);

}
