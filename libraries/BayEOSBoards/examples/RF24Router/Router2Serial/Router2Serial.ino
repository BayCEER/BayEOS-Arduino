/*
 * Sketch for BayEOS-RF24-Router Board
 *
 * The Board can take up to three RF24-modules. 
 *
 */

#define NRF24_CHANNEL 0x2b
// Comment Channel 2 and 3 in case you did not install a module
// #define NRF24_2CHANNEL 0x4b
// #define NRF24_3CHANNEL 0x6b
#define WITH_RF24_CHECKSUM 1
const uint8_t pipe_0[] = {0x12, 0x79, 0x43, 0x6c, 0xf3};
const uint8_t pipe_1[] = {0x24, 0x79, 0x43, 0x6c, 0xf3};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x96};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};

#define SERIAL_CLIENT 1
#define DEBUG_CLIENT 1

#include <RF24RouterBoard.h>

void setup()
{
  initLCB();
  initRF24();
}

void loop()
{
  handleRF24();
  client.sendFromBuffer();
}
