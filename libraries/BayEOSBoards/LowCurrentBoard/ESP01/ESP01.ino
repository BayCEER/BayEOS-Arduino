/****************************************************************

   Sketch for ESP01-Test with new SMD-Version of
   BayEOS Low Power Board

 ***************************************************************/
#define TICKS_PER_SECOND 128
#define ACTION_COUNT 1

#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BayTCPESP8266.h>
#include <Sleep.h>

#define ESP01_POWER_PIN 7
#define BAT_MULTIPLIER 3.3*320/100/1023
#define ADC_BATPIN A7
// we will collect 60 measurements before we try to send
#define WLAN_SEND_COUNT 60

uint16_t wlan_counter = 0;
uint8_t tx_error, res, wlan_status, low_bat = 0;
float bat, tmp_float;
unsigned long last_measurement;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
BayESP8266 client = BayESP8266(Serial, ESP01_POWER_PIN);

#include <LowCurrentBoard.h>


void setup(void) {
  initLCB(); //init time2
  adjust_OSCCAL();
  //CHANGE CONFIG!!
  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|WLAN-T1|SSID|PW|||"));
  client.powerUp();
  res = client.begin(115200);
  blinkLED(res + 1);
  /*
     1 == OK
     2 == NO Communication
  */
  delay(2000);

  res = client.sendMessage("WLAN started");
  blinkLED(res + 1);

  /*
     1 == OK
     2 == no success
     3 == timeout
     4 == network timeout
     5 == modem timeout
  */
  analogReference(DEFAULT);
  bat = BAT_MULTIPLIER * analogRead(ADC_BATPIN);

  client.powerDown();



  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset();
  client.setBuffer(myBuffer);
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  myBuffer.skip(); //skip the unread frames in Buffer!!
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
      if (res = client.sendMultiFromBuffer(2000)) tx_error ++;
      else tx_error = 0;
      blinkLED(res + 1);
      analogReference(DEFAULT);
      bat = BAT_MULTIPLIER * analogRead(ADC_BATPIN);
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
