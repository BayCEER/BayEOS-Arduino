#include "BayBluetooth.h"
BayBluetooth::BayBluetooth(HardwareSerial &serial):BaySerial(serial){
}


void BayBluetooth::inquirable(void){
	  print("\r\n+STPIN=0000\r\n"); // Set pin to 0000
	  delay(2000); // This delay is required.
	  print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable
	  delay(2000); // This delay is required.
	  flush();
}

void BayBluetooth::begin(long baud,const char* name){
	  BaySerial::begin(baud);
	  print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
	  print("\r\n+STNA=");
	  print(name);
	  print("\r\n"); //set the bluetooth name
	  print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
	  print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
	  flush();
}

void BayBluetooth::begin(long baud,int eeprom_offset,uint8_t start_byte){
	  BaySerial::begin(baud);
	  print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
	  print("\r\n+STNA=");
      if(EEPROM.read(eeprom_offset)==start_byte){
    	  uint8_t i=1;
    	  while(char n=EEPROM.read(eeprom_offset+i)){
          print(n);
          i++;
    	  }
      } else print("NA");
	  print("\r\n"); //set the bluetooth name
	  print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
	  print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
	  flush();
}
