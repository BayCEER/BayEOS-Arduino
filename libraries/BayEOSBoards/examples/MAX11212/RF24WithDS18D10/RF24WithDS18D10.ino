#define SAMPLING_INT 64
#define RF24CHANNEL 0x5e
#define RF24ADDRESS 0x45e431ae12
#define WITH_CHECKSUM 1
#define ORIGIN "0102"
#define LCB_BAT_MULTIPLIER 3.3*200/100/1023



#include <MAX11212Board.h>
#include <DS18B20.h>

DS18B20 ds1(3,0,3);
DS18B20 ds2(6,3,3);

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

const byte* new_addr;
uint8_t channel;
float temp;

void readBat(){
  digitalWrite(7,HIGH);
  analogReference(DEFAULT);
  batLCB = LCB_BAT_MULTIPLIER * analogRead(A7);
  digitalWrite(7,LOW);
}

void setup(void) {
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 20); //use skip!
  initLCB(); //init time2
  Sleep.setupTimer2(2); //init timer2 to 1/16 sec
  initMAX11212();
  ds1.setAllAddrFromEEPROM();
  ds2.setAllAddrFromEEPROM();
   pinMode(7,OUTPUT);
  readBat(); 
  startLCB();
}

void loop(void){
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
  
  adc.read(1); //read with calibration
  adc.read(); //read once without calibration
  client.startDataFrameWithOrigin(BayEOS_Float32le, ORIGIN, WITH_CHECKSUM,1);
  client.addChannelValue(millis());
  client.addChannelValue(batLCB);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
  sendOrBufferLCB();
  
  client.startDataFrameWithOrigin(BayEOS_Int16le, ORIGIN, WITH_CHECKSUM,1);
  for (uint8_t i = 0; i < 6; i++) {
     float mV=readChannel(i, 30)*1000;
     float wc=4.824e-10*mV*mV*mV-2.278e-6*mV*mV+3.898e-3*mV-2.154; // eqation 2 Teros10 page 14 [m³/m³]
     client.addChannelValue(wc*100*100,i+3); //Water content in %
  }
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    client.sendOrBuffer();

  ds1.t_conversion();
  ds2.t_conversion();
  delayLCB(1000);
  client.startDataFrameWithOrigin(BayEOS_Int16le, ORIGIN, WITH_CHECKSUM,1);  
   while(channel=ds1.getNextChannel()){
       ds1.readChannel(channel,&temp);
     client.addChannelValue(temp*100,channel+10); //Water content in %
     
   }
   while(channel=ds2.getNextChannel()){
       ds2.readChannel(channel,&temp);
     client.addChannelValue(temp*100,channel); //Water content in %
     
   }
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    client.sendOrBuffer();
    readBat();
  }
 if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();
  
  
}

