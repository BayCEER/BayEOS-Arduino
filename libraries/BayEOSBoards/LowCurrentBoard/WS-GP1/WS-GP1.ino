/****************************************************************
 * 
 * Sketch for WS-GP1 Weatherstaton
 * 
 * with
 * air temperature - air humidty (SHT21)
 * ait temperature ntc via mcp CH3+CH4 + A1
 * air humidity via mcp CH2 + Upstepper D4 (1 Second warmup)
 * solar via mcp CH1 + D6
 * rain gauge (INT0)
 * wind direction A2 (signal) + A3
 * wind speed (INT1)
 *
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
#include <BayDebug.h>
#include <MCP342x.h>
#include <Sleep.h>
#include <SHT2xSleep.h>
MCP342x mcp342x = MCP342x();
const byte addr = 0;
const uint8_t mode = 0; //0 == one-shot mode - 1 == continuos mode
const uint8_t rate = 2; //0-3: 12bit ... 18bit
const uint8_t gain = 0; //max Voltage: 0,512 Volt

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
BayDebug client(Serial);

#include <LowCurrentBoard.h>

#if WITHRAINGAUGE
  float last_rain_count;
#endif
float tmp_float;

#include <math.h>
float ntc10_R2T(float r){
  float log_r=log(r);
  return 440.61073-75.69303*log_r+
         4.20199*log_r*log_r-0.09586*log_r*log_r*log_r;
}


void setup(void) {
  pinMode(A3,OUTPUT); //Winddirection
  pinMode(A1,OUTPUT); //NTC
  pinMode(6,OUTPUT); //Solar
  pinMode(4,OUTPUT); //Humidity
  client.begin(9600, 3);
  Serial.print("Starting...");
  myBuffer.init(flash,10); //Do not restore pointers
  myBuffer.reset();
  client.setBuffer(myBuffer);
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ! 
  digitalWrite(3,HIGH); //Enable Pullup on Pin 3 == INT1
  initLCB(); //init time2
  Serial.println("End setup");
}

void loop()
{

#if WITHRAINGAUGE
  handleRainEventLCB();
#endif

  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    digitalWrite(4,HIGH); //Humidity with Upstepper!
    delayLCB(1000); //We have to 1 second
    client.startDataFrame();
    client.addChannelValue(millis());
    digitalWrite(A1,HIGH); //NTC
    digitalWrite(6,HIGH); //Solar
   
    client.addChannelValue(SHT2x.GetTemperature());
    client.addChannelValue(SHT2x.GetHumidity());
    SHT2x.reset();

    mcp342x.setConf(addr, 1, 0, mode, rate, gain); //Solar
    delayLCB(100);
    client.addChannelValue(mcp342x.getData(addr)*1000); //solar
    digitalWrite(6,LOW); //Solar

    mcp342x.setConf(addr, 1, 2, mode, rate, gain); //Widerstand
    delayLCB(100);
    float strom=mcp342x.getData(addr)/19868.0;
    mcp342x.setConf(addr, 1, 3, mode, rate, gain); //NTC
    delayLCB(100);
    client.addChannelValue(ntc10_R2T(mcp342x.getData(addr)/strom));
    digitalWrite(A1,LOW); //NTC
    
    mcp342x.setConf(addr, 1, 1, mode, rate, gain); //Humidity
    delayLCB(100);
    client.addChannelValue(mcp342x.getData(addr)*100); //Humidity 
    readBatLCB();
    digitalWrite(4,LOW); //Humidity
    
    digitalWrite(A3,HIGH); //Winddirection
    delayLCB(20);
    client.addChannelValue(analogRead(A2)); //Winddirection
    digitalWrite(A3,LOW); //Winddirection
    
#if WITHWIND
    noInterrupts();
    tmp_float=wind_count;
    wind_count = 0;
    interrupts();
    client.addChannelValue(tmp_float / (float)SAMPLING_INT);
#endif

#if WITHRAINGAUGE
    noInterrupts();
    tmp_float=rain_count;
    interrupts();
    client.addChannelValue(tmp_float);
    client.addChannelValue((tmp_float-last_rain_count)/(float)SAMPLING_INT*60);
    last_rain_count=tmp_float;
#endif
     sendOrBufferLCB();
     delay(50);
    
  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
    delay(50);
  }

 // sleepLCB();

  
}

