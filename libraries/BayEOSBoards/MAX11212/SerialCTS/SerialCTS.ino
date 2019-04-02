#define SAMPLING_INT 32
// Set a unique board name
#define ORIGIN "TEROS10-nr4"
// Set a startup delay to have the boards on the same bus
// out of sync
#define STARTUP_DELAY 20000

#include <MAX11212Board.h>

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

  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 0); //do not use skip!
  initLCB(); //init time2
  delayLCB(STARTUP_DELAY);
  initMAX11212();
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    adc.read(1); //read with calibration
    adc.read(); //read once without calibration
    client.startDataFrameWithOrigin(BayEOS_Float32le, ORIGIN, 0, 1);
    client.addChannelValue(millis());
    for (uint8_t i = 0; i < 6; i++) {
      float mV=readChannel(i, 30)*1000;
      float wc=4.824e-10*mV*mV*mV-2.278e-6*mV*mV+3.898e-3*mV-2.154; // eqation 2 Teros10 page 14 [m³/m³]

      client.addChannelValue(wc*100); //Water content in %
      //  }
    }     
    sendOrBufferLCB();

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}



