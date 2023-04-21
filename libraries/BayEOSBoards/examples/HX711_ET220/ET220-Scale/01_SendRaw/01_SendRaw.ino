/*
 * This sketch sends the raw readings from 4 HX711 and Dallas Temperature Sensors
 * 
 * The sketch can be used to collect calibration data
 */
#define SAMPLING_INT 32
#define RF24CHANNEL 0x37
//#define RF24CHANNEL 0x4e
#define RF24ADDRESS 0x45c431ae48
//#define RF24ADDRESS 0x45c4a1ae96
#define WITH_CHECKSUM 1

#include <HX711Array.h>

uint8_t dout[] = {A3, A2, A1, A0};
uint8_t sck = A4;

HX711Array scale;

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <DS18B20.h>
DS18B20 ds1(2, 0, 1);
DS18B20 ds2(3, 0, 1);
DS18B20 ds3(4, 0, 1);
DS18B20 ds4(6, 0, 1);
float temp;

#include <LowCurrentBoard.h>

void readBat(void){
  pinMode(7,OUTPUT);
  digitalWrite(7,HIGH);
  analogReference(DEFAULT);
  batLCB=3.3*200/100/1023*analogRead(A7);
  digitalWrite(7,LOW);
}

void setup()
{
  ds1.setAllAddr(); //Search for new or removed sensors
  ds2.setAllAddr(); 
  ds3.setAllAddr(); 
  ds4.setAllAddr(); 
  ds1.setResolution(12);
  ds2.setResolution(12);
  ds3.setResolution(12);
  ds4.setResolution(12);

  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 20); // use skip!
  initLCB(); //init time2
  scale.begin(dout, 4, sck); //start HX711Array with 4 ADCs
  scale.set_gain(128);
  scale.power_down();
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    ds1.t_conversion(); //Start T-conversion
    ds2.t_conversion(); //Start T-conversion
    ds3.t_conversion(); //Start T-conversion
    ds4.t_conversion(); //Start T-conversion
    delayLCB(700);

    long adcs[4];
    long adc;
    uint8_t counts[4];
    scale.power_up();
    uint8_t tries=0;
    while(tries<10){
      adc=scale.read_average_with_filter(adcs,3000,counts);
      if(adc!=0xf0000000) break;
      tries++;
    }
    readBat();
    scale.power_down();
    float t_mean=0;
    ds1.readChannel(1, &temp);
    t_mean+=temp;
    ds2.readChannel(1, &temp);
    t_mean+=temp;
    ds3.readChannel(1, &temp);
    t_mean+=temp;
    ds4.readChannel(1, &temp);
    t_mean+=temp;
    t_mean/=4;
    
    client.startDataFrame(BayEOS_Float32le,WITH_CHECKSUM);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB);
    client.addChannelValue(t_mean);
    client.addChannelValue(adc);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    sendOrBufferLCB();
    //Read battery voltage _after_ long uptime!!!

    client.startDataFrame(BayEOS_Float32le,WITH_CHECKSUM);
    client.addChannelValue(adcs[0],11);
    client.addChannelValue(adcs[1]);
    client.addChannelValue(adcs[2]);
    client.addChannelValue(adcs[3]);
#if WITH_CHECKSUM
    client.addChecksum();
#endif
    client.sendOrBuffer();
 

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}
