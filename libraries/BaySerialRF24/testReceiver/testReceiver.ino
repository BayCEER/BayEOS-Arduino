#include <BaySerialRF24.h>
#include <BayDebug.h>

RF24 radio(9,10);
BaySerialRF24 client(radio);
BayDebug db(Serial);

const uint8_t channel=0x62; //Same as sender
//RX and TX musst be opposite to sender
const uint8_t rx_adr[5] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t tx_adr[5] = {0x24, 0xae, 0x31, 0xc4, 0x45};

void setup() {
  db.begin(9600);
  Serial.println("Starting...");
  client.begin(channel,rx_adr, tx_adr);
}

void loop(){
   if(! client.readIntoPayload()){
  //got something...
      db.startFrame(client.getPayload(0));
      for(uint8_t i=1;i<client.getPacketLength();i++){
         db.addToPayload(client.getPayload(i));
      }
      db.sendPayload();
   } else 
    Serial.println("timeout");

  
}
  
