#include <ltc1290.h>

LTC1290 myLTC=LTC1290(23,22,21,20); 

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  myLTC.init();
}
unsigned long start,ende;
int value;
int values[8];
uint8_t rep=2;
// the loop routine runs over and over again forever:
void loop() {
  start=millis();
  myLTC.read(values,rep);
  ende=millis();
  for(uint8_t i=0;i<8;i++){
    //value=myLTC.read(i,rep);
    value=values[i];
    Serial.print(value);
    Serial.print(" ");
  }
//  ende=millis();
  Serial.println();
  Serial.print((1<<(rep+3)));
  Serial.print(" Samples in ");
  Serial.print(ende-start);
  Serial.println(" ms");
  Serial.print((float)(ende-start)/(1<<(rep+3)));
  Serial.println(" ms per Sample");
  
  
  delay(1000);        
}
