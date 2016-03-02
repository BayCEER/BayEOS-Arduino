/*
* Serial Test
*/
#include <HardwareSerialPlus.h>


#define R_BUFFER_SIZE 512
unsigned char buffer_cb_rx[R_BUFFER_SIZE];
unsigned char buffer_cb_tx[R_BUFFER_SIZE];



void setup(void){
  SerialPlus.setRxBuffer(buffer_cb_rx,R_BUFFER_SIZE);
  SerialPlus.setTxBuffer(buffer_cb_tx,R_BUFFER_SIZE);
  delay(10);
  SerialPlus.begin(9600);

}

void loop(void){
  for(uint8_t i=0;i<20;i++){
    SerialPlus.print(i);
    SerialPlus.println(" - waiting...");
    delay(200);
  }

/*
  SerialPlus.print(SerialPlus._rx_buffer_head);
  SerialPlus.print(" ");
  SerialPlus.print(SerialPlus._rx_buffer_tail);
  SerialPlus.print(" ");
  SerialPlus.print(SerialPlus._tx_buffer_head);
  SerialPlus.print(" ");
  SerialPlus.println(SerialPlus._tx_buffer_tail);
*/ 
  SerialPlus.print("got: ");
  SerialPlus.println(SerialPlus.available());
  while(SerialPlus.available()){
    SerialPlus.print((char) SerialPlus.read());
  }
  SerialPlus.println();
  SerialPlus.println();
}
    
