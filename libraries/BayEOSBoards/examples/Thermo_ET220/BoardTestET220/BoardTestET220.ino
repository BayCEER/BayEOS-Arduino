#include <BayEOSBufferSPIFlash.h>
float span;

SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;
#include <HX711Array.h>
uint8_t dout[] = { A2, A1, A0 };
uint8_t sck = 6;
long adc[3];
HX711Array scale;

#include <DS18B20.h>
DS18B20 ds = DS18B20(A3, 0, 1);

#define SW10_PW 4
#define SW10_AD A6


#include <BayDebug.h>
BayDebug client(Serial);

#define ACTION_COUNT 1
#include <LowCurrentBoard.h>

void setup() {
  client.begin(9600);
  Serial.println("Starting Test");
  myBuffer.init(flash);  //This will restore old pointers
  Serial.print("Flash: ");
  Serial.println(flash.getCapacity());
  Serial.flush();
  //myBuffer.reset(); //This will set all pointers to zero
  myBuffer.skip();             //This will move read pointer to write pointer
  myBuffer.setRTC(myRTC, 0);   //Nutze RTC relativ!
  client.setBuffer(myBuffer);  //use skip!
  initLCB();                   //init time2
  scale.begin(dout, 3, sck);   //start HX711Array with 3 ADCs
  scale.set_gain(128);
  readBat();
  scale.power_down();
  ds.setAllAddr();
  ds.setResolution(12);
  pinMode(SW10_PW, OUTPUT);

  Serial.println("Test done");
  Serial.flush();
  startLCB();
}


void readBat() {
  analogReference(DEFAULT);
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);
  batLCB = 3.3 * 200 / 100 / 1023 * analogRead(A7);
  digitalWrite(POWER_PIN, LOW);
  pinMode(POWER_PIN, INPUT);
}

void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement
    ds.t_conversion();  //Start T-conversion
    delayLCB(700);
    scale.power_up();
    scale.read_average(adc);
    readBat();
    scale.power_down();
    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    client.addChannelValue(1000 * batLCB);
    float temp;
    ds.readChannel(1, &temp);
    client.addChannelValue(temp);
//Themospannung: 4,1mV/100°C
    client.addChannelValue(temp+3.3 / 128 / 256 / 256 / 256 / 4.1 *100 *100 * adc[0]);
    client.addChannelValue(temp+3.3 / 128 / 256 / 256 / 256 / 4.1 *100 *100 * adc[1]);
    client.addChannelValue(temp+3.3 / 128 / 256 / 256 / 256 / 4.1 *100 *100 * adc[2]);
    digitalWrite(SW10_PW,HIGH);
    delayLCB(15);
    client.addChannelValue(analogRead(SW10_AD));
    digitalWrite(SW10_PW,LOW);
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
