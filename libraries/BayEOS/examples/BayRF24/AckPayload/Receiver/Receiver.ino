#include <BayDebug.h>
BayDebug client(Serial);
#include <BayRF24.h>
BayRF24 radio(9, 10, 0);//no power down

#define RF24_CHANNEL 0x34
#define RF24_PIPE 0x371f4e2712

#define C_SET_DELAY 0x1

void setup(void) {
  radio.init(RF24_PIPE,RF24_CHANNEL,0); 
  radio.openReadingPipe(0, RF24_PIPE);
  radio.startListening();
  client.begin(9600, 1);
  Serial.println("Starting");

}

void loop(void) {
  while (! radio.readIntoPayload() ) {
    //got a frame - copy frame to BayDebug
    client.startFrame();
    client.addToPayload(radio.getPayload(), radio.getPacketLength());
    Serial.print("GOT: ");
    client.sendPayload();
  }

  if (Serial.available()) {
    //We want to change the delay time on the sensor node
    char c = Serial.read(); //Read first Letter
    delay(10);
    while (Serial.available()) {
      delay(1);
      Serial.read();
    }
    unsigned long delay_time;
    switch (c) {
      case '1':
        delay_time = 1000;
        break;
      case '2':
        delay_time = 2000;
        break;
      case '3':
        delay_time = 4000;
        break;
      case '4':
        delay_time = 8000;
        break;
      default:
        delay_time = 16000;
    }
    //Prepare a BayEOS_Command-Frame
    Serial.print("Set delay_time: ");
    Serial.println(delay_time);
    radio.startFrame(BayEOS_Command);
    radio.addToPayload((uint8_t) C_SET_DELAY);
    radio.addToPayload(delay_time);
    //Write BayEOS-Command-Frame to ackPayload of pipe 0
    radio.writeAckPayload(0,radio.getPayload(),radio.getPacketLength());
  }
  delay(1);
}
