#include <BayEOS.h>
#include <XBee.h>
#include <BayXBee.h>


BayXBee client = BayXBee(Serial1);


void setup(void) {
  client.begin(9600);
  Serial.begin(9600);

}

int count=0;
void loop(void) {
/*  if(client.sendMessage("Just a message ;-)"))
    Serial.println("Send failed");
  else
    Serial.println("Send ok");
  */

  //Construct DataFrame
  client.startDataFrame();
  
  client.addChannelValue(count);
  client.addChannelValue(millis() / 1000);
  client.addChannelValue(analogRead(A0));
  client.sendPayload();
  count++;
/*
  client.sendError("Just a test error message ;-)");
*/
  delay(60000);

}
