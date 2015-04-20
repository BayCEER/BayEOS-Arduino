/*
  Read and print PWD11 values 
*/

#include <PWD11.h>
#include <SoftwareSerial.h>

PWD11 pwd11(8,9);

void setup(void){
  Serial.begin(9600);      
  pwd11.begin();
}
  
void loop(void){   
  float* v;
  v = pwd11.readMessage2();  
  for(int i=0;i<10;i++){
	Serial.print(i);       
	Serial.print(":");       
	Serial.println(v[i]);       
  }  
  delay(100);      
} 

