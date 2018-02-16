/*
 * This is a sample sketch to test the function of the class
 * with a dummy device
 * 
 * please adjust SAMPLING_INT, TRY_INT when 
 * using with a real device
 */
#include <BayDebug.h>
#include <BayEOSBufferSDFat.h>
#include <Sleep.h>
#include <RAD7Serial.h>

#define SD_CSPIN 4
#define SAMPLING_INT 10000L
#define TRY_INT 10000L

BayDebug client(Serial);
BayEOSBufferSDFat myBuffer;
RAD7Serial myRAD7(Serial1, client);

unsigned long last_data_try, last_data, last_buffered_data;
int res;

void setup(void) {
  client.begin(9600,1);
  Serial1.begin(1200);
  Serial.println("Starting...");
  Serial.println("SD");

  while (!SD.begin(SD_CSPIN)) {
    Serial.println("No SD");
    delay(2000);
  }
  myBuffer = BayEOSBufferSDFat(2000000000L, 1); //Append mode!
  client.setBuffer(myBuffer, 0);
  last_data -= SAMPLING_INT;
  last_data_try -= SAMPLING_INT;

  Serial.println("Setup OK");
  
}

void loop(void) {
  if (myBuffer.available() && (millis() - last_buffered_data) > 10000) {
    Serial.println("sending");
    while(myBuffer.available() && ! client.sendFromBuffer()){
      
    }
  }

  if ((millis() - last_data) > SAMPLING_INT
      && (millis() - last_data_try) > TRY_INT ) {
    Serial.println("reading");
    last_data_try = millis();

    if (myRAD7.get_rnumber() > 100)
      myRAD7.deleteData();

    res = myRAD7.readData();
    if (res > 0) last_data = millis();
  }
}

