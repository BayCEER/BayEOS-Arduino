/* Simple Class to read MaxSonar range values
*/
#include <SoftwareSerial.h> 
#include <MB7364.h>
#include <inttypes.h>
#include <Arduino.h>

MB7364::MB7364(uint8_t rx, uint8_t tx):SoftwareSerial(rx,tx,true)
{
} 

void MB7364::begin(){
  SoftwareSerial::begin(9600); 
}

int MB7364::range(){
  int index = 0;
  boolean startReading = false;  
  char in[4] = {0};  
    
  int a = SoftwareSerial::available();
  for(int s=0;s<a;s++){
     SoftwareSerial::read(); 
  }

  while(index < 4){
    if(SoftwareSerial::available()){
      byte b = SoftwareSerial::read();
      if (b=='R'){
        startReading = true;        
      } else if (startReading){
        in[index++] = b;
      }
    }      
  }
  return atoi(in);
}