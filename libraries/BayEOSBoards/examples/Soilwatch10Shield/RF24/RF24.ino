#define SAMPLING_INT 64

#define RF24CHANNEL 0x4c
#define RF24ADDRESS 0x172af39a12LL

//Do not use long names!! RF24 is limited to 32 byte!!!
//e.g: [DF][long][CF][RO][l][NAME][DF][T][cpu][bat][a0][a1][a2][a3][a4][cs]
//      1    4    1   1   1   3    1   1   2    2    2   2   2   2   2   2 = 29
#define BOXNAME "P11"
#define SENSORANZAHL 4

#include <BayEOSBufferSPIFlash.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayRF24.h>
BayRF24 client = BayRF24(9, 10);

#include <LowCurrentBoard.h>

void setup()
{
  client.init(RF24ADDRESS, RF24CHANNEL);
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); //use skip!
  initLCB(); //init time2
  pinMode(POWER_PIN,OUTPUT);
  startLCB();
  
}

float bat=4.0;

void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    client.startDataFrameWithOrigin(BayEOS_Int16le,BOXNAME,1,1);
    client.addChannelValue(millis());
    client.addChannelValue(1000 * bat);
    digitalWrite(POWER_PIN,HIGH);
    delay(15);
    for(uint8_t i=0;i<SENSORANZAHL;i++){
       client.addChannelValue(analogRead(A0+i));
    }
    client.addChecksum();
    bat=3.3*2*analogRead(A7)/1023;
    digitalWrite(POWER_PIN,LOW);
    sendOrBufferLCB();

  }

  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
  }
  sleepLCB();

}



