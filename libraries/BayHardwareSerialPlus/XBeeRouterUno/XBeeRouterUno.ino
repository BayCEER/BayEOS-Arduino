/*
  XBeeRouter Uno

  SPEC:
  XBee: Serial
  RAM-Buffer

  This sketch uses a modified HardwareSerial implementation
  It will only complile as long as no access to the default
  serial objects (Serial, Serial1 etc) is made.

  Please use SerialPlus, SerialPlus1 ... instead
*/
#include <HardwareSerialPlus.h>
#include <XBee.h>
#include <Ethernet.h>
#include <SPI.h>
#include <SdFat.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPEth.h>
#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
#include <BayHardwareSerialPlus.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEA };
byte ip[] = { 132, 180, 112, 17 };
#define RX_SERIAL SerialPlus
XBeePlus xbee_rx = XBeePlus(RX_SERIAL);

BayEth client;
BayEOSBufferRAM  myBuffer;

/*
 * BayEOSRouter.h relies on 
 * TX: client
 * RX: xbee_rx
 * 
 * So please do not change names above...
 * 
 */
#define RX_BUFFER_SIZE 200
#define WITH_RF24_RX 0
#define WITH_BAYEOS_LOGGER 0
#define WITH_TFT 0
#define WITH_WATCHDOG 0
#define SENDING_INTERVAL 30000L
#define NEXT_TRY_INTERVAL 120000L
#include <BayEOSRouter.h>


void setup(void) {
  initRouter();
  client.readConfigFromStringPGM(
    PSTR("192.168.0.1|80|gateway/frame/saveFlat|admin|xbee|DEMO-Router|||||")
  );
  Ethernet.begin(mac, ip);
  myBuffer = BayEOSBufferRAM(3000);
  client.setBuffer(myBuffer);
}

void loop(void) {
  if (millis() - last_alive > 30000) {
    last_alive = millis();
    client.startDataFrame(BayEOS_Float32le);
    client.addToPayload((uint8_t) 0);
    client.addToPayload((float) millis() / 1000);
    client.sendOrBuffer();
    //client.writeToBuffer();
  }

  handle_RX_data();
  client.sendFromBuffer();
}


