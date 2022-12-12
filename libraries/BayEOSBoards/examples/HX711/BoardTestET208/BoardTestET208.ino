#include <BayEOSBufferSPIFlash.h>
float span;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
#include <HX711Array.h>
uint8_t dout[] = {6, 4};
uint8_t sck = 3;
long adc[2];
HX711Array scale;

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
  scale.begin(dout, 2, sck); //start HX711Array with 2 ADCs
  scale.set_gain(128);
  scale.power_down();
  Serial.println("Test done");
  Serial.flush();
  pinMode(POWER_PIN, OUTPUT);
  startLCB();

}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement

    scale.power_up();
    scale.read_average(adc);
    scale.power_down();
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    digitalWrite(POWER_PIN, HIGH);
    analogReference(DEFAULT);
    client.addChannelValue(3.3 * (100 + 100) / 100 * analogRead(A7) / 1023);
    digitalWrite(POWER_PIN, LOW);

    //Ausgabe der Spannung in µV bei 3.3V Boardspannung!
    client.addChannelValue(3300000.0 / 128 / 256 / 256 / 256 * adc[0]);
    client.addChannelValue(3300000.0 / 128 / 256 / 256 / 256 * adc[1]);
    sendOrBufferLCB();

    Serial.flush();
  }
  if (ISSET_ACTION(7)) {
    UNSET_ACTION(7);
    client.sendFromBuffer();
    Serial.flush();
  }
  sleepLCB();

}
