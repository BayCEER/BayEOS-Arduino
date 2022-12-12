/*
   Test-Sketch from ADC-R-Board


  A) ELECTRODES
       |--R_PRE1--A0---E1+ E1- -|
  D6 --|--R_PRE2--A1---E2+ E2- -|-- D4
       |--R_PRE3--A2---E3+ E3- -|

  B) NTC05
       |--R_NRC1--A3--  NTC1 --|
  D3 --|--R_NTC2--A4--  NTC2 --|-- GND
       |--R_NTC3--A5--  NTC3 --|

  C) RAIN

  D2 -- RAIN -- GND

*/

#define R_NTC 10000.0
#define R_PRE 100000.0

#include <NTC.h>
NTC_ADC ntc1(3, A3, R_NTC, 5.0);
NTC_ADC ntc2(3, A4, R_NTC, 5.0);
NTC_ADC ntc3(3, A5, R_NTC, 5.0);

#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;

#include <BayDebug.h>
BayDebug client(Serial);

#define ACTION_COUNT 1
#define SAMPLING_INT 4
#include <LowCurrentBoard.h>

void setup()
{
  pinMode(POWER_PIN, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
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
  Serial.println("Test done");
  Serial.flush();
  startLCB();

}


void loop() {
  if (ISSET_ACTION(0)) {
    UNSET_ACTION(0);
    //eg measurement

    client.startDataFrame(BayEOS_Float32le);
    client.addChannelValue(millis());
    //Ausgabe der Spannung in mV bei 10V Anregung!
    client.addChannelValue(ntc1.getTemp());
    client.addChannelValue(ntc2.getTemp());
    client.addChannelValue(ntc3.getTemp());

    digitalWrite(6, HIGH);
    client.addChannelValue(R_PRE / (1023.0 / analogRead(A0) - 1));
    client.addChannelValue(R_PRE / (1023.0 / analogRead(A1) - 1));
    client.addChannelValue(R_PRE / (1023.0 / analogRead(A2) - 1));
    digitalWrite(6, LOW);
    digitalWrite(4, HIGH);
    client.addChannelValue(R_PRE * (1023.0 / analogRead(A0) - 1));
    client.addChannelValue(R_PRE * (1023.0 / analogRead(A1) - 1));
    client.addChannelValue(R_PRE * (1023.0 / analogRead(A2) - 1));
    digitalWrite(4, LOW);
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



