#define SAMPLING_INT 32
#define RF24CHANNEL 0x2b
#define RF24ADDRESS 0x40c467ae12
#define WITH_CHECKSUM 1
#define LCB_BAT_MULTIPLIER 3.3*200/100/1023

#include <MAX11212Board.h>

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);


#include <LowCurrentBoard.h>

void readBat(){
  digitalWrite(7,HIGH);
  analogReference(DEFAULT);
  batLCB = LCB_BAT_MULTIPLIER * analogRead(A7);
  digitalWrite(7,LOW);
}


void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);

  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 0); //do not use skip!
  initLCB(); //init time2
  initMAX11212();
  readBat(); 
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    adc.read(1); //read with calibration
    adc.read(); //read once without calibration
    client.startDataFrame(BayEOS_Int16le, WITH_CHECKSUM);
    client.addChannelValue(millis());
  client.addChannelValue(batLCB*1000);
    for (uint8_t i = 0; i < 6; i++) {
      float mV=readChannel(i, 30)*1000;
      //float wc=4.824e-10*mV*mV*mV-2.278e-6*mV*mV+3.898e-3*mV-2.154; // eqation 2 Teros10 page 14 [m³/m³]

      client.addChannelValue(mV*10); 
      //  }
    }     
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}
