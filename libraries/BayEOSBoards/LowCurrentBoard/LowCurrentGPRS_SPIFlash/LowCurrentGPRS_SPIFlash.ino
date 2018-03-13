/****************************************************************

   Sketch for SIM800L-Test with new SMD-Version of
   BayEOS Low Power Board

 ***************************************************************/
#define TICKS_PER_SECOND 128
#define WITHDALLAS 0
#define WITHWIND 1
#define WITHRAINGAUGE 1
#define SAMPLING_INT 30
#define ACTION_COUNT 1

#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BayTCPSim900.h>
#include <MCP342x.h>
#include <Sleep.h>
MCP342x mcp342x = MCP342x();
const byte addr = 0;
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t gain = 0; //max Voltage: 0,512 Volt

#define GPRS_POWER_PIN 7
#define BAT_MULTIPLIER 3.3*320/100/1023
#define ADC_BATPIN A7
// we will collect 60 measurements before we try to send
#define GPRS_SEND_COUNT 60

uint16_t gprs_counter = 0;
uint8_t tx_error, res, gprs_status;
float bat, tmp_float;
unsigned long last_measurement;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
BayGPRS client(Serial);

#include <LowCurrentBoard.h>

#if WITHRAINGAUGE
float last_rain_count;
#endif


void setup(void) {
  initLCB(); //init time2
  pinMode(GPRS_POWER_PIN, OUTPUT);
  digitalWrite(GPRS_POWER_PIN, HIGH);
  delay(1000);
  adjust_OSCCAL();
  //CHANGE CONFIG!!
  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|GPRS-T1|pinternet.interkom.de|||5659|"));
  client.changeIPR(38400);
  res = client.begin(38400);
  blinkLED(res + 1);
  /*
     1 == OK
     2 == NO Communication
     3 == PIN failed
     4 == PIN locked
     5 == Not CREG
     6 == Not CGATT
     7 == No SIM Card
  */
  delay(2000);

  res = client.sendMessage("GPRS started");
  blinkLED(res + 1);
  if (! res) myRTC.adjust(client.now());


  /*
     1 == OK
     2 == no success
     3 == timeout
     4 == network timeout
     5 == gprs modem timeout
  */
  analogReference(DEFAULT);
  bat = BAT_MULTIPLIER * analogRead(ADC_BATPIN);
  digitalWrite(GPRS_POWER_PIN, LOW);
  delay(2000);


  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset();
  client.setBuffer(myBuffer);
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  if (myRTC.now().get() < 2000) myBuffer.skip(); //got no time! skip the unread frames in Buffer!!
  digitalWrite(2, HIGH); //Enable Pullup on Pin 2 == INT0
  digitalWrite(3, HIGH); //Enable Pullup on Pin 3 == INT1
  startLCB();
  gprs_counter = GPRS_SEND_COUNT - 2;
  last_measurement = myRTC.now().get();
}

void loop()
{
#if WITHRAINGAUGE
  handleRainEventLCB();
#endif

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    client.startDataFrame();
    client.addChannelValue(millis());
    mcp342x.setConf(addr, 1, 0, mode, rate, gain);
    delayLCB(100);
    client.addChannelValue(mcp342x.getData(addr) * 1000);
#if WITHWIND
    noInterrupts();
    tmp_float = wind_count;
    wind_count = 0;
    interrupts();
    if (tmp_float) {
      //Calibration equation
      tmp_float = tmp_float * 60 / (float)(myRTC.now().get() - last_measurement) / 37.547 + 0.28;
    }
    last_measurement = myRTC.now().get();
    client.addChannelValue(tmp_float);
#endif

#if WITHRAINGAUGE
    noInterrupts();
    tmp_float = rain_count;
    interrupts();
    //RG2+WS-CA: 0.2mm per count
    client.addChannelValue(tmp_float * 0.2);
    client.addChannelValue((tmp_float - last_rain_count) / (float)SAMPLING_INT * 60 * 0.2);
    last_rain_count = tmp_float;
#endif
    client.addChannelValue(bat);
    //Some DEBUG values
    client.addChannelValue(myBuffer.readPos());
    client.addChannelValue(myBuffer.writePos());
    client.addChannelValue(myRTC.now().get());
    client.writeToBuffer();
    gprs_counter++;

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    if (gprs_counter > GPRS_SEND_COUNT) {
      switch (gprs_status) {
        case 0:
          digitalWrite(GPRS_POWER_PIN, HIGH);
          adjust_OSCCAL();
          client.begin(38400, 1);
          gprs_status = 1;
          tx_error = 0;
          break;
        case 1:
          if (client.isRegistered()) {
            gprs_status = 2;
            tx_error = 0;
          } else tx_error++;
          break;
        case 2:
          if (client.isAttached()) {
            gprs_status = 3;
            tx_error = 0;
          } else tx_error++;
        case 3:

          adjust_OSCCAL();
          if (res = client.sendMultiFromBuffer(2000)) tx_error += 20;
          else tx_error = 0;
          blinkLED(res + 1);

          break;
      }
      analogReference(DEFAULT);
      bat = BAT_MULTIPLIER * analogRead(ADC_BATPIN);
      //We got to many errors or buffer is empty or low bat
      if (tx_error > 100 || ! myBuffer.available() || bat < 3.7) {
        if (bat < 3.7) {
          client.startFrame(BayEOS_ErrorMessage);
          client.addToPayload("Low bat");
          client.writeToBuffer();
        }
        if (tx_error) {
          client.startFrame(BayEOS_ErrorMessage);
          client.addToPayload("TX-Error:");
          client.addToPayload('0' + gprs_status);
          client.addToPayload('/');
          client.addToPayload('0' + res);
          client.writeToBuffer();
        }
        gprs_counter = 0;
        gprs_status = 0;
        tx_error = 0;
        digitalWrite(GPRS_POWER_PIN, LOW);
        Serial.end();
        pinMode(1, INPUT);
        if (bat > 4.1 && tx_error) gprs_counter = GPRS_SEND_COUNT - 1;
      }
    }
  }


  sleepLCB();

}

