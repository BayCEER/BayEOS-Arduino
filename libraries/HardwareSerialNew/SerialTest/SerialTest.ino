/*
* Serial Test
*/
#include <HardwareSerialNew.h>

#define BUFFER_SIZE 128
char buffer[BUFFER_SIZE];
int count=0;

#define R_BUFFER_SIZE 256
unsigned char buffer_cb_rx[R_BUFFER_SIZE];
unsigned char buffer_cb_tx[R_BUFFER_SIZE];



void setup(void){

  Serial.setRxBuffer(buffer_cb_rx,R_BUFFER_SIZE);
  Serial.setTxBuffer(buffer_cb_tx,R_BUFFER_SIZE);
  delay(10);
  Serial.begin(9600);
}

void loop(void){
  for(uint8_t i=0;i<20;i++){
    Serial.print(i);
    Serial.println(" - waiting...");
    delay(200);
  }


  count=0;
  while(Serial.available() && count<BUFFER_SIZE){
    delay(1);
    buffer[count]=Serial.read();
    if(buffer[count]=='\n'){
      buffer[count]=0;
      break;
    }
    count++;
  }
  
  
  Serial.print("got: ");
  count=0;
  while(buffer[count]){
    Serial.print(buffer[count]);
    count++;
  }
  buffer[0]=0;
  Serial.println();
}
    
