#include <BayEOSBufferSPIFlash.h>
#include <DendroET208.h>

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayDebug.h>
BayDebug client(Serial);

#define ACTION_COUNT 1
#include <LowCurrentBoard.h>

void setup()
{
  client.begin(9600);
  Serial.println("Starting Test");
  myBuffer.init(flash); //This will restore old pointers
  Serial.print("Flash: ");
  Serial.println(flash.getCapacity());
  Serial.flush();
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0); //Nutze RTC relativ!
  client.setBuffer(myBuffer); //use skip!
  initLCB(); //init time2
  initDendro();
  startLCB();
}


void loop() {
 Serial.print("Calibration...");
  Serial.flush();
  adc.read(1); //read with calibration
  adc.read(); //read once without calibration
  Serial.println("done");
  Serial.flush();
  Serial.println(readDendro());
     Serial.flush();
}
