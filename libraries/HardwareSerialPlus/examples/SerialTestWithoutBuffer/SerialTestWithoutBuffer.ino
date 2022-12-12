/*
* Serial Test
*/
#include <HardwareSerialPlus.h>

#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];
int count=0;




void setup(void){

  delay(10);
  SerialPlus.begin(9600);
}

void loop(void){
  for(uint8_t i=0;i<20;i++){
    SerialPlus.print(i);
    SerialPlus.println(" - waiting...");
    delay(200);
  }


  count=0;
  while(SerialPlus.available() && count<BUFFER_SIZE){
    delay(1);
    buffer[count]=SerialPlus.read();
    if(buffer[count]=='\n'){
      buffer[count]=0;
      break;
    }
    count++;
  }
  
  
  SerialPlus.print("got: ");
  count=0;
  while(buffer[count]){
    SerialPlus.print(buffer[count]);
    count++;
  }
  buffer[0]=0;
  SerialPlus.println();
}
    
