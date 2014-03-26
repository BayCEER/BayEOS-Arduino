#include "BayEOSBufferSD.h"
BayEOSBufferSD::BayEOSBufferSD(unsigned long max_length,uint8_t append,const char *f):BayEOSBuffer(){
	strncpy(_filename,f,12);
	_filename[12]=0;
#if SERIAL_DEBUG
	Serial.println(_filename);
	 Serial.println("open file:");
	 delay(100);
#endif
    _f=SD.open(_filename, O_CREAT | O_RDWR | O_APPEND);
	 _max_length=max_length;
#if SERIAL_DEBUG
	 Serial.println(_f.size());
#endif
	 if(append) set(_f.size());
     else reset();
}



void BayEOSBufferSD::resetStorage(void){
  _f.close();
  SD.remove(_filename);
  _f=SD.open(_filename,O_CREAT | O_RDWR | O_TRUNC);
//  Serial.println("SD reset");
}

uint8_t BayEOSBufferSD::write(const uint8_t b){
	return _f.write(b);
}

uint8_t BayEOSBufferSD::write(const uint8_t *b,uint8_t length){
//	Serial.print("SD.write:");
//	Serial.println(_f.size());
	return _f.write(b,length);
}

uint8_t BayEOSBufferSD::seek(unsigned long pos){
	return _f.seek(pos);
}

int BayEOSBufferSD::read(void){
	return _f.read();
}

int BayEOSBufferSD::read(uint8_t *dest,int length){
	return _f.read(dest,length);
}

void BayEOSBufferSD::flush(void){
	_f.flush();
	//	Serial.println(_f.size());
}
