/****************************************************************

   Konfigurable Sketch for loggers

   with
   0-4 ADC-Channels
   0-2 Count Channels
   0-1 Battery Voltage

 ***************************************************************/
//Change 1 to 0 to disable channel
#define WITH_ADC0 1
#define WITH_ADC1 0
#define WITH_ADC2 0
#define WITH_ADC3 0
#define WITH_COUNT0 0
#define WITH_COUNT1 0
#define WITH_BAT 0

//ADC Configuration
const uint8_t gain0 = 3; //0-3: x1, x2, x4, x8
const uint8_t gain1 = 3; //0-3: x1, x2, x4, x8
const uint8_t gain2 = 3; //0-3: x1, x2, x4, x8
const uint8_t gain3 = 3; //0-3: x1, x2, x4, x8
/*
   Note - setting gainl will result in the following maximum Voltages
   2,048 - 1,024 - 0,512 - 0,256
*/


//Advanced Configuration
//Define maximal count rates
// e.g. 16 ticks per second and 12 lag ticks limits counts to 0.75 seconds
//for rain gauge this is ok. Anemometers need higher resolution
#define TICKS_PER_SECOND 16
#define COUNT_LAGTICKS 12
// The logger will store average values of several raw samples.
// Raw sampling interval can be defined here. More samples increase power consumption
#define SAMPLING_INT 30



#include <BayEOSBufferEEPROM.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
#include <MCP342x.h>

#define ACTION_COUNT 0
#define CONNECTED_PIN 9
#define POWER_PIN 7
uint8_t connected = 0;



BaySerial client(Serial);
uint8_t i2c_addresses[] = {0x50, 0x51, 0x52, 0x53};
BayEOSBufferMultiEEPROM myBuffer;
BayEOSLogger myLogger;
MCP342x mcp342x = MCP342x();
const byte addr = 0;
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 3; //0-3: 12bit ... 18bit

#include <LowCurrentBoard.h>


#if WITH_COUNT0
volatile uint8_t int0_event = 0;
volatile uint16_t int0_event_ticks;
uint16_t int0_count;
void int0_isr(void) {
  int0_event = 1;
  int0_event_ticks = ticks;
}

void handleINT0Event(void) {
  if (int0_event) {
    detachInterrupt(0);
  }
  if (int0_event && ((ticks - int0_event_ticks) > COUNT_LAGTICKS)) {
    attachInterrupt(0, int0_isr, FALLING);
    int0_count++;
    int0_event = 0;
  }
}

#endif

#if WITH_COUNT1
volatile uint8_t int1_event = 0;
volatile uint16_t int1_event_ticks;
uint16_t int1_count;
void int1_isr(void) {
  int1_event = 1;
  int1_event_ticks = ticks;
}

void handleINT1Event(void) {
  if (int1_event) {
    detachInterrupt(1);
  }
  if (int1_event && ((ticks - int1_event_ticks) > COUNT_LAGTICKS)) {
    attachInterrupt(1, int1_isr, FALLING);
    int1_count++;
    int1_event = 0;
  }
}

#endif



float values[5];
uint16_t count;
unsigned long last_measurement;

//Add your sensor measurements here!
void measure() {
  if (myLogger._logged_flag) {
    myLogger._logged_flag = 0;
    count = 0;
    for (uint8_t i = 0; i < 5; i++) {
      values[i] = 0;
    }
  }
  count++;
  client.startDataFrame();
#if WITH_ADC0
  mcp342x.setConf(addr, 1, 0, mode, rate, gain0);
  delayLCB(350);
  values[1] += mcp342x.getData(addr);
  client.addChannelValue(values[1] / count);
#endif
#if WITH_ADC1
  mcp342x.setConf(addr, 1, 1, mode, rate, gain1);
  delayLCB(350);
  values[2] += mcp342x.getData(addr);
  client.addChannelValue(values[2] / count);
#endif
#if WITH_ADC2
  mcp342x.setConf(addr, 1, 2, mode, rate, gain2);
  delayLCB(350);
  values[3] += mcp342x.getData(addr);
  client.addChannelValue(values[3] / count);
#endif
#if WITH_ADC3
  mcp342x.setConf(addr, 1, 3, mode, rate, gain3);
  delayLCB(350);
  values[4] += mcp342x.getData(addr);
  client.addChannelValue(values[4] / count);
#endif
#if WITH_COUNT0
  client.addChannelValue(int0_count);
#endif
#if WITH_COUNT1
  client.addChannelValue(int1_count);
#endif
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  analogReference(INTERNAL);
  if (digitalRead(CONNECTED_PIN)) //only read if not connected!!
    myLogger._bat = (1.1 * 320 / 100 / 1023 * analogRead(A0)) * 1000;
  values[0] += ((float)myLogger._bat) / 1000;
  analogReference(DEFAULT);
  digitalWrite(POWER_PIN, LOW);
  pinMode(POWER_PIN, INPUT);
#if WITH_BAT
  client.addChannelValue(values[0] / count);
#endif


}

void setup()
{
  initLCB(); //init time2
  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  pinMode(POWER_PIN, OUTPUT);

#if WITH_COUNT0
  digitalWrite(2, HIGH); //Enable Pullup on Pin 2 == INT0
  attachInterrupt(0, int0_isr, FALLING);
  int0_count = 0;
  int0_event = 0;
#endif
#if WITH_COUNT1
  digitalWrite(3, HIGH); //Enable Pullup on Pin 3 == INT1
  attachInterrupt(1, int1_isr, FALLING);
  int1_count = 0;
  int1_event = 0;
#endif
  myBuffer.init(4, i2c_addresses, 65536L);
  myBuffer.setRTC(myRTC); //Nutze RTC absolut!
  client.setBuffer(myBuffer);
  //register all in BayEOSLogger
  myLogger.init(client, myBuffer, myRTC, 60, 2500); //min_sampling_int = 60
  //disable logging as RTC has to be set first!!
  myLogger._logging_disabled = 1;
  Wire.begin();
  startLCB();
}

void loop()
{
#if WITH_INT0
  handleINT0Event();
#endif
#if WITH_INT1
  handleINT1Event();
#endif

  if (ISSET_ACTION(7) || connected) {
    UNSET_ACTION(7);
    //Enable logging if RTC give a time later than 2010-01-01
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



