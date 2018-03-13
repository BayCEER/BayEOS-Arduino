#include "BaySerialRF24.h"
BaySerialRF24::BaySerialRF24(RF24 &radio, int timeout) {
	BaySerialInterface::_timeout = timeout;
	_radio = &radio;
}

int BaySerialRF24::available(void) {
	if (write_pos)
		flush();
	if (length > read_pos)
		return length - read_pos;
	if (_radio->available(&pipe)) {
		length = _radio->getDynamicPayloadSize();
		_radio->read(buffer, length);
		read_pos = 0;
		return length;
	}
	return 0;
}

int BaySerialRF24::i_available(void) {
	return BaySerialRF24::available();
}

void BaySerialRF24::begin(long baud) {
}

void BaySerialRF24::begin(uint8_t ch, uint8_t* rx_adr, uint8_t* tx_adr,uint8_t flush_size) {
	_radio->begin();
	_radio->setChannel(ch);
	_radio->enableDynamicPayloads();
	_radio->setCRCLength(RF24_CRC_16);
	_radio->setDataRate(RF24_250KBPS);
	_radio->setPALevel(RF24_PA_HIGH);
	_radio->setRetries(15, 15);
	_radio->setAutoAck(true);
	_radio->openWritingPipe(tx_adr);
	_radio->openReadingPipe(1, rx_adr);
	_radio->startListening();
	_flush_size=flush_size;
}

void BaySerialRF24::flush(void) {
	_radio->stopListening();
	_radio->write(buffer, write_pos);
	_radio->startListening();
	write_pos = 0;
	length = 0;
	read_pos = 0;

}
void BaySerialRF24::end(void) {
//	_radio->powerDown();
}
int BaySerialRF24::read(void) {
	if (!available())
		return -1;
	uint8_t b = buffer[read_pos];
	read_pos++;
	return b;
}

size_t BaySerialRF24::write(uint8_t c) {
	if (length)
		length = 0;
	buffer[write_pos] = c;
	write_pos++;
	if (write_pos >= _flush_size) {
		flush();
	}
	return 1;
}

