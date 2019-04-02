#define SAMPLING_INT 32
// Set a unique board name
#define ORIGIN "Test-Board2"
// Set a startup delay to have the boards on the same bus
// out of sync
#define STARTUP_DELAY 10000

#include <HX711Array.h>
#include <NTC.h>
#define BAT_MULTIPLIER 3.3*200.0/100.0/1023

uint8_t dout[] = {6, 4};
uint8_t sck = 3;
long adc[2];
float temp0, temp1;

HX711Array scale;
NTC_HX711 ntc(scale, A3, 2*470000, 3.0); //adjust resistor values
Scale4PointCal cal0;
Scale4PointCal cal1(28); //One Config needs 28 Byte in EEPROM - so second has a offest of 28

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BaySerial.h>
// Client with timeout 300ms, 38400 baud and CTS-pin 9
BaySerial client(Serial, 300, 38400, 9);

#include <LowCurrentBoard.h>

void setup()
{
  //Do not call client.begin(...) here
  //client.begin and client.end is automatically called by sendPayload()

  pinMode(POWER_PIN,OUTPUT);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 0); //do not use skip!
  initLCB(); //init time2
  delayLCB(STARTUP_DELAY);
  scale.begin(dout, 2, sck); //start HX711Array with 1 ADCs
  cal0.readConf();
  cal1.readConf();
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    ntc.readResistance();
    temp0 = ntc.getTemp(0);
    temp1 = ntc.getTemp(1);

    scale.power_up();
    scale.read_average(adc);
    digitalWrite(POWER_PIN,HIGH);
    client.startDataFrameWithOrigin(BayEOS_ChannelFloat32le, ORIGIN, 0, 1);
    client.addChannelValue(millis());
    client.addChannelValue(BAT_MULTIPLIER*analogRead(A7));
    digitalWrite(POWER_PIN,LOW);
    scale.power_down();
    sendOrBufferLCB();
    client.addChannelValue(temp0);
    client.addChannelValue(adc[0]);
    client.addChannelValue(cal0.getWeight(adc[0], temp0));
    client.addChannelValue(temp1);
    client.addChannelValue(adc[1]);
    client.addChannelValue(cal1.getWeight(adc[1], temp1));

    
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!
    readBatLCB();

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}



