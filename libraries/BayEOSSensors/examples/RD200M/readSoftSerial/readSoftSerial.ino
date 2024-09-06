/*
 * Example file for SoftwareSerial
 * 
 * Connect 
 * RX to 10 (SoftwareSerial TX)
 * TX to 11 (SorfwareSerial RX)
 * 
 * Tested with ArduinoMega
 * 
 */


 #include <RD200M.h>

RD200MSoftserial radon(11,10);

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
