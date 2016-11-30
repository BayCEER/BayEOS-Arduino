/*
   Simple Sketch for BayEOSBufferSPIFlash

   Connect SPIFlash to Hardware SPI
   CS Pin is 10


*/
#include <BayEOSBufferSPIFlash.h>
#include <BayEOS.h>
#include <BayDebug.h>


BayDebug client = BayDebug(Serial);
SPIFlash flash(10);
BayEOSBufferSPIFlash myBuffer;
unsigned long last_data = 10000;
unsigned long last_buffered_data;
unsigned long start;

void setup(void) {
  client.begin(9600, 1);
  Serial.print("Starting...");
  delay(10);
  start = millis();
  myBuffer.init(flash, 10); //This will restore old pointers
  //only stores Pointer positions each 10th write!
  //myBuffer.reset(); //This will set all pointers to zero
  client.setBuffer(myBuffer);
  Serial.print(" (");
  Serial.print(millis() - start);
  Serial.println("ms)");

}

void loop(void) {

  //Resend buffered frames
  //one every second
  //sending to frequently my make xbee operationable (channel overload)
  if ((millis() - last_buffered_data) > 500) {
    client.sendFromBuffer();
    last_buffered_data = millis();

    //Uncomment to get information about the buffer pointer positions
    /*
      Serial.print("Read-Pos: ");
      Serial.print(myBuffer.readPos());
      Serial.print(" - End-Pos: ");
      Serial.print(myBuffer.endPos());
      Serial.print(" - Write-Pos: ");
      Serial.println(myBuffer.writePos());
    */
  }

  if ((millis() - last_data) > 2000) {
    last_data = millis();

    //Construct DataFrame
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis() / 1000);
    //client.sendOrBuffer();
    Serial.print("Writing DF from ");
    Serial.print(myBuffer.writePos());
    Serial.print(" to ");
    start = millis();
    client.writeToBuffer();
    Serial.print(myBuffer.writePos());
    Serial.print(" (");
    Serial.print(millis() - start);
    Serial.println("ms)");

    //Construct Message
    client.startFrame(BayEOS_Message);
    client.addToPayload("Just a message ;-)");
    //client.sendOrBuffer();
    Serial.print("Writing Message from ");
    Serial.print(myBuffer.writePos());
    Serial.print(" to ");
    start = millis();
    client.writeToBuffer();
    Serial.print(myBuffer.writePos());
    Serial.print(" (");
    Serial.print(millis() - start);
    Serial.println("ms)");
  }

}
