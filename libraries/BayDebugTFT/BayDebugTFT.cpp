#include "BayDebugTFT.h"


BayTFTDebug::BayTFTDebug(UTFT *utft, char *tx_buffer, uint8_t rows, uint8_t cols):
		TFTStream(utft, tx_buffer, rows, cols){
	_modus=1;
}
