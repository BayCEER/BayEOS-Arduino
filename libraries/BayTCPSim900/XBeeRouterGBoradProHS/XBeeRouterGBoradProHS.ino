/*
  GBoardPro XBeeRouter

ATTENTION: THIS IS THE ROUTER BASED ON DEFAULT HARDWARE SERIAL
SENING LANGE XBEE FRAMES MAY RESULT IN HARDWARE BUFFER OVERFLOW 
AND DATA LOSS

  SPEC:
  XBee: Serial3
  GPRS: Serial2 RST: 47 PWR: 46
  Logger: Serial
  SD:CS: 4

  Router - XBee-GPRS with SD-Card Buffer and Watchdog


  This sketch uses a modified HardwareSerial implementation
  It will only complile as long as no access to the default
  serial objects (Serial, Serial1 etc) is made.

  Please use Serial, Serial1 ... instead



*/

#include <HardwareSerial.h>

#define WITH_RF24_RX 0
#define WITH_BAYEOS_LOGGER 0
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

//UTFT-Output
#include <UTFT.h>
#include <TFTStream.h>
#include <BayDebugTFT.h>


#if WITH_RF24_RX
//RF24
#include <digitalWriteFast.h>
#include "iBoardRF24.h"

//GBoard Pro
iBoardRF24 radio(12, 11, 8, 7, 9, 2);
const uint64_t pipes[6] = { 0x45c431ae12LL, 0x45c431ae24LL, 0x45c431ae48LL,
                            0x45c431ae9fLL, 0x45c431aeabLL, 0x45c431aebfLL
                          };
#endif

//UTFT myGLCD(ITDB18SP,33,32,31,35,34); //160x120 Serial
//36+37 for VCC + GND
UTFT myGLCD(ITDB24E_8, 38, 39, 40, 41); //240x320 Parallel
#define UTFT_AUTOOFF 120000 /*ms*/
#define utftcols 30
#define utftrows 26
char utftbuffer[utftrows * (utftcols + 1)];

BayTFTDebug TFT = BayTFTDebug(&myGLCD, utftbuffer, utftrows, utftcols);

#define UTFTprintP(x) utftprintPGM(PSTR(x))
#define UTFTprintlnP(x) utftprintlnPGM(PSTR(x))

#define SENDING_INTERVAL 120000
#define RX_SERIAL Serial3
#define TX_SERIAL Serial2

BayGPRS client = BayGPRS(TX_SERIAL, 46);

#if WITH_BAYEOS_LOGGER
#include <BayEOSLogger.h>
#define LOGGER_BAUD_RATE 38400
BaySerial loggerclient = BaySerial();
BayEOSLogger myLogger;
#endif


//BayDebug client;
XBee xbee_rx = XBee();
BayEOSBufferSDFat myBuffer;
RTC_SIM900 myRTC;

uint16_t rx_panid;

/*
   Create a huge ring buffer to store incoming RX Packages while
   arduino is busy with GPRS...
*/
#define RX_BUFFER_SIZE 1024
unsigned char buffer[RX_BUFFER_SIZE];

unsigned long last_alive, last_send, pos, last_eeprom;
uint16_t rx_ok, rx_error, tx_error;
uint8_t rep_tx_error, tx_res;
uint8_t last_rx_rssi;
uint8_t startupframe, startupsend;

//****************************************************************
// Watchdog Interrupt Service / is executed when  watchdog timed out
volatile uint8_t wdcount = 0;
ISR(WDT_vect) {
  wdcount++;
  if (wdcount > 30) {
    asm volatile (" jmp 0"); //restart programm
  }
}

/*************************************************
   FUNCTIONS
*************************************************/
volatile bool tft_switch = 0;
void tftOn(void) {
  tft_switch = 1;
}

unsigned long tft_autooff;

void utftprintPGM(const char *str) {
  char c;
  while (true) {
    c = pgm_read_byte(str);
    if (!c) break;
    TFT.write(c);
    str++;
  }
}

void utftprintlnPGM(const char *str) {
  utftprintPGM(str);
  TFT.println();
}

void tftSwitchOn(void) {
  //  pinMode(36, OUTPUT);
  //  pinMode(37, OUTPUT);
  //  digitalWrite(37,HIGH);
  //  digitalWrite(36,LOW);
  TFT.lcdOn();
  TFT.begin();
  TFT.flush();
  tft_autooff = millis() + UTFT_AUTOOFF;
}

void tftSwitchOff(void) {
  TFT.end();
  TFT.lcdOff();
  //digitalWrite(37,LOW);
}

void handle_RX_data(void) {
  uint8_t count;
  while (RX_SERIAL.available()) {
    xbee_rx.readPacket();

    if (xbee_rx.getResponse().isAvailable()) {
      switch (xbee_rx.parseRX16(client, rx_panid)) {
        case 0:
          //ok
          rx_ok++;
          client.writeToBuffer();
          if (TFT.isOn()) {
            xbee_rx.parseRX16(TFT, rx_panid);
            TFT.sendPayload();
            TFT.flush();
          }

          break;
        case 1:
          rx_error++;
          break;
        case 2:
          break;
      };
    }
    count++;
    if (count > 20) {
      client.startFrame(BayEOS_ErrorMessage);
      client.addToPayload("Warning - Too much RX-Data");
      client.writeToBuffer();
      return;
    }
  }

}

#if WITH_RF24_RX
void handle_RF24(void) {
  uint8_t pipe_num, len;
  uint8_t payload[32];
  if (  len = radio.readPipe(payload, &pipe_num) ) {
    //Note: RF24 is handelt like XBee with PANID0
    client.startRoutedFrame(pipe_num, 0);
    for (uint8_t i = 0; i < len; i++) {
      client.addToPayload(payload[i]);
    }
    client.writeToBuffer();
    if (TFT.isOn()) {
      TFT.startRoutedFrame(pipe_num, 0);
      for (uint8_t i = 0; i < len; i++) {
        TFT.addToPayload(payload[i]);
      }

      TFT.sendPayload();
      TFT.flush();
    }

  }

}
#endif

void setup(void) {
  attachInterrupt(0, tftOn, CHANGE);
  digitalWrite(18, HIGH); //Pullup for Interrupt INT5
  attachInterrupt(5, tftOn, FALLING);

  tftSwitchOn();
  UTFTprintP("FW ");
  UTFTprintlnP(__DATE__);
  TFT.flush();
#if WITH_BAYEOS_LOGGER
  loggerclient.begin(LOGGER_BAUD_RATE);
#endif

  UTFTprintlnP("Starting XBee... ");
  TFT.flush();
  xbee_rx.setSerial(RX_SERIAL);
  xbee_rx.begin(38400);
  while (!rx_panid) {
    rx_panid = xbee_rx.getPANID();
  }
  UTFTprintP("PANID: ");
  TFT.println(rx_panid);
  TFT.flush();

  startupframe = 1;
  startupsend = 1;
  Sleep.setupWatchdog(9); //init watchdog timer to 8 sec


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
  //   radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_HIGH);
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




