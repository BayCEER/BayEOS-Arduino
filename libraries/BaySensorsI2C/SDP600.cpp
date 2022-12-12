#include "SDP600.h"

void SDP600::begin(int scaleFactor){
	Wire.begin();
	_scaleFactor=scaleFactor;
}

float SDP600::read(void){
    Wire.beginTransmission((uint8_t) SDP600_I2C_ADDR);
    Wire.write(0xF1); //trigger measurement
    Wire.endTransmission();
    Wire.requestFrom((uint8_t) SDP600_I2C_ADDR, (byte)3);
    uint8_t _crc = 0;// Initialize CRC calculation
    uint8_t msb = Wire.read();
    calcCRC(msb,&_crc);
    uint8_t lsb = Wire.read();
    calcCRC(lsb,&_crc);
    uint8_t crc = Wire.read();
    Wire.endTransmission();
    if(_crc!=crc) return NAN;
    else {
    	int val=(msb<<8)|lsb;
    	return 1.0/_scaleFactor*val;
    }

}

void SDP600::reset(void){
    Wire.beginTransmission((uint8_t) SDP600_I2C_ADDR);
    Wire.write(0xFE); //reset
    Wire.endTransmission();
}

void SDP600::setResolution(uint8_t res){
	uint8_t msb=B01110001; // bit 2-4 define resolution
	uint8_t lsb=B10000010;
	if(res<9 || res>16) res=12;
	res-=9;
	msb|=(res<<1); // set bits 2-4 for selected resolution
    Wire.beginTransmission((uint8_t) SDP600_I2C_ADDR);
    Wire.write((uint8_t) 0xE4);
    Wire.write(msb);
    Wire.write(lsb);
    Wire.endTransmission();
}

void SDP600::calcCRC(uint8_t value, uint8_t *crc)
{
  const uint8_t POLY = 0x31;   // Polynomial: x**8 + x**5 + x**4 + 1
  int8_t i;
  *crc ^= value;
  for (i = 8; i > 0; i--)
  {
    if (*crc & 0x80)
      *crc = (*crc << 1) ^ POLY;
    else
      *crc = (*crc << 1);
  }
}


void SDP800::begin(){
	Wire.begin();
    delay(1);
    Wire.beginTransmission((uint8_t) SDP800_I2C_ADDR);
    Wire.write((uint8_t) 0x36);
    Wire.write((uint8_t) 0x15); //Differential prssure average till read
    Wire.endTransmission();
}

void SDP800::scaleFactor(void){
    Wire.requestFrom((uint8_t) SDP800_I2C_ADDR, (byte)9);

    for(uint8_t i=0;i<6;i++) Wire.read(); //skip bytes to get scale factor

    uint8_t _crc = 0;// Initialize CRC calculation
    uint8_t msb = Wire.read();
    calcCRC(msb,&_crc);
    uint8_t lsb = Wire.read();
    calcCRC(lsb,&_crc);
    uint8_t crc = Wire.read();
    Wire.endTransmission();
	_scaleFactor=(msb<<8)|lsb;

}

float SDP800::read(void){
    Wire.requestFrom((uint8_t) SDP800_I2C_ADDR, (byte)3);
    uint8_t _crc = 0xff;// Initialize CRC calculation
    uint8_t msb = Wire.read();
    calcCRC(msb,&_crc);
    uint8_t lsb = Wire.read();
    calcCRC(lsb,&_crc);
    uint8_t crc = Wire.read();
    Wire.endTransmission();
    if(_crc!=crc ) return NAN;
    else {
    	int val=(msb<<8)|lsb;
    	return 1.0/_scaleFactor*val;
    }
}

void SDP800::calcCRC(uint8_t value, uint8_t *crc)
{
  const uint8_t POLY = 0x31;   // Polynomial: x**8 + x**5 + x**4 + 1
  int8_t i;
  *crc ^= value;
  for (i = 8; i > 0; i--)
  {
    if (*crc & 0x80)
      *crc = (*crc << 1) ^ POLY;
    else
      *crc = (*crc << 1);
  }
}
