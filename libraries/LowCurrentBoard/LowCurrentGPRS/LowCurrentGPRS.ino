/****************************************************
* This is a example sketch for using GPRS with 
* LOW POWER CONSUMPTION 
* GPRS-MODEM will get switched off totally by MOSFET
* Sending occurs only once an hour
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

#define GPRS_POWER_PIN 8

BayGPRS client=BayGPRS(Serial);
BayEOSBufferEEPROM myBuffer;

#define SAMPLING_INT 64
#define LCB_BAT_MULTIPLIER 1.1*540/100/1023
// we will collect 120 measurements before we try to send
uint8_t gprs_counter=0;
uint8_t gprs_status=0;
#define GPRS_SEND_COUNT 240
uint8_t tx_error=0;

#define ACTION_COUNT 1
#define LED_PIN 5
#include <LowCurrentBoard.h>

uint8_t res;
void setup()
{
  pinMode(GPRS_POWER_PIN,OUTPUT);
  digitalWrite(GPRS_POWER_PIN,LOW);
  initLCB();
  //CHANGE CONFIG!!
  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|LP-GPRS2|internet|||1802|"));
  blinkLED(2); 
  client.softSwitch();
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
  
  digitalWrite(GPRS_POWER_PIN,HIGH);
 
  Wire.begin();
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); 
  initLCB(); //init time2   
  readBatLCB(); 
  startLCB();
  gprs_counter=GPRS_SEND_COUNT-2;
}


void loop()
{
  if(ISSET_ACTION(0)){
    UNSET_ACTION(0);
    gprs_counter++;
    client.startDataFrame(BayEOS_ChannelFloat32le);
    client.addChannelValue(millis(),1);
    client.addChannelValue(batLCB,2);
    client.writeToBuffer();
    readBatLCB(); 
  }
  
  if(ISSET_ACTION(7)){
    UNSET_ACTION(7);
    if(gprs_counter>GPRS_SEND_COUNT){
      if(! gprs_status){
        digitalWrite(GPRS_POWER_PIN,LOW);
        client.softSwitch();
        client.begin(38400);
        gprs_status=1;
      }
      if(client.sendMultiFromBuffer()) tx_error++;
      else tx_error=0;
      
      if(tx_error>2 || ! myBuffer.available()){
        gprs_counter=0;
        gprs_status=0;
        tx_error=0;
        digitalWrite(GPRS_POWER_PIN,HIGH);
      }
    }
  }
  
  sleepLCB();
}
    

