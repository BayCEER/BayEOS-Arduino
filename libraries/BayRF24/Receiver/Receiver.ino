#include <BayEOS.h>
#include <BayDebug.h>
#include <BaySerial.h>
#include <SPI.h>
#define LPArduino 1
#define GBoard 2
#define GBoardPro 3
//Set the board type here!!
#define BOARD LPArduino
//#define BOARD GBoard
//#define BOARD GBoardPro

#define DEBUG_OUTPUT 1

#if DEBUG_OUTPUT
BayDebug client(Serial);
#else
BaySerial client(Serial);
#endif

#if (BOARD == GBoardPro)
#include <iBoardRF24.h>
iBoardRF24 radio(12, 11, 8, 7, 9, 2);
//Old address style
const uint64_t pipes[6] = { 0x45c431ae12LL, 0x45c431ae24LL, 0x45c431ae48LL,
                            0x45c431ae96LL, 0x45c431aeabLL, 0x45c431aebfLL
                          };

#else
#include <RF24.h>
#if (BOARD == GBoard)
RF24 radio(8,9);
#else
RF24 radio(9,10);
#endif
const uint8_t pipes[6][5] = { 
    {0x12, 0xae, 0x31, 0xc4, 0x45},
    {0x24, 0xae, 0x31, 0xc4, 0x45},
    {0x48},{0x96},{0xab},{0xbf}
};

#endif

#include <printf.h>
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10


/*
const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x96};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};
*/

void setup(void) {
  radio.begin();
  radio.setChannel(0x72);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  //radio.openWritingPipe(pipes[0]);
  for (uint8_t i = 0; i < 6; i++) {
    radio.openReadingPipe(i, pipes[i]);
  }
  /*
  radio.openReadingPipe(0, pipe_0);
  radio.openReadingPipe(1, pipe_1);
  radio.openReadingPipe(2, pipe_2);
  radio.openReadingPipe(3, pipe_3);
  radio.openReadingPipe(4, pipe_4);
  radio.openReadingPipe(5, pipe_5);
  */
#if DEBUG_OUTPUT
  client.begin(9600, 1);
#else
  client.begin(38400);
#endif

  printf_begin();
  Serial.println("Starting");
  radio.printDetails();
  Serial.println("------------");
  client.sendMessage("RF24-Router started");
  radio.startListening();

}

uint8_t nrf_payload[32];
String origin="RF-x";
void loop(void) {
  uint8_t pipe_num, len;
  while( radio.available(&pipe_num) ) {
 //   client.startRoutedFrame(pipe_num, 0);
    origin[3]='0'+pipe_num;
    client.startOriginFrame(origin,1);
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
  delay(1);
}
