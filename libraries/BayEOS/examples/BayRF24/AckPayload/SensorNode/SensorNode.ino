#include <BayRF24.h>
BayRF24 client(9, 10);

#define RF24_CHANNEL 0x34
#define RF24_PIPE 0x371f4e2712

#define C_SET_DELAY 0x1

void setup(void) {
  client.init(RF24_PIPE, RF24_CHANNEL);
  Serial.begin(9600);
  Serial.println("Starting");
}

unsigned long delay_time = 16000;
void loop(void) {
  Serial.print("Sending ...  ");
  client.startDataFrame();
  client.addChannelValue(millis());
  client.addChannelValue(delay_time);
  if(client.sendPayload()) Serial.println("failed");
  else Serial.println("ok");
  if (! client.readIntoPayload() &&  client.getPayload(0) == BayEOS_Command) {
    //got a ack paylaod and BayEOS_Command
    uint8_t command = client.getPayload(1);
    switch (command) {
      case C_SET_DELAY:
        memcpy((uint8_t*)&delay_time, client.getPayload() + 2, 4);
        Serial.print("Got new delay time: ");
        Serial.println(delay_time); 
        client.startFrame(BayEOS_CommandResponse);
        client.addToPayload(command);
        client.addToPayload(delay_time);
        client.sendPayload();
        break;
    }
  }

  delay(delay_time);
}
