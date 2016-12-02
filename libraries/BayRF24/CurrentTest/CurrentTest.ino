/*
   Simple Sketch for BayEOSBufferRAM

                avr.  max.  time
   Power Down   1µA
   Read         4mA   5mA  0.48ms
   Write        4mA   5A   0.69ms

*/
#include <BayEOS.h>
#include <SPI.h>
#include <RF24.h>
#include <BayEOS.h>
#include <BayRF24.h>
#include <Sleep.h>

volatile unsigned long seconds = 0;

#define RF24ADDRESS 0x45c431ae12LL
#define RF24CHANNEL 0x72

BayRF24 client=BayRF24(9,10);

//The very important 32.686kHz interrupt handler
SIGNAL(TIMER2_OVF_vect) {
  //seconds++;
  seconds += 1; //Adjust to your timer2 time!
}



void setup(void) {
  Serial.begin(9600);
  Serial.print("Starting...");
  client.init(RF24ADDRESS,RF24CHANNEL);
  Sleep.setupTimer2(); //init timer2 to 1 sec
  Serial.print("Done");
  delay(100);
}

void loop(void) {
  if ((seconds % 1) == 0) {
    //delay(200);
    //Construct DataFrame
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    client.sendPayload();
  }
  
  Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);    // sleep function called here


}
