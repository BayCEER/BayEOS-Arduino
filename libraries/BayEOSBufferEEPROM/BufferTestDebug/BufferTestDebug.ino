#include <BayEOSBuffer.h>
#include <Wire.h>
#include <I2C_eeprom.h>
#include <BayEOSBufferEEPROM.h>
#include <BayEOS.h>
#include <BayDebug.h>


//Set 0 for single EEPROM
#define MULTIEEPROM 1

#if MULTIEEPROM
uint8_t i2c_addresses[]={0x50,0x51,0x52,0x53};
BayEOSBufferMultiEEPROM myBuffer;
#else
BayEOSBufferEEPROM myBuffer;
#endif

BayDebug client = BayDebug(Serial);

void setup(void) {
  client.begin(9600, 1);
  Serial.println("Starting...");
  delay(10);
#if MULTIEEPROM
  myBuffer.init(2,i2c_addresses,65536L); //with flush
  //myBuffer.reset(); //uncomment to clear buffer
 #else
 myBuffer.init(0x50, 65536L,0); //no flush!
#endif
  //myBuffer.reset(); //This will set all pointers to zero
  client.setBuffer(myBuffer);
}

int i;
void loop(void) {

  //Construct DataFrame
  client.startDataFrame(BayEOS_Float32le);
  client.addChannelValue(i);
  if ((i % 2) == 0) client.addChannelValue(millis() / 1000);
  if ((i % 3) == 0) client.addChannelValue(millis() / 1000);
  if ((i % 4) == 0) client.addChannelValue(millis() / 1000);
  Serial.println("added Frame");
  client.writeToBuffer();
  Serial.print(myBuffer.readPos());
  Serial.print("\t");
  Serial.print(myBuffer.writePos());
  Serial.print("\t");
  Serial.print(myBuffer.endPos());
  Serial.print("\t");
  Serial.println(myBuffer.available());
  if ((i % 4) == 3) {
    client.sendFromBuffer();
    myBuffer.initNextPacket();
    myBuffer.next();
    Serial.println("popped one Frame");
    Serial.print(myBuffer.readPos());
    Serial.print("\t");
    Serial.print(myBuffer.writePos());
    Serial.print("\t");
    Serial.print(myBuffer.endPos());
    Serial.print("\t");
    Serial.println(myBuffer.available());
  }
  i++;
  delay(1);
}
