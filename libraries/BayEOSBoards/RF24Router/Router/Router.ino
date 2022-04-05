/*
 * Sketch for BayEOS-RF24-Router Board
 * 
 * The Board can take up to three RF24-modules. Power consumption with a lipo battery is about 2Ah/d
 * 
 */

#define GPRS_CONFIG "132.180.112.55|80|gateway/frame/saveFlat|import|import|GPRS-RB|pinternet.interkom.de|||1812|"
//#define POWER_PIN 6
//#define WLAN_CONFIG "192.168.2.108|80|gateway/frame/saveFlat|import|import|WLAN-RB|WIFISSID|secretPW|||"

#define SAMPLING_INT 128
#define NRF24_CHANNEL 0x2b
//Comment Channel 2 and 3 in case you did not install a module
//#define NRF24_2CHANNEL 0x4b
//#define NRF24_3CHANNEL 0x6b
#define WITH_RF24_CHECKSUM 1
const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x96};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};

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
