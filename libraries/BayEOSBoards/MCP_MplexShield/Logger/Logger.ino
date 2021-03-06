
#include <BayEOSBufferSPIFlash.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <Sleep.h>
#include <MCP_MPLEX.h>
#include <Wire.h>
#include <MCP342x.h>
#include <math.h>

const byte addr = 0;
const uint8_t gain = 0; //0-3: x1, x2, x4, x8
const uint8_t rate = 2; //0-3: 12bit ... 18bit
//  create an objcet of the class MCP342x
MCP342x mcp342x(addr);
//Configure your resistors on the board!
const uint16_t R[] = { 19877, 19975, 20028, 19966,
                       20015, 19926, 19898, 19912
                     };
float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}

#define CONNECTED_PIN 9
#define SAMPLING_INT 30
#define ACTION_COUNT 0
#define TICKS_PER_SECOND 16
uint8_t connected = 0;

BaySerial client(Serial);
SPIFlash flash(8); //CS-Pin of SPI-Flash
BayEOSBufferSPIFlash myBuffer;
BayEOSLogger myLogger;
#include <LowCurrentBoard.h>



//Add your sensor measurements here!
float values[9];
uint16_t count;
unsigned long last_measurement;

//Add your sensor measurements here!
uint16_t current_tics, last_tics;
void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < 9; i++) {
      values[i] = 0;
    }
  }






  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  if (digitalRead(CONNECTED_PIN))
    myLogger._bat = (1.1 * 320 / 100 / 1023 * analogRead(A0)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  digitalWrite(POWER_PIN, LOW);
  digitalWrite(MPLEX_POWER_PIN, HIGH);
  for (uint8_t ch = 0; ch < 8; ch++) {
    mplex_set_channel(ch);
    mcp342x.runADC(0);
    delay(mcp342x.getADCTime());
    float I = mcp342x.getData() / (float) R[ch];

    mcp342x.runADC(1);
    delay(mcp342x.getADCTime());
    float R_mess = mcp342x.getData() / I;
    values[ch + 1] += ntc10_R2T(R_mess);
  }
  digitalWrite(MPLEX_POWER_PIN, LOW);


  count++;

  client.startDataFrame(0x41);
  client.addChannelValue(millis(), 1);
  for (uint8_t i = 0; i < 9; i++) {
    client.addChannelValue(values[i] / count, i + 2);
  }



}



void setup() {
  pinMode(POWER_PIN, OUTPUT);
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myBuffer.init(flash);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 2500); //min_sampling_int = 60
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  Wire.begin();
  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  initLCB(); //init time2
}

void loop() {
  //Enable logging if RTC give a time later than 2010-01-01
  if (ISSET_ACTION(7) || connected) {
    UNSET_ACTION(7);
    if (myLogger._logging_disabled && myRTC.now().get() > 315360000L)
      myLogger._logging_disabled = 0;

    if (! myLogger._logging_disabled && (myLogger._mode == LOGGER_MODE_LIVE ||
                                         (myRTC._seconds - last_measurement) >= SAMPLING_INT)) {
      last_measurement = myRTC._seconds;
      measure();
    }
    myLogger.run();

    if (! connected && myLogger._logging_disabled) {
      pinMode(LED_BUILTIN, OUTPUT);
      digitalWrite(LED_BUILTIN, HIGH);
      delayLCB(200);
      digitalWrite(LED_BUILTIN, LOW);
      delayLCB(800);
      pinMode(LED_BUILTIN, INPUT);
    }
    //check if still connected
    if (connected && digitalRead(CONNECTED_PIN)) {
      connected++;
      if (connected > 5) {
        client.flush();
        client.end();
        connected = 0;
        myLogger._mode = 0;
      }
    }
    //Connected pin is pulled to GND
    if (!connected && ! digitalRead(CONNECTED_PIN)) {
      connected = 1;
      adjust_OSCCAL();
      client.begin(38400);
    }
  }
  //sleep until timer2 will wake us up...
  if (! connected) {
    Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
  }


}

