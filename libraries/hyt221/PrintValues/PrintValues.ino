#include <Wire.h>
#include <hyt221.h>

void setup(void){
  Serial.begin(9600);
  Wire.begin();
}

void loop(void){
 float temp,hum;
 hyt221_measure(&temp,&hum);
 Serial.print("Temp: ");
 Serial.println(temp); 
 Serial.print("Hum: ");
 Serial.println(hum);
 Serial.println();
 delay(1000); 
  
}
