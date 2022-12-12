/****************************************************************

  Logger Sketch 
  D2 -- RAIN -- GND

***************************************************************/

#define BLINK_ON_LOGGING_DISABLED 1
#define NRF24_TRYINT 60
const uint8_t channel = 0x62;
const uint8_t adr[] = {0x12, 0xae, 0x31, 0xc4, 0xfb};
//channel map and unit map must not exceed 98 characters!
char channel_map[] = "time;bat;rain;rain cum";
char unit_map[] = "ms;V;counts;counts";



#include <BayEOSBufferSPIFlash.h>
#include <BaySerialRF24.h>
#include <BayEOSLogger.h>
#include <Sleep.h>

RF24 radio(9, 10);
BaySerialRF24 client(radio, 100, 3); //wait maximum 100ms for ACK

SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#define TICKS_PER_SECOND 4
#include <LowCurrentBoard.h>

volatile uint8_t int0_flag;
void int0_isr(void){
  int0_flag=1;
}


//Add your sensor measurements here!
unsigned long last_measurement;

uint16_t current_tics, last_tics;

float cum_rain;
float last_rain;
void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    last_rain=cum_rain;
  }

  //Add your sensor measurements here!

  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  myLogger._bat = (1.1 * (220 + 100) / 100 / 1023 * analogRead(A7)) * 1000;
  digitalWrite(POWER_PIN, LOW);
  client.startDataFrame();
  client.addChannelValue(millis());
  client.addChannelValue(0.001*myLogger._bat);
  client.addChannelValue(cum_rain-last_rain);
  client.addChannelValue(cum_rain);
}



void setup() {
  client.init(channel, adr);
  radio.powerDown();
  pinMode(POWER_PIN, OUTPUT);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC, RTC_RELATIVE_SECONDS);
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 2700); //min_sampling_int = 60, LOW BAT Warning 3000mV
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  myLogger.setChannelMap(channel_map);
  myLogger.setUnitMap(unit_map);
  pinMode(2,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(2),int0_isr,FALLING);
  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  myLogger._bat = (1.1 * (220 + 100) / 100 / 1023 * analogRead(A7)) * 1000;
  digitalWrite(POWER_PIN, LOW);
  initLCB(); //init time2
}

unsigned long last_try = -NRF24_TRYINT;


void loop() {
  if(int0_flag){
    delay(20);
    int0_flag=0;
    cum_rain++;
  }
  if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                       (myRTC.get() - last_measurement) >= SAMPLING_INT)) {
    last_measurement = myRTC.get();
    measure();
  }
  myLogger.run(client.connected);

  if (! client.connected) {
    myLogger._mode = 0;
    //sleep until timer2 will wake us up...
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);

    //check if receiver is present
    if ((myRTC.get() - last_try) > NRF24_TRYINT) {
      last_try = myRTC.get();
      blinkLED(0);
      client.sendTestByte();
#if BLINK_ON_LOGGING_DISABLED
      if (! client.connected && myLogger._logging_disabled) {
        last_try -= (NRF24_TRYINT - 5);
        blinkLED(10);
      }
#endif
    }
  } else if ((millis() - client.last_activity) > 30000) {
    //check if still connected
    last_try = myRTC.get();
    client.sendTestByte();
  }


}
