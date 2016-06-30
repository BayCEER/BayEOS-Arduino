#include <BayEOSBuffer.h>
#include <BayEOSBufferRAM.h>
#include <BayEOS.h>
#include <BayDebug.h>


BayDebug client = BayDebug(Serial);
BayEOSBufferRAM myBuffer;
unsigned long last_data;
unsigned long last_buffered_data;


void setup(void) {
  client.begin(9600);
  myBuffer = BayEOSBufferRAM(237);
  client.setBuffer(myBuffer);
}

int i;
void loop(void) {

  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(i);
  client.addChannelValue(millis() / 1000);
  if ((i % 2) == 0) client.addChannelValue(millis() / 1000);
  if ((i % 3) == 0) client.addChannelValue(millis() / 1000);
  if ((i % 4) == 0) client.addChannelValue(millis() / 1000);
  client.writeToBuffer();
  client.sendFromBuffer();
  i++;
  Serial.print(myBuffer.readPos());
  Serial.print("\t");
  Serial.print(myBuffer.writePos());
  Serial.print("\t");
  Serial.print(myBuffer.endPos());
  Serial.print("\t");
  Serial.println(myBuffer.available());

  delay(1000);
}
