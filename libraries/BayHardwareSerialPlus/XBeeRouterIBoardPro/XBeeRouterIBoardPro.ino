/*
  IBoardPro XBeeRouter

  SPEC:
  XBee: Serial3
  SD:CS: 4

  Router - XBee-Ethernet with SD-Card Buffer and Watchdog

  This sketch uses a modified HardwareSerial implementation
  It will only complile as long as no access to the default
  serial objects (Serial, Serial1 etc) is made.

  Please use SerialPlus, SerialPlus1 ... instead


*/

#include <HardwareSerialPlus.h>

#include <EEPROM.h>
#include <XBee.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <SPI.h>
#include <Base64.h>
#include <BayTCP.h>
#include <Ethernet.h>
#include <BayTCPEth.h>
#include <BayDebug.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <Sleep.h>
#include <BayHardwareSerialPlus.h>


#define SENDING_INTERVAL 120000
#define RX_SERIAL SerialPlus3

BayEth client;
byte mac[6];
byte ip[4];
byte mask[4];
byte default_gw[4];

XBeePlus xbee_rx = XBeePlus(RX_SERIAL);
BayEOSBufferSDFat myBuffer;
RTC_SIM900 myRTC;


/*
   Create a huge ring buffer to store incoming RX Packages while
   arduino is busy with GPRS...
*/
#define RX_BUFFER_SIZE 1024
unsigned char buffer[RX_BUFFER_SIZE];

/*
 * BayEOSRouter.h relies on 
 * TX: client
 * RX: xbee_rx
 * 
 * So please do not change names above...
 * 
 */
#define WITH_RF24_RX 1
/*
#define RF24_CHANNEL 72
#define RF24_PIPES
const uint64_t pipes[6] = {0x45c431ae12LL, 0x45c431ae24LL, 0x45c431ae48LL,
  0x45c431ae9fLL, 0x45c431aeabLL, 0x45c431aebfLL
};
*/
#define WITH_BAYEOS_LOGGER 0
#define WITH_TFT 1
#define WITH_WATCHDOG 1
#include <BayEOSRouter.h>



void setup(void) {
  initRouter();


  if (!SD.begin(4)) {
    UTFTprintlnP("No SD.");
    TFT.flush();
    delay(1000);
    return;
  }

  UTFTprintlnP("READ config");
  TFT.flush();
  client.readConfigFromFile("ETH.TXT");
  for (uint8_t i = 1; i <= 10; i++) {
    TFT.print(i);
    UTFTprintP(": ");
    TFT.println(*client.getConfigPointer((i - 1)));
  }
  TFT.flush();
  memcpy(mac, client.parseMAC(*client.getConfigPointer(BayTCP_CONFIG_MAC)), 6);
  memcpy(ip, client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_IP)), 4);
  memcpy(mask, client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_MASK)), 4);
  memcpy(default_gw, client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_DEFAULT_GW)), 4);
  if (*(*client.getConfigPointer(1) + 2) == '9') {
    client._urlencode = 0;
    UTFTprintlnP("Using old port 8090 without urlencode");
    TFT.flush();
  }
  if (ip[0])
    Ethernet.begin(mac, ip, default_gw, default_gw, mask);
  else
    Ethernet.begin(mac);



  myBuffer = BayEOSBufferSDFat(2000000000, 1); //Append mode!
  UTFTprintlnP("Buffer Ok");
  UTFTprintP("Size: ");
  pos = myBuffer.readPos();
  TFT.println(pos);
  TFT.flush();


  client.setBuffer(myBuffer, 0);
#if WITH_BAYEOS_LOGGER
  loggerclient.setBuffer(myBuffer);
  myLogger.init(loggerclient, myBuffer, myRTC);
#endif
  myBuffer.seekReadPointer(pos); //Logger.init moves Read pointer!


  UTFTprintlnP("Setup ok :-)");
  TFT.flush();

  client.startFrame(BayEOS_Message);
  client.addToPayload("Router started - FW ");
  client.addToPayload(__DATE__);
  client.writeToBuffer();

  tft_autooff = millis() + UTFT_AUTOOFF;



}

void loop(void) {
  wdcount = 0; //clear watchdog count!

  if ((tft_autooff - millis()) > UTFT_AUTOOFF) {
    tftSwitchOff();
  }

  if (tft_switch) {
    tft_switch = 0;
    if (! TFT.isOn())  tftSwitchOn();
    tft_autooff = millis() + UTFT_AUTOOFF;
  }


  if ((millis() - last_alive) > SENDING_INTERVAL || startupframe) {
    startupframe = 0;
    last_alive = millis();
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myBuffer.readPos());
    client.addChannelValue(tx_error);
    client.addChannelValue(rx_ok);
    client.addChannelValue(rx_error);
    client.addChannelValue((float)analogRead(A4) / 1023 * 3.3 * 10);
    client.writeToBuffer();


    if (((millis() - last_send) > SENDING_INTERVAL || myBuffer.available() > 2000 || startupsend)) {
      last_send = millis();
      UTFTprintP("Sending ");
      if ( (tx_res = client.sendMultiFromBuffer()) ) {
        UTFTprintP("failed - ");
        TFT.println(tx_res);
        tx_error++;
        rep_tx_error++;

      } else {
        rep_tx_error = 0;
        startupsend = 0;
        UTFTprintlnP("OK");
      }

    }
    TFT.flush();

  }

  handle_RX_data();

#if WITH_RF24_RX == 1
  handle_RF24();
#endif

#if WITH_BAYEOS_LOGGER
  myLogger.liveData(1000);
  myLogger.handleCommand();
  //Send data stored on SD
  //Just one frame each time...
  myLogger.sendData();
  //Send binary dump of SD-file
  //Just one part each time
  myLogger.sendBinaryDump();
#endif

}




