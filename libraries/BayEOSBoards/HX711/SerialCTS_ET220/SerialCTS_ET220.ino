#define SAMPLING_INT 32
// Set a unique board name
#define ORIGIN "Tensio-1"
// Set a startup delay to have the boards on the same bus
// out of sync
#define STARTUP_DELAY 5000

#include <HX711Array.h>

uint8_t dout[] = {A3, A2, A1, A0};
uint8_t sck = A4;
long adc[4];

HX711Array scale;

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
  scale.begin(dout, 4, sck); //start HX711Array with 4 ADCs
  scale.set_gain(32);
  scale.power_down();
  
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement

    scale.power_up();
    scale.read_average(adc);
    scale.power_down();
    client.startDataFrameWithOrigin(BayEOS_Float32le, ORIGIN, 0, 1);
    client.addChannelValue(millis());
    //Ausgabe der Spannung in mV bei 10V Anregung!
    client.addChannelValue(10000.0/32/8388608*adc[0]);
    client.addChannelValue(10000.0/32/8388608*adc[1]);
    client.addChannelValue(10000.0/32/8388608*adc[2]);
    client.addChannelValue(10000.0/32/8388608*adc[3]);
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}



