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
#define WITH_RF24_RX 0
#define RF24_CHANNEL 0x72
#define WITH_BAYEOS_LOGGER 0
#define WITH_TFT 1
#define WITH_WATCHDOG 1
#define WITH_RX_XBEE 1


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


#define SENDING_INTERVAL 120000L
#define NEXT_TRY_INTERVAL 1200000L
#define MAX_BUFFER_AVAILABLE 1000
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
   BayEOSRouter.h relies on
   TX: client
   RX: xbee_rx

   So please do not change names above...

*/
#define WITH_RF24_RX 1
/*
  #define RF24_CHANNEL 72
  #define RF24_PIPES
  const uint64_t pipes[6] = {0x45c431ae12LL, 0x45c431ae24LL, 0x45c431ae48LL,
  0x45c431ae96LL, 0x45c431aeabLL, 0x45c431aebfLL
  };
*/
#define WITH_BAYEOS_LOGGER 0
#define WITH_TFT 1
#define WITH_WATCHDOG 1
#include <BayEOSRouter.h>



void setup(void) {
  initRouter();


  while(!SD.begin(4)) {
#if WITH_TFT
    UTFTprintlnP("No SD.");
    TFT.flush();
    delay(1000);
#endif
  }

#if WITH_TFT
  UTFTprintlnP("READ config");
  TFT.flush();
#endif
  client.readConfigFromFile("ETH.TXT");
#if WITH_TFT
  for (uint8_t i = 1; i <= 10; i++) {
    TFT.print(i);
    UTFTprintP(": ");
    TFT.println(*client.getConfigPointer((i - 1)));
  }
  TFT.flush();
#endif
  memcpy(mac, client.parseMAC(*client.getConfigPointer(BayTCP_CONFIG_MAC)), 6);
  memcpy(ip, client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_IP)), 4);
  memcpy(mask, client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_MASK)), 4);
  memcpy(default_gw, client.parseIP(*client.getConfigPointer(BayTCP_CONFIG_DEFAULT_GW)), 4);
  if (*(*client.getConfigPointer(1) + 2) == '9') {
    client._urlencode = 0;
#if WITH_TFT
    UTFTprintlnP("Using old port 8090 without urlencode");
    TFT.flush();
#endif
  }
  if (ip[0])
    Ethernet.begin(mac, ip, default_gw, default_gw, mask);
  else
    Ethernet.begin(mac);



  myBuffer = BayEOSBufferSDFat(2000000000, 1); //Append mode!
  pos = myBuffer.readPos();
#if WITH_TFT
  UTFTprintlnP("Buffer Ok");
  UTFTprintP("Size: ");
  TFT.println(pos);
  TFT.flush();
#endif

  client.setBuffer(myBuffer, 0);
#if WITH_BAYEOS_LOGGER
  loggerclient.setBuffer(myBuffer);
  myLogger.init(loggerclient, myBuffer, myRTC);
#endif
  myBuffer.seekReadPointer(pos); //Logger.init moves Read pointer!

#if WITH_TFT
  UTFTprintlnP("Setup ok :-)");
  TFT.flush();
#endif

  client.startFrame(BayEOS_Message);
  client.addToPayload("Router started - FW ");
  client.addToPayload(__DATE__);
  client.writeToBuffer();

  tft_autooff = millis() + UTFT_AUTOOFF;



}

void loop(void) {
  wdcount = 0; //clear watchdog count!

#if WITH_TFT
  if ((tft_autooff - millis()) > UTFT_AUTOOFF) {
    tftSwitchOff();
  }

  if (tft_switch) {
    tft_switch = 0;
    if (! TFT.isOn())  tftSwitchOn();
    tft_autooff = millis() + UTFT_AUTOOFF;

  }
#endif

  if (millis() > next_alive) {
    next_alive = millis() + SENDING_INTERVAL;
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myBuffer.readPos());
    client.addChannelValue(tx_error);
    client.addChannelValue(rx_ok);
    client.addChannelValue(rx_error);
    client.addChannelValue((float)analogRead(A4) / 1023 * 3.3 * 10);
    client.writeToBuffer();
  }



  if ((tx_error == 0 && (millis() > next_send || myBuffer.available() > MAX_BUFFER_AVAILABLE) )
      || (tx_error && millis() > next_try) ) {
    sendData();
  }

#if WITH_RX_XBEE
  handle_RX_data();
#endif

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




