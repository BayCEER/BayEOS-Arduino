#include "RD200M.h"


void RD200M_interface::reset(void) {
	write(_reset, 4);
	_ready = false;
}

void RD200M_interface::init(void) {
	begin(19200);
	_ready = false;
}


void RD200M_interface::update(void) {
	while(i_available()) read();
	write(req, 4);
	_ready = false;
	_state=0;
	unsigned long start_time=millis();
	while(true){
		while(! i_available()){
			if((millis()-start_time)>20000){
				return;
			}
		}
		byte c = read();

		switch (_state) {
		case 0:
			if (c == 0x02) {
				_state = 1;
				_csum = 0;
			}
			break;
		case 1:
			if (c == 0x10) {
				_state = 2;
				_csum += c;
			} else {
				_state = 0;
				return;
			}
			break;
		case 2:
			if (c == 0x04) {
				_state = 3;
				_csum += c;
			} else {
				_state = 0;
				return;
			}
			break;
		case 3: //Status Byte
			_state = 4;
			_csum += c;
			_status = c;
			_up = 0x3 & c;
			break;
		case 4: //Uptime
			_elapsed = c;
			_csum += c;
			_state = 5;
			break;
		case 5: //Value higher Value
			_value = c * 100;
			_state = 6;
			_csum += c;
			break;
		case 6: //Value lower Value
			_value += c;
			_state = 7;
			_csum += c;
			break;
		case 7:
			_csum += c;
			if (0xff != _csum) {
				_ready = false; //checksum error
			} else
				_ready = true;
			_state = 0;
			return;
			break;
		}
	}
}

RD200M::RD200M(HardwareSerial &serial) {
	_serial = &serial;
}

RD200MSoftserial::RD200MSoftserial(uint8_t rxPin, uint8_t txPin):SoftwareSerial(rxPin,txPin){
}
