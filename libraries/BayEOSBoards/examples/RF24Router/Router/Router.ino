/*
 * Sketch for BayEOS-RF24-Router Board
 * 
 * The Board can take up to three RF24-modules. 
 * 
 */

//GPRS-CONFIG-String
#define SIM800_CONFIG "http://132.180.112.55/gateway/frame/saveFlat|gw-user|gw-pw|GPRS-RB|pinternet.interkom.de|||1812|"
 
//ESP01-Config
//#define ROUTER_NAME "ESP01Router"
//#define GATEWAY_IP "192.168.2.2"
//#define GATEWAY_USER "import"
//#define GATEWAY_PW "ChangeME"



#define SAMPLING_INT 128
#define NRF24_CHANNEL 0x2b
//Comment Channel 2 and 3 in case you did not install a module
//#define NRF24_2CHANNEL 0x4b
//#define NRF24_3CHANNEL 0x6b
#define WITH_RF24_CHECKSUM 1
const uint8_t pipe_0[] = {0x12, 0x79, 0x43, 0x6c, 0xf3};
const uint8_t pipe_1[] = {0x24, 0x79, 0x43, 0x6c, 0xf3};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x96};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};

#define EEPROM_BUFFER_STATUS_BYTE 0xc0 /* indicates whether there is valid data in the flash buffer */
// Change the EEPROM_BUFFER_STATUS_BYTE if you want the board to reset the buffer once

#include <RF24RouterBoard.h>

void setup()
{
  initLCB();
  initRF24();
}

void loop()
{
  if (gprs_status)
    handleRF24();
  checkAction0();
  sleepLCB();
}
