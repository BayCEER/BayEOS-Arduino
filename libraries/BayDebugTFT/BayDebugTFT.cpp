#include "BayDebugTFT.h"


BayTFTDebug::BayTFTDebug(UTFT *utft, char *tx_buffer, uint8_t rows, uint8_t cols):
		TFTStream(utft, tx_buffer, rows, cols){
	_modus=1;
}

void BayTFTDebug::flush(void){
	TFTStream::flush();
}

int BayTFTDebug::available(void){
	return 0;
}

int BayTFTDebug::peek(void){
	return 0;
}

int BayTFTDebug::read(void){
	return 0;
}

size_t BayTFTDebug::write(uint8_t b){
	return TFTStream::write(b);
}
