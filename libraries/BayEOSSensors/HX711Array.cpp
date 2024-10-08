#include <Arduino.h>
#include <HX711Array.h>
#include <Sleep.h>
#include <math.h>
#include <EEPROM.h>

HX711Array::HX711Array() {
}

HX711Array::~HX711Array() {
}

void HX711Array::begin(uint8_t* dout, uint8_t length, uint8_t pd_sck,
		uint8_t gain) {
	_pd_sck = pd_sck;
	_length = length;
	if (_length > 4)
		_length = 4;
	_dout = dout;
	_p = (uint8_t*) _values;
	pinMode(_pd_sck, OUTPUT);
	for (uint8_t i = 0; i < _length; i++) {
		pinMode(_dout[i], INPUT_PULLUP);
	}
	set_gain(gain);
}

bool HX711Array::is_ready() {
	for (uint8_t i = 0; i < _length; i++) {
		if (digitalRead(_dout[i])){
			return 0;
		}
	}
	return 1;
}

void HX711Array::set_gain(uint8_t gain) {
	switch (gain) {
	case 128:		// channel A, gain factor 128
		GAIN = 1;
		break;
	case 64:		// channel A, gain factor 64
		GAIN = 3;
		break;
	case 32:		// channel B, gain factor 32
		GAIN = 2;
		break;
	}

	digitalWrite(_pd_sck, LOW);
	read();
}

void HX711Array::shiftIn(uint8_t nr) {
	uint8_t j;
	for (j = 0; j < _length; j++) {
		_p[nr + 4 * j] = 0;
	}
	for (uint8_t i = 0; i < 8; ++i) {
		noInterrupts();
		digitalWrite(_pd_sck, HIGH);
        delayMicroseconds(1);
		digitalWrite(_pd_sck, LOW);
		interrupts();
		for (j = 0; j < _length; j++) {
			_p[nr + 4 * j] |= digitalRead(_dout[j]) << (7 - i);
		}
       	delayMicroseconds(1);
	}
}

void HX711Array::set_no_sleep(bool ns){
	no_sleep=ns;
}

long HX711Array::read(uint8_t timeout) {
	// wait for the chip to become ready
	while (!is_ready()) {
		timeout--;
		if(! timeout) return 0xf0000000;
		if(! no_sleep) Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE); // sleep function called here
		else delay(5);
	}
	unsigned long value = 0;

	// pulse the clock pin 24 times to read the data
	shiftIn(2);
	shiftIn(1);
	shiftIn(0);

	// set the channel and the gain factor for the next reading using the clock pin
	for (uint8_t i = 0; i < GAIN; i++) {
		noInterrupts();
		digitalWrite(_pd_sck, HIGH);
        delayMicroseconds(1);
		digitalWrite(_pd_sck, LOW);
		interrupts();
        delayMicroseconds(1);
	}

	// Replicate the most significant bit to pad out a 32-bit signed integer
	for (uint8_t j = 0; j < _length; j++) {
		if (_p[2 + j * 4] & 0x80)
			_p[3 + j * 4] = 0xFF;
		else
			_p[3 + j * 4] = 0x00;
		value += _values[j];
	}

	return value;
}
const long* HX711Array::get_single_readings() {
	return _values;
}
long HX711Array::read_average(uint8_t times,uint8_t timeout) {
	long sum = 0;
	long value;
	for (uint8_t i = 0; i < times; i++) {
		value=read(timeout);
		if(value==0xf0000000) return value;
		sum += value;
	}
	return sum / times;
}

uint8_t HX711Array::read_average(long* res,uint8_t times,uint8_t timeout) {
	uint8_t i,j;
	for(i=0;i<_length;i++){
		res[i]=0;
	}
	for(j=0;j<times;j++){
		if(read(timeout)==0xf0000000) return 0;
		for(i=0;i<_length;i++){
			res[i]+=_values[i];
		}
	}
	for(i=0;i<_length;i++){
		res[i]/=times;
	}
	return 1;
}

void bubble_sort(long *a, int size) {
    for(uint8_t i=0; i<(size-1); i++) {
        for(uint8_t o=0; o<(size-(i+1)); o++) {
                if(a[o] > a[o+1]) {
                    long t = a[o];
                    a[o] = a[o+1];
                    a[o+1] = t;
                }
        }
    }
}

long HX711Array::read_average_with_filter(long* res, unsigned long max_deviation, uint8_t* counts,uint8_t times,uint8_t timeout) {
	uint8_t i,j;
	long current[4][5];
	uint8_t i_counts[4];
	if(! counts) counts=i_counts;
	read(timeout);
	for(j=0;j<5;j++){
		if(read(timeout)==0xf0000000) return 0xf0000000;
		for(i=0;i<_length;i++){
			current[i][j]=_values[i];
		}
	}


	for(i=0;i<_length;i++){
		res[i]=0;
		counts[i]=0;
		bubble_sort(current[i],5);
	}

	for(j=0;j<times;j++){
		if(read(timeout)==0xf0000000) return 0xf0000000;
		for(i=0;i<_length;i++){
			if(abs(_values[i]-current[i][2])<max_deviation){
				res[i]+=_values[i];
				counts[i]++;
			}
		}
	}
	long value=0;
	for(i=0;i<_length;i++){
		if(counts[i]<(times/2)) return 0xf0000000;
		res[i]/=counts[i];
		value+=res[i];
	}
	return value;
}


void HX711Array::power_down() {
	digitalWrite(_pd_sck, HIGH);
}

void HX711Array::power_up() {
	digitalWrite(_pd_sck, LOW);
}

Scale4PointCal::Scale4PointCal(int o){
	eeprom_offset=o;
}
void Scale4PointCal::setConfPoint(float t,uint8_t t_index,long adc, uint8_t adc_index){
	t_conf[t_index]=t;
	adc_conf[t_index+adc_index*2]=adc;
}

void Scale4PointCal::setConf(float w,float* t, long* a){
	scale_weight=w;
	memcpy(t_conf,t,sizeof(float[2]));
	memcpy(adc_conf,a,sizeof(long[4]));
}

void Scale4PointCal::getConf(float &w,float* t, long* a){
	w=scale_weight;
	memcpy(t,t_conf,sizeof(float[2]));
	memcpy(a,adc_conf,sizeof(long[4]));
}


void Scale4PointCal::setScaleWeight(float w){
	scale_weight=w;
}

void Scale4PointCal::setTare(long adc,float t){
	tare_weight=0;
	tare_weight=getWeight(adc,t);
}

void Scale4PointCal::saveConf(void){
	p = (uint8_t*) & scale_weight;
	for (i = 0; i < 4; i++) {
		EEPROM.update(eeprom_offset + i,*p);
		p++;
	}
	p = (uint8_t*) t_conf;
	for (i = 0; i < 8; i++) {
		EEPROM.update(eeprom_offset + i+4,*p);
		p++;
	}
	p = (uint8_t*) adc_conf;
	for (i = 0; i < 16; i++) {
		EEPROM.update(eeprom_offset + i+12,*p);
		p++;
	}

}
void Scale4PointCal::readConf(void){
	p = (uint8_t*) & scale_weight;
	for (i = 0; i < 4; i++) {
		*p = EEPROM.read(eeprom_offset + i);
		p++;
	}
	p = (uint8_t*) t_conf;
	for (i = 0; i < 8; i++) {
		*p = EEPROM.read(eeprom_offset + i+4);
		p++;
	}
	p = (uint8_t*) adc_conf;
	for (i = 0; i < 16; i++) {
		*p = EEPROM.read(eeprom_offset + i+12);
		p++;
	}
}
void Scale4PointCal::printConf(void){
	Serial.println("Temp\tZero\tWeight");
	for(i=0;i<2;i++){
		Serial.print(t_conf[i]);
		Serial.print("\t");
		Serial.print(adc_conf[i]);
		Serial.print("\t");
		Serial.println(adc_conf[i+2]);
	}
	Serial.print("Slope Weight: ");
	Serial.println(scale_weight);
}

float Scale4PointCal::getWeight(long adc, float t){
	long zero=adc_conf[0]+
			(adc_conf[1]-adc_conf[0])/
			(t_conf[1]-t_conf[0])*(t-t_conf[0]);
	long slope=adc_conf[2]+
			(adc_conf[3]-adc_conf[2])/
			(t_conf[1]-t_conf[0])*(t-t_conf[0])-zero;
	return scale_weight*(adc-zero)/slope-tare_weight;


}

long HX711_4PointCal::getRaw(void){
	return last_adc;
}


void HX711_4PointCal::tare(float t){
	readADC();
	setTare(last_adc,t);
}

void HX711_4PointCal::readADC(uint8_t c){
	power_up();
	last_adc=read_average(c);
	power_down();
}

ScaleTCal::ScaleTCal(int o){
	eeprom_offset=o;
}


void ScaleTCal::saveConf(float _slope,float* _m){
	uint8_t* p=(uint8_t*) _m;
	for(uint8_t i=0;i<16;i++){
		EEPROM.update(eeprom_offset + i,*p);
		p++;
	}
	p=(uint8_t*)&_slope;
	for(uint8_t i=16;i<20;i++){
		EEPROM.update(eeprom_offset + i,*p);
		p++;
	}
}
void ScaleTCal::readConf(void){
	uint8_t* p=(uint8_t*) m;
	for(uint8_t i=0;i<16;i++){
		*p=EEPROM.read(eeprom_offset+i);
		p++;
	}
	p=(uint8_t*)&slope;
	for(uint8_t i=16;i<20;i++){
		*p=EEPROM.read(eeprom_offset+i);
		p++;
	}
}

float ScaleTCal::getWeight(long adc,float t,float dt){
	return slope*(adc-m[0]-m[1]*t-m[2]*dt-m[3]*dt*dt);
}
