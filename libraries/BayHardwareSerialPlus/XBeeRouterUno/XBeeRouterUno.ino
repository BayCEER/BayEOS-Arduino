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
uint16_t rx_ok, rx_error, tx_error;
#define RX_SERIAL SerialPlus
XBeePlus xbee_rx = XBeePlus(RX_SERIAL);

#define RX_BUFFER_SIZE 200
unsigned char buffer[RX_BUFFER_SIZE];

BayEth client;
BayEOSBufferRAM  myBuffer;
uint16_t rx_panid;
unsigned long last_alive;

void setup(void) {
  RX_SERIAL.setRxBuffer(buffer, RX_BUFFER_SIZE);
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);
  client.readConfigFromStringPGM(
    PSTR("192.168.0.1|80|gateway/frame/saveFlat|admin|xbee|DEMO-Router|||||")
  );
  Ethernet.begin(mac, ip);
  while (! rx_panid) rx_panid = xbee_rx.getPANID();
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


void handle_RX_data(void) {
  if (RX_SERIAL.available()) {
    xbee_rx.readPacket();

    if (xbee_rx.getResponse().isAvailable()) {
      switch (xbee_rx.parseRX16(client, rx_panid)) {
        case 0:
          //ok
          rx_ok++;
          client.sendOrBuffer();

          break;
        case 1:
          rx_error++;
          break;
        case 2:
          break;
      };
    }
  }

}

