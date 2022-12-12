/*
 * Example Sketch for BayEOS CR2032 Mini Board with a
 * BME-Sensor attached to I2C
 * 
 * The board should run with a CR2032 coin cell for several months
 * 
 * This Sketch uses Origin Frames. This overcomes the 6 pipe limitation
 * of RF24. However, some frames may get lost, when boards send at the same time
 */

#define SAMPLING_INT 32

//We use routed origin frames in this sketch
//so you can have more than 6 RF24 devices sending to one router
//Change boxname
//Do not use long names!! RF24 is limited to 32 byte!!!
//e.g: [DF][long][CF][RO][l][NAME][DF][T][cpu][bat][temp][hum][cs]
//      1    4    1   1   1   3    1   1   2     2    2    2   2 = 23
const char boxname[]="B01";
#define RF24CHANNEL 0x37
#define RF24ADDRESS 0x45c431ae12LL
#define LCB_BAT_ADCPIN A7

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

#include <BME280SoftI2C.h>
BME280SoftI2C bme(A4,A5); // I2C

void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer, 120); //use skip!
  initLCB(); //init time2
  readBatLCB();
  while(!bme.begin(0x76)){};
  startLCB();
}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    bme.triggerMeasurement();
    delayLCB(1000);
    client.startDataFrameWithOrigin(BayEOS_Int16le,boxname,1,1);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB*1000);
    if(! bme.error){
      client.addChannelValue(bme.readTemperature()*100);
      client.addChannelValue(bme.readHumidity()*100);
    }
    client.addChecksum();
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


