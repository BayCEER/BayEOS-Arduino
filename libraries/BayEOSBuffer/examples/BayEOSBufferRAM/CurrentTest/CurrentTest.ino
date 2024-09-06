/*
   Simple Sketch for BayEOSBufferRAM

                avr.  max.  time
   Power Down   1µA
   Read         4mA   5mA  0.48ms
   Write        4mA   5A   0.69ms

*/
#include <BayEOSBufferRAM.h>
#include <BayDebug.h>
#include <Sleep.h>

volatile unsigned long seconds = 0;

//The very important 32.686kHz interrupt handler
ISR(TIMER2_OVF_vect) {
  //seconds++;
  seconds += 1; //Adjust to your timer2 time!
}


BayDebug client = BayDebug(Serial);
BayEOSBufferRAM myBuffer;

void setup(void) {
  client.begin(9600, 1);
  Serial.print("Starting...");
  myBuffer = BayEOSBufferRAM(1000);
  client.setBuffer(myBuffer);
  Sleep.setupTimer2(); //init timer2 to 1 sec
  Serial.print("Done");
  delay(100);
}

void loop(void) {
  if ((seconds % 2) == 1){
   // client.sendFromBuffer();
    client.readFromBuffer();
    myBuffer.next();
//    delay(100);
  }

  if ((seconds % 1) == 0) {
    //delay(200);
    //Construct DataFrame
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    client.writeToBuffer();
  }
  
  Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE);    // sleep function called here


}
