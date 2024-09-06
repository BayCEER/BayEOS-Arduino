#include "BayBluetooth.h"
BayBluetooth::BayBluetooth(HardwareSerial &serial):BaySerial(serial){
}


void BayBluetooth::inquirable(void){
	  _serial->print("\r\n+STPIN=0000\r\n"); // Set pin to 0000
	  delay(2000); // This delay is required.
	  _serial->print("\r\n+INQ=1\r\n"); //make the slave bluetooth inquirable
	  delay(2000); // This delay is required.
	  flush();
}

void BayBluetooth::begin(long baud,const char* name){
	  BaySerial::begin(baud);
	  _serial->print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
	  _serial->print("\r\n+STNA=");
	  _serial->print(name);
	  _serial->print("\r\n"); //set the bluetooth name
	  _serial->print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
	  _serial->print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
	  _serial->flush();
}

void BayBluetooth::begin(long baud,int eeprom_offset,uint8_t start_byte){
	  BaySerial::begin(baud);
	  _serial->print("\r\n+STWMOD=0\r\n"); //set the bluetooth work in slave mode
	  _serial->print("\r\n+STNA=");
      if(EEPROM.read(eeprom_offset)==start_byte){
    	  uint8_t i=1;
    	  while(char n=EEPROM.read(eeprom_offset+i)){
          _serial->print(n);
          i++;
    	  }
      } else _serial->print("NA");
      _serial->print("\r\n"); //set the bluetooth name
      _serial->print("\r\n+STOAUT=1\r\n"); // Permit Paired device to connect me
      _serial->print("\r\n+STAUTO=1\r\n"); // Auto-connection should be forbidden here
      _serial->flush();
}
