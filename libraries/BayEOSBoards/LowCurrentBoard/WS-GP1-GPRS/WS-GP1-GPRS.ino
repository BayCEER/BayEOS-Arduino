/****************************************************************

   Sketch for WS-GP1 Weatherstaton

   with
   air temperature - air humidty (SHT21)
   ait temperature ntc via mcp CH3+CH4 + A1
   air humidity via mcp CH2 + Upstepper D4 (1 Second warmup)
   solar via mcp CH1 + D6
   rain gauge (INT0)
   wind direction A2 (signal) + A3
   wind speed (INT1)

 ***************************************************************/
#define TICKS_PER_SECOND 128
#define WITHDALLAS 0
#define WITHWIND 1
#define WITHRAINGAUGE 1
#define SAMPLING_INT 30
#define ACTION_COUNT 1

#include <BayEOSBufferSPIFlash.h>
#include <BayTCPSim900.h>
#include <MCP342x.h>
#include <SHT2xSleep.h>
MCP342x mcp342x = MCP342x();
const byte addr = 0;
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t gain = 0; //max Voltage: 0,512 Volt

#define GPRS_POWER_PIN 7
#define BAT_MULTIPLIER 1.1*570/100/1023
#define GPRS_SEND_COUNT 240
// we will collect 240 measurements before we try to send
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

#include <math.h>
float ntc10_R2T(float r) {
  float log_r = log(r);
  return 440.61073 - 75.69303 * log_r +
         4.20199 * log_r * log_r - 0.09586 * log_r * log_r * log_r;
}

void setup(void) {
  initLCB(); //init time2
  pinMode(A3, OUTPUT); //Winddirection
  pinMode(A1, OUTPUT); //NTC
  pinMode(6, OUTPUT); //Solar
  pinMode(4, OUTPUT); //Humidity
  pinMode(GPRS_POWER_PIN, OUTPUT);
  digitalWrite(GPRS_POWER_PIN, HIGH);
  delay(1000);
  adjust_OSCCAL();
  //CHANGE CONFIG!!
  client.readConfigFromStringPGM(PSTR("132.180.112.55|80|gateway/frame/saveFlat|import|import|WS-GP1-XX|pinternet.interkom.de|||PIN|"));
  client.changeIPR(38400);
  res = client.begin(38400);
  if (! res) myRTC.adjust(client.now());
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


  /*
     1 == OK
     2 == no success
     3 == timeout
     4 == network timeout
     5 == gprs modem timeout
  */
  analogReference(INTERNAL);
  bat = BAT_MULTIPLIER * analogRead(A0);
  analogReference(DEFAULT);
  digitalWrite(GPRS_POWER_PIN, LOW);
  delay(2000);


  myBuffer.init(flash, 10); //This will restore old pointers
  client.setBuffer(myBuffer);
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
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
    digitalWrite(6, HIGH); //Solar
    delayLCB(500); //We have to wait??
    digitalWrite(A1, HIGH); //NTC

    client.addChannelValue(SHT2x.GetTemperature());
    client.addChannelValue(SHT2x.GetHumidity());
    SHT2x.reset();

    mcp342x.setConf(addr, 1, 0, mode, rate, gain); //Solar
    delayLCB(100);
    //Solar : 0.5 W/m² per mV
    client.addChannelValue(mcp342x.getData(addr) * 1000 * 0.5); //solar
    digitalWrite(6, LOW); //Solar

    mcp342x.setConf(addr, 1, 2, mode, rate, gain); //Widerstand
    delayLCB(100);
    float strom = mcp342x.getData(addr) / 19868.0;
    mcp342x.setConf(addr, 1, 3, mode, rate, gain); //NTC
    delayLCB(100);
    client.addChannelValue(ntc10_R2T(mcp342x.getData(addr) / strom));
    digitalWrite(A1, LOW); //NTC

    digitalWrite(4, HIGH); //Humidity with Upstepper!
    delayLCB(1500);
    mcp342x.setConf(addr, 1, 1, mode, rate, gain); //Humidity
    delayLCB(100);
    client.addChannelValue(mcp342x.getData(addr) * 100); //Humidity
    digitalWrite(4, LOW); //Humidity

    digitalWrite(A3, HIGH); //Winddirection
    delayLCB(20);
    tmp_float = analogRead(A2);
    tmp_float = tmp_float * 712 / 1023;
    if (tmp_float > 360) tmp_float = 360;
    client.addChannelValue(tmp_float); //Winddirection
    digitalWrite(A3, LOW); //Winddirection

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

          client.startDataFrameWithOrigin(BayEOS_Float32le, "GPRS", 0, 1);
          client.addChannelValue(bat, 30);
          client.addChannelValue(res);
          client.addChannelValue(client.getRSSI());
          client.addChannelValue(OSCCAL);
          client.writeToBuffer();
          adjust_OSCCAL();
          if (res = client.sendMultiFromBuffer(2000)) tx_error += 20;
          else tx_error = 0;
          blinkLED(res + 1);

          break;
      }
      analogReference(INTERNAL);
      bat = BAT_MULTIPLIER * analogRead(A0);
      analogReference(DEFAULT);
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
        if(bat>4.1 && tx_error) gprs_counter=GPRS_SEND_COUNT-1;
      }
    }
  }


  sleepLCB();

}

