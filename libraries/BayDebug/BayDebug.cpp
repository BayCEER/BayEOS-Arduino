#include "BayDebug.h"

void BayEOSDebugInterface::parseDataFrame(uint8_t offset) {
	if (getPayload(offset) != BayEOS_DataFrame) {
		println("No DF:");
		return;
	}
	offset++;
	uint8_t data_type = (getPayload(offset) & BayEOS_DATATYP_MASK);
	uint8_t channel_type = (getPayload(offset) & BayEOS_OFFSETTYP_MASK);
	uint8_t channel = 0;

	println("Data:");
	if (channel_type == 0x0) {
		offset++;
		channel = getPayload(offset);
	}
	offset++;

	while (offset < (getPacketLength() - _checksum)) {
		if (channel_type == BayEOS_ChannelLabel) {
			channel = getPayload(offset) + offset + 1; //this is actually the end of the channel label
			offset++;
			while (offset < (getPacketLength() - _checksum) && offset < channel) {
				print((char) getPayload(offset));
				offset++;
			}
		} else {
			if (channel_type == BayEOS_ChannelNumber) {

				channel = getPayload(offset);
				offset++;
			} else
				channel++;
			print("CH");
			print(channel);
		}
		print(":");
		switch (data_type) {
		case 0x1:
			print(*(float*) (_payload + offset));
			offset += 4;
			break;
		case 0x2:
			print(*(long*) (_payload + offset));
			offset += 4;
			break;
		case 0x3:
			print(*(int*) (_payload + offset));
			offset += 2;
			break;
		case 0x4:
			print(*(uint8_t*) (_payload + offset));
			offset++;
			break;
		default:
			print("Invalid data type");
			offset=(getPacketLength() - _checksum);
		}
		println();
	}
	return;
}

void BayEOSDebugInterface::parse(uint8_t offset) {
	uint16_t checksum;
	uint8_t current_offset;

//	print(" ");
	switch (getPayload(offset)) {
	case BayEOS_DataFrame:
		parseDataFrame(offset);
		break;
	case BayEOS_RoutedFrame:
		print("RF: MY:");
		print(*(uint16_t*) (_payload + offset + 1));
		print(" PAN:");
		println(*(uint16_t*) (_payload + offset + 3));
		parse(offset + 5);
		break;
	case BayEOS_RoutedFrameRSSI:
		print("RF: MY:");
		print(*(uint16_t*) (_payload + offset + 1));
		print(" PAN:");
		print(*(uint16_t*) (_payload + offset + 3));
		print(" RSSI:");
		println(*(uint8_t*) (_payload + offset + 5));
		parse(offset + 6);
		break;
	case BayEOS_OriginFrame:
	case BayEOS_RoutedOriginFrame:
		if (getPayload(offset) == BayEOS_RoutedOriginFrame)
			print('R');
		print("OF:");
		offset++;
		current_offset = getPayload(offset);
		offset++;
		while (current_offset > 0) {
			print((char) getPayload(offset));
			offset++;
			current_offset--;
		}
		println();
		parse(offset);
		break;
	case BayEOS_DelayedFrame:
	case BayEOS_DelayedSecondFrame:
		print("DF: Delay:");
		println(*(unsigned long*) (_payload + offset + 1));
		parse(offset + 5);
		break;
	case BayEOS_TimestampFrame:
		print("TF: TS:");
		println(*(unsigned long*) (_payload + offset + 1));
		parse(offset + 5);
		break;
	case BayEOS_ErrorMessage:
		print("EM: ");
		offset++;
		while (offset < getPacketLength() - _checksum) {
			print((char) getPayload(offset));
			offset++;
		}
		println();
		break;
	case BayEOS_Message:
		print("M: ");
		offset++;
		while (offset < getPacketLength() - _checksum) {
			print((char) getPayload(offset));
			offset++;
		}
		println();
		break;
	case BayEOS_ChecksumFrame:
		checksum = 0;
		current_offset = offset;
		while (current_offset < getPacketLength() - 2) {
			checksum += getPayload(current_offset);
			current_offset++;
		}
		checksum += *(uint16_t*) (_payload + current_offset);
		print("C:");
		if (checksum == 0xffff)
			print("OK");
		else
			print("ERR");
		println();
		offset++;
		_checksum = 2;
		parse(offset);
		break;
	default:
		print("U: ");
		print(getPayload(offset), HEX);
		print(" ");
		offset++;
		while (offset < getPacketLength() - _checksum) {
			print(getPayload(offset), HEX);
			print(":");
			offset++;
		}
		println();

	}

}

uint8_t BayEOSDebugInterface::sendPayload(void) {
	_checksum = 0;
	if (_modus & 0x1 == 0) {
		print("Payload: ");
		for (uint8_t i = 0; i < getPacketLength(); i++) {
			print(getPayload(i), HEX);
		}
	} else {
		parse();

	}
	if (_modus & 0x2)
		_error_next = !_error_next;
	print("TX: ");
	if(_error_next) println("failed");
	else println("ok");
	println();
	return (_error_next);
}

void BayDebug::begin(long baud, uint8_t modus) {
	_serial->begin(baud);
	_modus = modus;
}

BayDebug::BayDebug(HardwareSerial &serial) {
	_serial = &serial;
}
int BayDebug::available(void) {
	return _serial->available();
}
int BayDebug::read(void) {
	return _serial->read();
}
size_t BayDebug::write(uint8_t b) {
	return _serial->write(b);
}
int BayDebug::peek(void) {
	return _serial->peek();
}
;
void BayDebug::flush(void) {
	_serial->flush();
}
;

BayDebugCharbuffer::BayDebugCharbuffer(char* buffer, int size) {
	_buffer = buffer;
	_size = size - 1;
	_pos = 0;
}
uint8_t BayDebugCharbuffer::sendPayload(void) {
	uint8_t res=BayEOSDebugInterface::sendPayload();
	_pos=0;
	return res;
}

char* BayDebugCharbuffer::get(void) {
	return _buffer;
}


int BayDebugCharbuffer::available(void) {
	return 0;
}
int BayDebugCharbuffer::read(void) {
	return -1;
}
size_t BayDebugCharbuffer::write(uint8_t b) {
	if (_pos < _size) {
		_buffer[_pos] = b;
		_pos++;
		_buffer[_pos] = 0;
	}
	return 1;
}
int BayDebugCharbuffer::peek(void) {
	return -1;
}
void BayDebugCharbuffer::flush(void) {
}

