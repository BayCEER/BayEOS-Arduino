/*
  GBoardPro XBeeRouter



  SPEC:
  XBee: Serial3
  GPRS: Serial2 RST: 47 PWR: 46
  Logger: Serial
  SD:CS: 4

  Router - XBee-GPRS with SD-Card Buffer and Watchdog


  This sketch uses a modified HardwareSerial implementation
  It will only complile as long as no access to the default
  serial objects (Serial, Serial1 etc) is made.

  Please use SerialPlus, SerialPlus1 ... instead



*/

#include <HardwareSerialPlus.h>

#include <EEPROM.h>
#include <XBee.h>
#include <BayXBee.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <SPI.h>
#include <Base64.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>
#include <BayDebug.h>
#include <BayEOSBuffer.h>
#include <SdFat.h>
#include <BayEOSBufferSDFat.h>
#include <Sleep.h>
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <BayHardwareSerialPlus.h>


#define SENDING_INTERVAL 120000
#define RX_SERIAL SerialPlus3
#define TX_SERIAL SerialPlus2

BayGPRSPlus client = BayGPRSPlus(TX_SERIAL, 46);

//BayDebug client;
XBeePlus xbee_rx = XBeePlus();
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
#define RF24_CHANNEL 0x72
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
  client.readConfigFromFile("GPRS.TXT");
  for (uint8_t i = 1; i <= 10; i++) {
    TFT.print(i);
    UTFTprintP(": ");
    TFT.println(*client.getConfigPointer((i - 1)));
  }
  TFT.flush();
  //  client.softSwitch();
  if (*(*client.getConfigPointer(1) + 2) == '9') {
    client._urlencode = 0;
    UTFTprintlnP("Using old port 8090 without urlencode");
    TFT.flush();
  }


#if WITH_BAYEOS_LOGGER
  //Write Name to EEPROM (for BayLogger!)
  EEPROM.write(EEPROM_NAME_OFFSET, EEPROM_NAME_STARTBYTE);
  uint8_t i = 0;
  char c;
  while (c = *(*client.getConfigPointer(2) + i)) {
    EEPROM.write(EEPROM_NAME_OFFSET + i + 1, c);
    i++;
  }
  EEPROM.write(EEPROM_NAME_OFFSET + i + 1, 0);
#endif

  UTFTprintP("Starting GPRS...");
  TFT.flush();
  switch (client.begin(38400)) {
    case 0:
      UTFTprintlnP("OK");
      break;
    case 1:
      UTFTprintlnP("Communication Error");
      break;
    case 2:
      UTFTprintlnP("PIN Error");
      break;
    case 3:
      UTFTprintlnP("PIN Locked");
      break;
    case 4:
      UTFTprintlnP("No Network");
      break;
    case 5:
      UTFTprintlnP("No GPRS");
      break;
    case 6:
      UTFTprintlnP("No SIM Card");
      break;
  }
  TFT.flush();

  myRTC.adjust(client.now());

  myBuffer = BayEOSBufferSDFat(2000000000, 1); //Append mode!
  UTFTprintlnP("Buffer Ok");
  UTFTprintP("Size: ");
  pos = myBuffer.readPos();
  TFT.println(pos);
  TFT.flush();


  myBuffer.setRTC(myRTC, 0); //Relative Mode...
  client.setBuffer(myBuffer, 0);
#if WITH_BAYEOS_LOGGER
  loggerclient.setBuffer(myBuffer);
  myLogger.init(loggerclient, myBuffer, myRTC);
#endif
  myBuffer.seekReadPointer(pos); //Logger.init moves Read pointer!


  UTFTprintP("RSSI:");
  TFT.println(client.getRSSI());

  UTFTprintlnP("Setup ok :-)");
  TFT.flush();

  client.startFrame(BayEOS_Message);
  client.addToPayload("Router started - FW ");
  client.addToPayload(__DATE__);
  client.writeToBuffer();

  tft_autooff = millis() + UTFT_AUTOOFF;


#if WITH_RF24_RX == 1
  radio.begin();
  radio.setChannel(0x71);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setRetries(15,15);
  for (uint8_t i = 0; i < 6; i++) {
    radio.openReadingPipe(i, pipes[i]);
  }
  radio.startListening();
#endif

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

#if WITH_BAYEOS_LOGGER
  //Save read pos every hour
  if ( ((millis() - last_eeprom) > 3600000 )  || startupframe) {
    pos = myBuffer.readPos();
    for (uint8_t i = 0; i < 4; i++) {
      EEPROM.write(EEPROM_READ_POS_OFFSET + i, *(&pos + i));
    }
    last_eeprom = millis();
  }
#endif

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
    client.addChannelValue(myRTC.now().get());
    client.addChannelValue(client.now().get());
    client.addChannelValue(client.getRSSI());
    client.addChannelValue((float)analogRead(A4) / 1023 * 3.3 * 10);
    client.writeToBuffer();
  }

  if (
#if WITH_BAYEOS_LOGGER
    ! myLogger._mode &&
# endif
    ((millis() - last_send) > SENDING_INTERVAL || myBuffer.available() > 2000 || startupsend)) {
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
      unsigned long simTime = client.now().get();
      unsigned long rtcTime = myRTC.now().get();
      if ((simTime > rtcTime && (simTime - rtcTime) < 120)
          || (simTime > rtcTime && (rtcTime - simTime) < 120) ) {
        UTFTprintlnP("setting RTC");
        myRTC.adjust(DateTime(simTime));
      }

    }
    TFT.flush();

    if (rep_tx_error % 5 == 4) {
      client.softSwitch();
      client.startFrame(BayEOS_Message);
      client.addToPayload("TX-ERROR SoftSwitch");
      client.writeToBuffer();

    }
  }

  handle_RX_data();

#if WITH_RF24_RX 
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




