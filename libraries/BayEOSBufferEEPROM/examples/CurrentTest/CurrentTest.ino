/*
   Simple Sketch for BayEOSBufferEEPROM

                avr.  max.  time
   Power Down   1.4µA
   Read         5mA   6mA  2,327ms
   Write        5mA   6mA  10.39ms
   Write+Flush  5mA   6mA  24.59ms

*/
#include <BayEOSBufferEEPROM.h>
#include <BayDebug.h>
#include <Sleep.h>

volatile unsigned long seconds = 0;

//The very important 32.686kHz interrupt handler
ISR(TIMER2_OVF_vect) {
  //seconds++;
  seconds += 1; //Adjust to your timer2 time!
}


BayDebug client = BayDebug(Serial);
BayEOSBufferEEPROM myBuffer;

void setup(void) {
  client.begin(9600, 1);
  Serial.print("Starting...");
  delay(10);
  myBuffer.init(0x50, 65536L); //This will restore old pointers!
  //  myBuffer.init(0x50, 65536L,0); //no flush!
  //myBuffer.reset(); //This will set all pointers to zero
  client.setBuffer(myBuffer);
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
    client.writeToBuffer();
  }
  
  Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);    // sleep function called here


}
