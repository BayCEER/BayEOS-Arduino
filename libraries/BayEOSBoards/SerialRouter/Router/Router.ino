/*
   Sketch for BayEOS-SerialRouter Board

   You can attach other BayEOS sensor boards to the router board
   Frames from sensor boards should be sent via Routed Origin Frames
   When a sensor boards want to sent the have to check the CTS line (usually pin 9). When
   it is high, it pulls it low. After transmission the boards have to
   release the CTS line and call Serial.end() to release the TX pin

*/

#define GPRS_CONFIG "132.180.112.55|80|gateway/frame/saveFlat|import|import|Serial-R|pinternet.interkom.de|||4814|"
//#define WLAN_CONFIG "132.180.112.55|80|gateway/frame/saveFlat|import|import|WLAN-RB|SSID|PASSWORD|||"

#define SAMPLING_INT 128

/* UNCOMMENT IF YOU WANT TO USE RF24 in addition to serial
  define NRF24_CHANNEL 0x2b
  #define WITH_RF24_CHECKSUM 1
  const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
  const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
  const uint8_t pipe_2[] = {0x48};
  const uint8_t pipe_3[] = {0x96};
  const uint8_t pipe_4[] = {0xab};
  const uint8_t pipe_5[] = {0xbf};
*/
#include <SerialRouterBoard.h>

void setup()
{
  initLCB();
#ifdef NRF24_CHANNEL
  initRF24();
#endif
  startLCB();
}

void loop()
{
#ifdef NRF24_CHANNEL
  if (gprs_status)
    handleRF24();
#endif
  checkAction0();
  readRX();
   
}

