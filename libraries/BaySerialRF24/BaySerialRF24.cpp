#include "BaySerialRF24.h"

BaySerialRF24::BaySerialRF24(RF24 &radio, int timeout, uint8_t retries) {
	BaySerialInterface::_timeout = timeout;
	_radio = &radio;
	_retries=retries;
}

int BaySerialRF24::available(void) {
	if (write_pos > 1)
		flush();
	if (length > read_pos)
		return length - read_pos;
	if(! _radio->available()) delay(3);
	while (_radio->available() && read_pos >= length) {
		length = _radio->getDynamicPayloadSize();
		if(! length){
			delay(5);
			continue; // Corrupt payload has been flushed
		}
		_radio->read(buffer, length);
		//Serial.print("R");
		if (buffer[0] != _r_counter) {
			read_pos = 1;
			_r_counter = buffer[0];
			last_activity = millis();
			//Serial.print(millis());
			//Serial.print(_r_counter);
			return length - 1;
		} else
			read_pos = length;

	}
	return 0;
}

int BaySerialRF24::i_available(void) {
	return BaySerialRF24::available();
}

void BaySerialRF24::begin(long baud) {
}

void BaySerialRF24::init(uint8_t ch, uint8_t *adr, uint8_t flush_size) {
	_radio->begin();
	_radio->setChannel(ch);
	_radio->enableDynamicPayloads();
	_radio->setCRCLength(RF24_CRC_16);
	_radio->setDataRate(RF24_250KBPS);
	_radio->setPALevel(RF24_PA_HIGH);
	_radio->setRetries(15, 8);
	_radio->setAutoAck(true);
	_radio->openWritingPipe(adr);
	_radio->openReadingPipe(0, adr);
	_radio->startListening();
	_flush_size = flush_size;
	last_activity = millis();
	write_pos = 1;
	read_pos = 1;
	length = 1;
	_r_counter = 0;
	_w_counter = 0;

}

void BaySerialRF24::flush(void) {
	if (write_pos < 2)
		return;
	_w_counter++;
	if (!_w_counter)
		_w_counter++; //no zero for counters
	buffer[0] = _w_counter;
	//delayMicroseconds(1500);
	if (read_pos) {
		stopListenMode();
	}
	uint8_t tx_try = 0;
	uint8_t res;
	//Serial.print("W");
	//Serial.print(_w_counter);
	//Serial.print(millis());
	while (tx_try < 2) {
		////Serial.print("X");
		res = _radio->write(buffer, write_pos);
		uint8_t curr_pa = 0;
		while (!res && curr_pa < 4) {
			_radio->setPALevel((rf24_pa_dbm_e) curr_pa);
			delayMicroseconds(random(1000));
			res = _radio->write(buffer, write_pos);
			curr_pa++;
		}
		if (res)
			break;
		tx_try++;
		delay(10);
	}
	if (!res) {
		_radio->setPALevel(RF24_PA_HIGH);
		//Serial.print("-");
	}
	write_pos = 1;
	_send_timeout = !res;
	startListenMode();
	if(res) last_activity = millis();

}
void BaySerialRF24::end(void) {
//	_radio->powerDown();
}
void BaySerialRF24::startListenMode(void) {
	_radio->flush_tx();
	_radio->startListening();
	read_pos = 1;
	length = 1;
}
void BaySerialRF24::stopListenMode(void) {
	_radio->stopListening();
	while (_radio->available()) { //This is old stuff - discard!
		_radio->flush_rx();
	}
	length = 0;
	read_pos = 0;

}

int BaySerialRF24::read(void) {
	if (!available())
		return -1;
	uint8_t b = buffer[read_pos];
	read_pos++;
	return b;
}

size_t BaySerialRF24::write(uint8_t c) {
	if (read_pos) {
		stopListenMode();
	}
	buffer[write_pos] = c;
	write_pos++;
	_send_timeout = false;
	if (write_pos >= _flush_size) {
		flush();
	}
	if (_send_timeout)
		return 0;
	return 1;
}


void BaySerialRF24::sendTestByte(uint8_t led) {
  uint8_t test_byte[] = {XOFF};
  uint8_t res = 0;
  if (! connected) _radio->powerUp();
  _radio->stopListening();
  res = _radio->write(test_byte, 1);
  uint8_t curr_pa = 0;
  while (!res && curr_pa < 4) {
    _radio->setPALevel((rf24_pa_dbm_e) curr_pa);
    delayMicroseconds(random(1000));
    res = _radio->write(test_byte, 1);
    curr_pa++;
  }
  if (res) {
    last_activity = millis();
    _radio->startListening();
    if(led) digitalWrite(led, 1);
    connected = 1;
  } else {
    if(led) digitalWrite(led, 0);
    _radio->powerDown();
    connected = 0;
  }
}
