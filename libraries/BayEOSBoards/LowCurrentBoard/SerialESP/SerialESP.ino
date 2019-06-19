/****************************************************************

   Sketch for ESP01 Serial Router connected
   to BayEOS Low Power Board via FTDI connection

   ESP01 must run SerialRouterWiFiManagerWebserver
   from BayEOS-ESP8266 library
  
***************************************************************/
#define ACTION_COUNT 1

#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <Sleep.h>

#define ESP01_POWER_PIN 9
#define POWER_PIN 7
#define BAT_MULTIPLIER 3.3*(100+100)/100/1023
#define ADC_BATPIN A7
// we will collect 60 measurements before we try to send
#define WLAN_SEND_COUNT 60

uint16_t wlan_counter = 0;
uint8_t tx_error, res, wlan_status, low_bat = 0;
float bat, tmp_float;
unsigned long last_measurement;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
BaySerialESP client(Serial, ESP01_POWER_PIN);

#include <LowCurrentBoard.h>


void setup(void) {
  initLCB(); //init time2
  adjust_OSCCAL();
  //CHANGE CONFIG!!
  client.begin(38400);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset();
  client.setBuffer(myBuffer);
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  myBuffer.skip(); //skip the unread frames in Buffer!!
  client.powerUp();
  while(client.isReady()){
    blinkLED(2);
    delay(1000);
  }
  client.powerDown();
  
  
  startLCB();
  wlan_counter = WLAN_SEND_COUNT - 2;
  last_measurement = myRTC.now().get();
}

void loop()
{
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    client.startDataFrame();
    client.addChannelValue(millis());
    pinMode(POWER_PIN,OUTPUT);
    digitalWrite(POWER_PIN,HIGH);
    analogReference(DEFAULT);
    bat = BAT_MULTIPLIER * analogRead(ADC_BATPIN);
    digitalWrite(POWER_PIN,LOW);

    client.addChannelValue(bat);
    //Some DEBUG values
    client.addChannelValue(myBuffer.readPos());
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myRTC.now().get());
    client.writeToBuffer();
    wlan_counter++;

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    if (wlan_counter > WLAN_SEND_COUNT) {
      adjust_OSCCAL();
      if (! wlan_status) {
        client.powerUp();
        wlan_status = 1;
      }
      if (res = client.sendMultiFromBuffer()) tx_error ++;
      else tx_error = 0;
      blinkLED(res + 1);
      if (tx_error > 5 || myBuffer.available() == 0) {
        wlan_counter = 0;
        wlan_status = 0;
        client.powerDown();
        tx_error = 0;
      }
    }
  }


  sleepLCB();

}
