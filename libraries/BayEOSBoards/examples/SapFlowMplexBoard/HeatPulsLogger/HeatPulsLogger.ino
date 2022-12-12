/****************************************************************

   Logger -Sketch

   Channels:
   1: Heat on/off  (1/0)
   2: Battery Voltage
   3: Heatcount
   4-6: thermocouple µV

***************************************************************/
#define ACTION_COUNT 1
#define SAMPLING_INT 300
const boolean measure_during_heatpuls=0;

//Puls Duration in V²*µs
//With a resistance of 4 Ohm this will result in 4*8 Ws
#define HEATCOUNT (4.0*4.0*8000000)

#include <RTClib.h>
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BaySerial.h>
#include <BayEOSLogger.h>
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
BaySerial client(Serial);
BayEOSLogger myLogger;
#define BAUD_RATE 38400
#define CONNECTED_PIN 9
uint8_t connected = 0;



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
  client.begin(BAUD_RATE);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset();
  client.setBuffer(myBuffer);
  myBuffer.setRTC(myRTC, 1); //Nutze RTC absolut!
  myLogger.init(client, myBuffer, myRTC, 30, 3500); //min sampling 30, battery warning 3500 mV

  mcp342x.reset();
  mcp342x.storeConf(rate, gain);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(HEAT_PIN, OUTPUT);
  analogWrite(HEAT_PIN, 0); //shut off


  pinMode(CONNECTED_PIN, INPUT_PULLUP);
  myLogger._logging_disabled = 1;

}

uint8_t mode = 0; //mode 0 == off, mode 1 == heat, mode 2 == measure
float heat;
unsigned long heat_time;
unsigned long start = micros();
unsigned long heat_start = micros();
unsigned long last_measurment;

void measure(void) {
  client.startDataFrame();
  client.addChannelValue(digitalRead(HEAT_PIN));
  client.addChannelValue(bat);
  client.addChannelValue(heat);
  if(mode!=1 || measure_during_heatpuls){
  for (uint8_t ch = 0; ch < 3; ch++) {
    digitalWrite(A1, ch & 0x4);
    digitalWrite(A2, ch & 0x2);
    digitalWrite(A3, ch & 0x1);
    if (connected || mode == 1) delay(10);
    else delayLCB(10);
    mcp342x.runADC(1);
    if (connected || mode == 1) delay(mcp342x.getADCTime());
    else delayLCB(mcp342x.getADCTime());
    client.addChannelValue(1e6 * mcp342x.getData());
    if (connected) {
      myLogger.handleCommand();
      myLogger.sendBinaryDump();
    }
  }
  } else {
    client.addChannelValue(NAN);
    client.addChannelValue(NAN);
    client.addChannelValue(NAN);
  }
  client.writeToBuffer();

}


void loop()
{
  if (myLogger._logging_disabled && myRTC.get() > 315360000L) {
    myLogger._logging_disabled = 0;
    UNSET_ACTION(0);

  }

  if (ISSET_ACTION(0) && ! myLogger._logging_disabled) {
    UNSET_ACTION(0);
    measure();
    mode = 1;
    heat = 0;
    last_measurement = myRTC.get();
    pinMode(POWER_PIN, OUTPUT);
    analogReference(DEFAULT);
    digitalWrite(POWER_PIN, HIGH);
    start = micros();
    heat_start = micros();
    digitalWrite(HEAT_PIN, HIGH);
  }

  if (mode ) {
    if ((myRTC.get() - last_measurement) > 2) {
      measure();
      last_measurement = myRTC.get();
    }
    if (mode == 1) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(HEAT_PIN, HIGH);
      if (heat < HEATCOUNT) {
        delay(10);
        bat = BAT_MULTIPLIER * analogRead(ADC_BATPIN);
        heat_time = (micros() - start);
        start = micros();
        myLogger._bat=1000*bat;
        heat += heat_time * bat * bat;
      } else {
        digitalWrite(HEAT_PIN, LOW);
        digitalWrite(POWER_PIN, LOW);
        digitalWrite(LED_BUILTIN, LOW);
        mode = 2;
        start = myRTC.get();


      }
    } else { //mode == 2
      if ((myRTC.get() - start) > 240) {
        mode = 0;
      }
    }
  }

  myLogger.liveData(500);
  myLogger.handleCommand();
  myLogger.sendBinaryDump();

  if (! connected && myLogger._logging_disabled) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(800);
  }

  if (! connected) {
    myLogger._mode = 0;
    if (! digitalRead(HEAT_PIN)) Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);
  }
  //check if still connected
  if (connected && digitalRead(CONNECTED_PIN)) {
    client.flush();
    client.end();
    connected=0;
  }
  //Connected pin is pulled to GND
  if (!connected && ! digitalRead(CONNECTED_PIN)) {
    connected = 1;
    adjust_OSCCAL();
    client.begin(38400);
  }


}
