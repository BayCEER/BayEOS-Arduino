/*
 * Example file for Arudino-Mega
 * 
 * Connect 
 * 
 * RX to TX1 (18) 
 * TX to RX1 (19)
 * 
 */


 #include <RD200M.h>

RD200M radon(Serial1);

 void setup(void){
    Serial.begin(9600);
    Serial.println("Starting");
    radon.init();
 }

 void loop(void){
  radon.update();
  if(radon._ready){
    Serial.print(radon._status);
    Serial.print("\t");
    Serial.print(radon._up);
    Serial.print("\t");
    Serial.print(radon._elapsed);
    Serial.print("\t");
    Serial.println(radon._value);
  } else {
    Serial.println("RD200M not ready");
  }
  delay(3000);
  
 }

