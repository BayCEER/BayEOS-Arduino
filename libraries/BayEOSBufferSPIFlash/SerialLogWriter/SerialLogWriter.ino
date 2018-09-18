/*
  Serial Log Writer

  This programm has to modes:
  1. Write-Mode
  Write mode is entered, when INT0 (Pin2) is pulled to GND at setup.
  Wait until LED will light up constantly.
  In Save-Mode the Buffer is reseted in setup(). 
  Serial input will get saved line by line. For each save LED will flash shortly.

  2. Print-Mode
  Print mode will print out all saved lines with milliseconds time 
  at start of the line.


*/
#include <BayEOSBufferSPIFlash.h>
SPIFlash flash(8);
BayEOSBufferSPIFlash myBuffer;


#define BAUD_RATE 9600
#define LED_PIN 5

uint8_t mode;

void setup(void) {
  pinMode(LED_PIN, OUTPUT);
  for (uint8_t i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(400);
    digitalWrite(LED_PIN, LOW);
    delay(400);
  }

  pinMode(2, INPUT_PULLUP);
  mode = digitalRead(2);
  myBuffer.init(flash,3); //This will restore old pointers

  Serial.begin(9600);
  if (! mode) {
    myBuffer.reset();
    digitalWrite(LED_PIN, HIGH);
  } else {
    myBuffer.seekReadPointer(0);
    Serial.print("Write-Pos: ");
    Serial.println(myBuffer.writePos());
  }
}

char buffer[100];
uint8_t pos=0;
unsigned long led_off;

void loop(void) {
  if (mode) {
    while (myBuffer.available()) {
      myBuffer.initNextPacket();
      Serial.print(myBuffer.packetMillis());
      Serial.print(": ");
      myBuffer.readPacket(buffer);
      for (uint8_t i = 0; i < myBuffer.packetLength(); i++) {
        Serial.print(buffer[i]);
      }
      Serial.println();
      myBuffer.next();
    }
  } else {
    if(! digitalRead(LED_PIN) && (millis()-led_off)>100){
      digitalWrite(LED_PIN,HIGH);
    }
    if(Serial.available()){
      buffer[pos]=Serial.read();
      if((buffer[pos]=='\n' && pos) || pos>90 ){
        digitalWrite(LED_PIN,LOW);
        led_off=millis();
        uint8_t l=pos+(uint8_t)(buffer[pos]!='\n');
       // Serial.println(l);
        myBuffer.addPacket(buffer,l);
        pos=0;
      } else if(buffer[pos]!='\n' && buffer[pos]!='\r') {
        if(pos<99) pos++;
      }
    }
  }

}
