#include "TFTStream.h"

TFTStream::TFTStream(UTFT *utft, char *tx_buffer, uint8_t rows, uint8_t cols){
	_utft=utft;
	_tx_buffer=tx_buffer;
	_rows=rows;
	_cols=cols;
	_first_row=1;
/*	for(int i=0;i<((int)_rows*(_cols+1));i++){
		_tx_buffer[i]=0;
	}
	*/
}

void TFTStream::begin(void){
	  _utft->InitLCD(PORTRAIT);
	  _utft->setFont(SmallFont);
	  _utft->clrScr();
	  _on=1;
}

void TFTStream::end(void){
  _on=0;
}


size_t TFTStream::write(uint8_t c){
	if(c==13) return 1;
	if(c==10){
		do{
			write(' ');
		} while(_ccol);
		return 1;
	}
	_tx_buffer[(int)_ccol+(_crow*(1+_cols))]=c;
	_ccol++;
	if(_ccol==_cols){
//		_tx_buffer[(int)_ccol+(_crow*(1+_cols))]=0;
		_ccol=0;
		_crow++;
		_first_row++;
		if(_first_row==_rows) _first_row=0;
		if(_crow==_rows){
			_crow=0;
		}
		for(uint8_t i=0; i<_cols;i++){
		  _tx_buffer[(int) i+(_crow*(1+_cols))]=' ';
		}
	}
	return 1;
}


void TFTStream::flush(void){
	if(! _on) return;
	int offset;
	for(uint8_t i=0;i<_rows;i++){
		offset=((i+_first_row) % _rows)*(_cols+1);
		_utft->print(_tx_buffer+offset,0,12*i);
	}

}
