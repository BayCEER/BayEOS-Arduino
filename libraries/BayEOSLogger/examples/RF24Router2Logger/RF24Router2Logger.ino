/************************************************
 * 
 * This is a router/logger sketch for the BayEOS-LowPower-Board
 * 
 * It will log received frames to flash storage (e.g. 4M) 
 * 
 * Example Capacity Calculation:
 * 
 * 4 x Sending boards - 60s
 * 8 Channels 2 Byte (16) + Checksum (3) + Data (3) + Orgin (5) + Timestamp (5)
 * -> 32 byte/Frame
 * -> 1.3 MB/week
 * 
 * Serial Readout-Time: 38400 baud:
 * -> 6min/week
 * 
 * Current Calculations
 * 15mA
 * 2200mA,4.2V LiPo -> 6 days
 * 6.6Ah, 12V + Downstepper: 60 days
 * 
 * 
 ************************************************/
#define RF24CHANNEL 0x33
const uint8_t pipes[6][5] = { 
    {0x12, 0xae, 0x31, 0xc4, 0x45},
    {0x24, 0xae, 0x31, 0xc4, 0x45},
    {0x48},{0x96},{0xab},{0xbf}
};

#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BaySerial.h>
BaySerial client(Serial);

#include <BayEOSLogger.h>
BayEOSLogger myLogger;

#include <RF24.h>
RF24 radio(9,10);

#include <LowCurrentBoard.h>

void setup(void) {
  initLCB();
  radio.begin();
  radio.setChannel(RF24CHANNEL);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  //radio.openWritingPipe(pipes[0]);
  for (uint8_t i = 0; i < 6; i++) {
    radio.openReadingPipe(i, pipes[i]);
  }
  radio.powerDown();
  
  myBuffer.init(flash,10);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer); 
  //register all in BayEOSLogger
  myLogger.init(client,myBuffer,myRTC,60); //min_sampling_int = 60
  myLogger._logging_disabled=1; 
  adjust_OSCCAL();
  client.begin(38400);
  startLCB();
  
}

uint8_t nrf_payload[32];
String origin="RF-x";
void loop(void) {
  if(myLogger._logging_disabled && myRTC.now().get()>315360000L){
      myLogger._logging_disabled = 0; 
      radio.powerUp(); 
      radio.startListening();
  }
  uint8_t pipe_num, len;
  while( radio.available(&pipe_num) ) {
    origin[3]='0'+pipe_num;
    client.startOriginFrame(origin,1);
    len = radio.getDynamicPayloadSize();
    radio.read(nrf_payload, len);
    for (uint8_t i = 0; i < len; i++) {
      client.addToPayload(nrf_payload[i]);
    }
    client.writeToBuffer();
    blinkLED(1);
    if(myLogger._mode==LOGGER_MODE_LIVE){
      if(client.sendPayload())
        myLogger._mode=0;
    }
  } 
  
  myLogger.handleCommand();
  myLogger.sendBinaryDump();

}

