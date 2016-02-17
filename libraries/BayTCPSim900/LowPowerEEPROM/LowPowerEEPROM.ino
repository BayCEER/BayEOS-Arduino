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

BayGPRS client=BayGPRS();
BayEOSBufferEEPROM myBuffer;

#define SAMPLING_INT 32
#define LCB_BAT_MULTIPLIER 1.1*540/100/1023
// we will collect 120 measurements before we try to send
uint8_t gprs_counter=0;
uint8_t gprs_status=0;
#define GPRS_SEND_COUNT 120
uint8_t tx_error;

#define ACTION_COUNT 1
#include <LowCurrentBoard.h>

void setup()
{
  pinMode(GPRS_POWER_PIN,OUTPUT);
  digitalWrite(GPRS_POWER_PIN,LOW);
  //CHANGE CONFIG!!
  client.readConfigFromStringPGM(PSTR("192.168.0.1|80|gateway/frame/saveFlat|admin|xbee|TestGPRS|pinternet.interkom.de|||PIN"));
  client.softSwitch();
  client.begin(38400);
  client.sendMessage("GPRS started");
  digitalWrite(GPRS_POWER_PIN,HIGH);
 
  Wire.begin();
  myBuffer.init(0x50,65536L,0); //NO flush!!
  myBuffer.setRTC(myRTC,0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); 
  initLCB(); //init time2   
  readBatLCB(); 
  startLCB();
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
      tx_error=client.sendMultiFromBuffer();
      
      if(tx_error || ! myBuffer.available()){
        gprs_counter=0;
        digitalWrite(GPRS_POWER_PIN,HIGH);
        gprs_status=0;
      }
    }
  }
  sleepLCB();
}
    

