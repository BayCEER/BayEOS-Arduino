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

void printBufferPointer(void) {
  Serial.print("Read-Pos: ");
  Serial.print(myBuffer.readPos());
  Serial.print(" - End-Pos: ");
  Serial.print(myBuffer.endPos());
  Serial.print(" - Write-Pos: ");
  Serial.print(myBuffer.writePos());
  Serial.print(" - Length: ");
  Serial.println(myBuffer.length());
}

void setup(void) {
  client.begin(38400, 1);
  Serial.print("Starting...");
  delay(10);
  myBuffer.init(flash); //This will restore old pointers
  client.setBuffer(myBuffer);
  Serial.println("Commands:");
  Serial.println("i: Buffer Init");
  Serial.println("x: Buffer Reset");
  Serial.println("s: Buffer Skip");
  Serial.println("F: full test");
  Serial.println("R: Run test for 100 Frames");
  Serial.println("a: Add Frame");
  Serial.println("r: read Frame");
  Serial.println("w: Buffer wrap around");

}

long count;
uint8_t percent = 0;
uint8_t new_percent = 0;

void loop(void) {
  if (Serial.available()) {
    char b = Serial.read();
    switch (b) {
      case 'a':
        client.startDataFrame();
        client.addChannelValue(millis());
        client.writeToBuffer();
        printBufferPointer();
        break;
      case 'r':
        if (myBuffer.available())
          client.sendFromBuffer();
        Serial.print("Length: ");
        Serial.println(client.getPacketLength());
        printBufferPointer();
        break;
      case 'i':
        myBuffer.init(flash); //This will set all pointers to zero
        printBufferPointer();
        break;
      case 'x':
        myBuffer.reset(); //This will set all pointers to zero
        printBufferPointer();
        break;
      case 's':
        myBuffer.skip(); //This will move read pointer to write pointer
        printBufferPointer();
        break;
      case 'F':
      case 'R':
        percent = 0;
        new_percent = 0;
        long num_frames;
        if (b == 'R') num_frames = 100;
        else num_frames = myBuffer.length() / (5 + 3 + 40);
        myBuffer.skip();
        printBufferPointer();
        for (long i = 0; i < num_frames; i++) {
          new_percent = i * 100 / num_frames;
          if (new_percent != percent) {
            if (new_percent > 100) new_percent = 100;
            Serial.print(new_percent);
            Serial.println("% done");
            printBufferPointer();
            percent = new_percent;
          }
          client.startDataFrame(BayEOS_Int32le);
          for (uint8_t j = 0; j < 10; j++) {
            client.addChannelValue(count);
          }
          if (client.writeToBuffer()){
            if (! client.readFromBuffer()) {
              Serial.print("Read failed g  ");
              printBufferPointer();
              myBuffer.skip();

            } else {
              uint8_t error = 0;
              for (uint8_t j = 0; j < 10; j++) {
                if (*(long*)(client.getPayload() + 8 + (j * 4)) != count){
                  error++;
                }
              }
              if (error) {
                Serial.print("Read failed #");
                Serial.print(error);
                Serial.print(" ");
                printBufferPointer();
                client.sendPayload();
              }
            }
          } else {
            Serial.print("Write failed  ");
            printBufferPointer();
          }
          myBuffer.next();
          count++;
        }
        printBufferPointer();
        break;
      case '\n':
      case '\r':
        break;
      case 'w':
        percent = 0;
        new_percent = 0;
        unsigned long start = myBuffer.endPos();
        Serial.println("Running Wrap around");
        unsigned long bytes = myBuffer.length() - myBuffer.writePos() + start;
        if (bytes > myBuffer.length()) bytes -= myBuffer.length();
        unsigned long bytes_left = myBuffer.length() - myBuffer.writePos() + start;
        if (bytes_left > myBuffer.length()) bytes_left -= myBuffer.length();
        while (myBuffer.endPos() == start) {
          client.startDataFrame();
          client.addChannelValue(millis());
          client.addChannelValue(millis());
          client.addChannelValue(millis());
          client.addChannelValue(millis());
          client.addChannelValue(millis());
          client.addChannelValue(millis());
          client.addChannelValue(millis());
          client.addChannelValue(millis());
          client.writeToBuffer();
          bytes_left = myBuffer.length() - myBuffer.writePos() + start;
          if (bytes_left > myBuffer.length()) bytes_left -= myBuffer.length();
          new_percent = 100 - 100 * (float)(bytes_left) / bytes;
          if (new_percent != percent) {
            if (new_percent > 100) new_percent = 100;
            Serial.print(new_percent);
            Serial.println("% done");
            printBufferPointer();
            percent = new_percent;
          }
        }
        break;
      default:
        Serial.println("unknown command");
    }

  }



}
