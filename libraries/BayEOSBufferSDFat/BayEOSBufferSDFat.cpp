#include "BayEOSBufferSDFat.h"
BayEOSBufferSDFat::BayEOSBufferSDFat(unsigned long max_length,uint8_t append,const char *p):BayEOSBuffer(){
	strncpy(_path,p,13);
	_path[13]=0;
    _f.open(_path, O_CREAT | O_RDWR | O_APPEND);
	 _max_length=max_length;
	 if(append) set(_f.fileSize());
     else reset();
}



void BayEOSBufferSDFat::resetStorage(void){
  _f.remove();
  _f.open(_path, O_CREAT | O_RDWR | O_TRUNC);
//  Serial.println("SD reset");
}

uint8_t BayEOSBufferSDFat::write(const uint8_t b){
	return _f.write(b);
}

uint8_t BayEOSBufferSDFat::write(const uint8_t *b,uint8_t length){
//	Serial.print("SD.write:");
//	Serial.println(_f.size());
	return _f.write(b,length);
}

uint8_t BayEOSBufferSDFat::seek(unsigned long pos){
	return _f.seekSet(pos);
}

int BayEOSBufferSDFat::read(void){
	return _f.read();
}

int BayEOSBufferSDFat::read(uint8_t *dest,int length){
	return _f.read(dest,length);
}

void BayEOSBufferSDFat::flush(void){
	_f.sync();
	//	Serial.println(_f.size());
}

SdFat SD=SdFat();// create usable instance
