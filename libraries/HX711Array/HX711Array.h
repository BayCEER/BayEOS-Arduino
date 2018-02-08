#ifndef HX711Array_h
#define HX711Array_h

#include "Arduino.h"

class HX711Array {
private:
	uint8_t _pd_sck;	// Power Down and Serial Clock Input Pin
	uint8_t* _dout;		// Serial Data Output Pins
	uint8_t _length;
	uint8_t GAIN;		// amplification factor
	long _values[4];
	uint8_t* _p;
	void shiftIn(uint8_t nr);

public:
	HX711Array();
	virtual ~HX711Array();
	// define clock and data pins channel, and gain factor
	// channel selection is made by passing the appropriate gain: 128 or 64 for channel A, 32 for channel B
	// gain: 128 or 64 for channel A; channel B works with 32 gain factor only
	void begin(uint8_t* dout, uint8_t length, uint8_t pd_sck,
			uint8_t gain = 128);
	// check if HX711Array is ready
	// from the datasheet: When output data is not ready for retrieval, digital output pin _dout is high. Serial clock
	// input _pd_sck should be low. When _dout goes to low, it indicates data is ready for retrieval.
	bool is_ready();

	// set the gain factor; takes effect only after a call to read()
	// channel A can be set for a 128 or 64 gain; channel B has a fixed 32 gain
	// depending on the parameter, the channel is also set to either A or B
	void set_gain(uint8_t gain = 128);

	// waits for the chip to be ready and returns a reading
	long read(uint8_t timeout = 128);

	// returns pointer to last single reads
	const long* get_single_readings();

	// returns an average reading; times = how many times to read
	long read_average(uint8_t times = 10, uint8_t timeout = 128);

	// fills res array with average reading; times = how many times to read
	uint8_t read_average(long* res, uint8_t times = 10, uint8_t timeout = 128);

	// puts the chip into power down mode
	void power_down();

	// wakes up the chip after power down mode
	void power_up();
};


class HX711_4PointCal: public HX711Array {
private:
	int eeprom_offset;
	float t_conf[2];
	long adc_conf[4];
	long last_adc;
	float scale_weight, tare_weight;
	uint8_t i;
	uint8_t* p;
public:
	HX711_4PointCal(int o = 0);
	void setConfPoint(float t, uint8_t t_index, long adc, uint8_t adc_index);
	void setConf(float w, float* t, long* a);
	void setScaleWeight(float w);
	void saveConf(void);
	void readConf(void);
	void printConf(void);

	void readADC(uint8_t c = 20);
	long getRaw(void);
	float getWeight(float t);
	void tare(float t);
};

class NTC_Sensor {
protected:
	float ntc_type; //e.g. 3.0 for 3k or 5.0 for 5k....
public:
	virtual float readResistance(void)=0;
	float getTemp(void);
};

class NTC_ADC: public NTC_Sensor {
private:
	uint8_t power_pin;
	uint8_t adc_pin;
	float pre_resistor;
public:
	NTC_ADC(uint8_t pp, uint8_t ap, float pr, float nt);
	float readResistance(void);
};

#include <MCP342x.h>
class NTC_MCP342x: public NTC_Sensor {
private:
	uint8_t power_pin;
	uint8_t adc_ch_r;
	uint8_t adc_ch_ntc;
	float pre_resistor;
	MCP342x* mcp;

public:
	NTC_MCP342x(MCP342x &m, uint8_t pp, uint8_t ch_r, uint8_t ch_n, float pr, float nt);
	float readResistance(void);
	uint8_t sleep_count;

};


#endif /* HX711Array_h */
