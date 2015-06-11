#include <Wire.h>
#include <hyt221.h>

void setup(void){
  Serial.begin(9600);
  Wire.begin();
}

void loop(void){
 float temp,hum;
 long starttime=micros();
 hyt221_measure(&temp,&hum);
 long endtime=micros();
 Serial.print("Temp: ");
 Serial.println(temp); 
 Serial.print("Hum: ");
 Serial.println(hum);
 Serial.print("Measurement took ");
 Serial.print(endtime-starttime);
 Serial.println("us");
 Serial.println();
 
 delay(1000); 
  
}
