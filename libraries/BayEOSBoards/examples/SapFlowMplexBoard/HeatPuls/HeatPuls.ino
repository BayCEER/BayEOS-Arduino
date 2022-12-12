/****************************************************************

   Sketch for ESP01 Serial Router connected
   to BayEOS Low Power Board via FTDI connection

   ESP01 must run SerialRouterWiFiManagerWebserver
   from BayEOS-ESP8266 library

***************************************************************/
#define ACTION_COUNT 1
#define SAMPLING_INT 300

//Puls Duration in V²*µs
//With a resistance of 4 Ohm this will result in 4*8 Ws
#define HEATCOUNT (4.0*4.0*8000000)

#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <Sleep.h>

#define HEAT_PIN 6
#define ESP01_POWER_PIN 9
#define POWER_PIN 7
#define BAT_MULTIPLIER 3.3*(100+100)/100/1023
#define ADC_BATPIN A7
// we will collect 60 measurements before we try to send
#define WLAN_SEND_COUNT 60

uint8_t tx_error, res, wlan_status, low_bat = 0;
float bat, tmp_float;
unsigned long last_measurement;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
BaySerialESP client(Serial, ESP01_POWER_PIN);

#include <MCP342x.h>
const byte addr = 0;
const uint8_t gain = 3; //0-3: x1, x2, x4, x8
const uint8_t rate = 3; //0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);


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
  while (client.isReady()) {
    blinkLED(2);
    delay(1000);
  }
  client.powerDown();

  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  analogWrite(HEAT_PIN, 0); //shut off


  startLCB();
  last_measurement = myRTC.now().get();
}

void measure(void) {
  client.startDataFrame();
  for (uint8_t ch = 0; ch < 3; ch++) {
    digitalWrite(A1, ch & 0x4);
    digitalWrite(A2, ch & 0x2);
    digitalWrite(A3, ch & 0x1);
    delayLCB(10);
    mcp342x.runADC(1);
    delayLCB(mcp342x.getADCTime());
    client.addChannelValue(1e6*mcp342x.getData(), 11);
  }
  client.writeToBuffer();

}

void loop()
{
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);

    float heat = 0;
    pinMode(POWER_PIN, OUTPUT);
    digitalWrite(POWER_PIN, HIGH);
    analogReference(DEFAULT);
    measure();
    unsigned long heat_time;
    unsigned long start = micros();
    unsigned long heat_start = micros();
    digitalWrite(HEAT_PIN, HIGH);
    while (heat < HEATCOUNT) {
      bat = BAT_MULTIPLIER * analogRead(ADC_BATPIN);
      heat_time = (micros() - start);
      start = micros();
      heat += heat_time * bat * bat;
      delay(10);
    }
    digitalWrite(HEAT_PIN, LOW);
    client.startDataFrame();
    client.addChannelValue(micros() - heat_start);
    client.addChannelValue(bat);
    client.addChannelValue(heat);
    client.writeToBuffer();


    digitalWrite(POWER_PIN, LOW);

    start = myRTC.get();
    while ((myRTC.get() - start) < 240) {
      measure();
      delayLCB(1000);
    }
  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    if (myBuffer.available()) {
      adjust_OSCCAL();
      if (! wlan_status) {
        client.powerUp();
        wlan_status = 1;
      }
      if (res = client.sendMultiFromBuffer()) tx_error ++;
      else tx_error = 0;
      blinkLED(res + 1);
      if (tx_error > 5 || myBuffer.available() == 0) {
        wlan_status = 0;
        client.powerDown();
        tx_error = 0;
      }
    }
  }


  sleepLCB();

}
