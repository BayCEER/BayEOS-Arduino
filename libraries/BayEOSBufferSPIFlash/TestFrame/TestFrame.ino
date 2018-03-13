/*
   Simple Sketch for BayEOSBufferSPIFlash

   Connect SPIFlash to Hardware SPI
   CS Pin is 10


*/
#include <BayEOSBufferSPIFlash.h>
#include <BayDebug.h>


BayDebug client = BayDebug(Serial);
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
unsigned long last_data = 10000;
unsigned long last_buffered_data;
unsigned long start;

void setup(void) {
  client.begin(9600, 1);
  Serial.print("Starting...");
  delay(10);
  start = micros();
  myBuffer.init(flash); //This will restore old pointers
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip(); //This will move read pointer to write pointer
  start=micros()-start;
  client.setBuffer(myBuffer);
  Serial.print(" (");
  Serial.print(start);
  Serial.println("µs)");

}

void loop(void) {

  //Resend buffered frames
  //one every second
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
    start = micros();
    client.writeToBuffer();
    start=micros()-start;
    Serial.print(myBuffer.writePos());
    Serial.print(" (");
    Serial.print(start);
    Serial.println("µs)");

    //Construct Message
    client.startFrame(BayEOS_Message);
    client.addToPayload("Just a message ;-)");
    //client.sendOrBuffer();
    Serial.print("Writing Message from ");
    Serial.print(myBuffer.writePos());
    Serial.print(" to ");
    start = micros();
    client.writeToBuffer();
    start=micros()-start;
    Serial.print(myBuffer.writePos());
    Serial.print(" (");
    Serial.print(start);
    Serial.println("µs)");
  }

}
