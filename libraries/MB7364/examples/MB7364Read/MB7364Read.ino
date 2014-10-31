/*
  Read Distance of HRXL MaxSonar 7364
*/

#include <MB7364.h>
#include <SoftwareSerial.h>

MB7364 sonar(10,11);

void setup(void){
  Serial.begin(9600);      
  sonar.begin();
}
  
void loop(void){ 
  Serial.println(sonar.range());       
  delay(5000);      
} 

