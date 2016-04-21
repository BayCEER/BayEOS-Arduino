/****************************************************
* nRF24l01+ Router
*
*****************************************************/

#include <BayEOSBuffer.h>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <BayEOS.h>
#include <Sleep.h>
#include <SoftwareSerial.h>
#include <RTClib.h>
#include <Base64.h>
#include <SdFat.h>
#include <BayTCP.h>
#include <BayTCPSim900.h>

#define NRF24_CHANNEL 0x72

#include <RF24.h>
RF24 radio(9, 10);
const uint8_t pipe_0[] = {0x12, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_1[] = {0x24, 0xae, 0x31, 0xc4, 0x45};
const uint8_t pipe_2[] = {0x48};
const uint8_t pipe_3[] = {0x9f};
const uint8_t pipe_4[] = {0xab};
const uint8_t pipe_5[] = {0xbf};
uint16_t rx_ok, rx_error, tx_error;

BayGPRS client=BayGPRS(Serial,2);
BayEOSBufferEEPROM myBuffer;

#define SAMPLING_INT 64
#define LCB_BAT_MULTIPLIER 1.1*540/100/1023
// we will collect 120 measurements before we try to send

#define ACTION_COUNT 2
#define LED_PIN 5
#include <LowCurrentBoard.h>

uint8_t res;

void initRF24(void) {
  radio.powerUp();
  radio.setChannel(NRF24_CHANNEL);
  radio.setPayloadSize(32);
  radio.enableDynamicPayloads();
  radio.setCRCLength( RF24_CRC_16 ) ;
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(15, 15);
  radio.setAutoAck(true);
  //  radio.openWritingPipe(pipe_0);
  radio.openReadingPipe(0, pipe_0);
  radio.openReadingPipe(1, pipe_1);
  radio.openReadingPipe(2, pipe_2);
  radio.openReadingPipe(3, pipe_3);
  radio.openReadingPipe(4, pipe_4);
  radio.openReadingPipe(5, pipe_5);

  radio.startListening();
}


void setup()
{
  initLCB();
  //CHANGE CONFIG!!
  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|LP-GPRS|pinternet.interkom.de|||1812|"));
  blinkLED(2); 
  res=client.begin(38400);
  blinkLED(res+1); 
  /*
   * 1 == OK
   * 2 == NO Communication
   * 3 == PIN failed
   * 4 == PIN locked
   * 5 == Not CREG
   * 6 == Not CGATT
   * 7 == No SIM Card
  */
  delay(2000);
  
  res=client.sendMessage("GPRS started");
  blinkLED(res+1);
  
  /*
   * 1 == OK
   * 2 == no success
   * 3 == timeout
   * 4 == network timeout
   * 5 == gprs modem timeout
  */ 
  
 
  Wire.begin();
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); 
  readBatLCB(); 
  startLCB();
  radio.begin();
  initRF24();
}


void loop()
{
  handleRtcLCB();

  if(ISSET_ACTION(0)){
    UNSET_ACTION(0);
    client.startDataFrame(BayEOS_ChannelFloat32le);
    client.addChannelValue(millis(),1);
    client.addChannelValue(batLCB,2);
    client.addChannelValue(rx_ok,3);
    client.addChannelValue(rx_error,4);
    client.addChannelValue(tx_error,5);
    client.writeToBuffer();
    readBatLCB(); 
  }
  if(ISSET_ACTION(1)){
    UNSET_ACTION(1);
    if(res=client.sendMultiFromBuffer(2000)) tx_error++;
    else tx_error=0;
    blinkLED(res+1);
  }

  handle_RF24();  
  sleepLCB();
}
    
void handle_RF24(void) {
  uint8_t pipe_num, len;
  uint8_t payload[32];
  uint8_t count;
  while (radio.available(&pipe_num)) {
    count++;
    if (len = radio.getDynamicPayloadSize()) {
      client.startRoutedFrame(pipe_num, 0);
      // Fetch the payload
      radio.read( payload, len );
      for (uint8_t i = 0; i < len; i++) {
        client.addToPayload(payload[i]);
      }
      client.writeToBuffer();
 
    } else {
      rx_error++;
      radio.read( payload, len );
    }
    if (count > 10) return;
  }
}

