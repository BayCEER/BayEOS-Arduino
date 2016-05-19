/*

  BayEOSnRF24-Router

*/

#include <BayEOS.h>
#include <BayDebug.h>
#include <SPI.h>
#include <RF24.h>
BayDebug client = BayDebug(Serial);
#include <printf.h>
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9, 10);

//
// Topology
//
/*
const uint64_t pipes[6] = {0x45c431ae12LL, 0x45c431ae24LL, 0x45c431ae48LL,
  0x45c431ae96LL, 0x45c431aeabLL, 0x45c431aebfLL
};
*/
const uint64_t pipes[6] = { 0x85c831a312LL, 0x85c831a324LL, 0x85c831a348LL,
                            0x85c831a39fLL, 0x85c831a3abLL, 0x85c831a3bfLL
                          };


void setup(void) {
  radio.begin();
  radio.setChannel(0x61);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  for (uint8_t i = 0; i < 6; i++) {
    radio.openReadingPipe(i, pipes[i]);
  }
  radio.startListening();
  client.begin(9600, 1);
  printf_begin();
  Serial.println("Starting");
  radio.printDetails();
  Serial.println("------------");
  client.sendMessage("RF24-Router started");

}

uint8_t nrf_payload[32];
void loop(void) {
  uint8_t pipe_num, len;
  if ( radio.available(&pipe_num) ) {
    client.startRoutedFrame(pipe_num, 0);
    len = radio.getDynamicPayloadSize();
    radio.read(nrf_payload, len);
    Serial.print("GOT: ");
    for (uint8_t i = 0; i < len; i++) {
      Serial.print(nrf_payload[i], HEX);
      client.addToPayload(nrf_payload[i]);
    }
    Serial.println();
    client.sendPayload();
  }

}
