/*
   Simple Sketch for BayEOSBufferSPIFlash

   Connect SPIFlash to Hardware SPI
   CS Pin is 10
                avr.  max.  time
   Power Down   6µA
   Read         5mA   15mA  0.54ms
   Write        5mA   20mA  1.14ms
   Write+Flush  5mA   20mA  2.18ms
   Erase        25mA        50ms

*/
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BayDebug.h>
#include <Sleep.h>

volatile unsigned long seconds = 0;
#define WITH_FLASH 1


//The very important 32.686kHz interrupt handler
SIGNAL(TIMER2_OVF_vect) {
  //seconds++;
  seconds += 1; //Adjust to your timer2 time!
}


BayDebug client = BayDebug(Serial);
#if WITH_FLASH
SPIFlash flash(10);
BayEOSBufferSPIFlash myBuffer;
#endif

void setup(void) {
  client.begin(9600, 1);
  Serial.print("Starting...");
  delay(10);
#if WITH_FLASH
  myBuffer.init(flash, 10); //This will restore old pointers
  //only stores Pointer positions each 10th write!
  //myBuffer.reset(); //This will set all pointers to zero
  client.setBuffer(myBuffer);
#endif
  Sleep.setupTimer2(); //init timer2 to 1 sec
}

void loop(void) {
  if ((seconds % 2) == 1){
    //client.sendFromBuffer();
    client.readFromBuffer();
    myBuffer.next();
    //delay(100);
  }

  if ((seconds % 2) == 0) {
    //delay(200);
    //Construct DataFrame
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
#if WITH_FLASH
    client.writeToBuffer();
#endif
  }
  
  Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);    // sleep function called here


}
