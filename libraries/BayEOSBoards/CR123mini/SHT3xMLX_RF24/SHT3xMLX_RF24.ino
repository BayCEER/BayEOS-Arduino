/*
 * Example Sketch for BayEOS CR123 Mini Board with a
 * SHT31 + MLX90614 sensor attached to A4/A5 read out via SoftI2C
 * 
 * The board should run with a CR123 for several months
 * 
 * This Sketch uses Origin Frames. This overcomes the 6 pipe limitation
 * of RF24. However, some frames may get lost, when boards send at the same time
 */

#define SAMPLING_INT 32

//We use routed origin frames in this sketch
//so you can have more than 6 RF24 devices sending to one router
//Change boxname
//Do not use long names!! RF24 is limited to 32 byte!!!
//e.g: [DF][long][CF][RO][l][NAME][DF][T][cpu][bat][temp][hum][mlxa][mlxo][cs]
//      1    4    1   1   1   3    1   1   2     2    2    2     2     2   2 = 27
const char boxname[]=" MXS";
#define RF24CHANNEL 0x5e
#define RF24ADDRESS 0x45e431ae48
#define LCB_BAT_ADCPIN A7

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

#include <SHT3x.h>
SHT3x sht(A4, A5);

#include <MLX90614SoftI2C.h>
MLX90614SoftI2C mlx(A4,A5);


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
  startLCB();
}

float temperature;
float humidity;
int ret;

void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);    
    
    mlx.exitSleepMode(100);
    ret=sht.measureSleep(&temperature, &humidity);
    client.startDataFrameWithOrigin(BayEOS_Int16le,boxname,1,1);
    client.addChannelValue(millis());
    client.addChannelValue(batLCB*1000);
    client.addChannelValue(temperature*100);
    client.addChannelValue(humidity*100);
    client.addChannelValue(mlx.readAmbientTempC()*100);
    client.addChannelValue(mlx.readObjectTempC()*100);
    mlx.enterSleepMode();
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
