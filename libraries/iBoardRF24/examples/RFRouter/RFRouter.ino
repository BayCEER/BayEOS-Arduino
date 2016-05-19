#include <SPI.h>
#include <digitalWriteFast.h>
#include "iBoardRF24.h"
#include <BayEOS.h>
#include <BayDebug.h>
#include <BaySerial.h>
#include "printf.h"

iBoardRF24 radio(12, 11, 8, 7, 9, 2); //GBoard Pro
//BayDebug client=BayDebug(Serial);
BaySerial client = BaySerial(Serial);

const uint64_t pipes[6] = {0x45c431ae12LL, 0x45c431ae24LL, 0x45c431ae48LL,
                           0x45c431ae96LL, 0x45c431aeabLL, 0x45c431aebfLL
                          };
void setup(void) {
  radio.begin();
  radio.setChannel(0x71);
  radio.enableDynamicPayloads();
  //   radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  for (uint8_t i = 0; i < 6; i++) {
    radio.openReadingPipe(i, pipes[i]);
  }
  radio.startListening();

  client.begin(9600);
  printf_begin();
  radio.printDetails();
  client.sendMessage("RF24-Router started");


}

uint8_t payload[32];
void loop(void) {
  uint8_t pipe_num, len;
  if ( radio.readPipe(payload, &pipe_num) ) {
    client.startRoutedFrame(pipe_num, 0);
    for (uint8_t i = 0; i < len; i++) {
      client.addToPayload(payload[i]);
    }
    client.sendPayload();


  }

}
