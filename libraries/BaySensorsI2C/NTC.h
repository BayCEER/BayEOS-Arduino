#ifndef NTC_h
#define NTC_h
#include <Arduino.h>

//Define a abstract class NTC_Sensor
//this class handles the R2T transformation
//readResistance has to be implemented
class NTC_Sensor {
protected:
	float ntc_type; //e.g. 3.0 for 3k or 5.0 for 5k....
public:
	NTC_Sensor(float nt);
	float R2T(float t);
	virtual float readResistance(void)=0;
	float getTemp(void);
};

class NTC_Calc: public NTC_Sensor {
public:
	NTC_Calc(float nt):NTC_Sensor(nt){}
	float readResistance(void);
};


//NTC sensor directly connnected to Arduino ADC
class NTC_ADC: public NTC_Sensor {
private:
	uint8_t power_pin;
	uint8_t adc_pin;
	float pre_resistor;
public:
	NTC_ADC(uint8_t pp, uint8_t ap, float pr, float nt);
	float readResistance(void);
};

//NTC sensor read out with MCP342x 18bit ADC
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

//NTC sensor read out with HX711 24bit ADC Channel B
#include <HX711Array.h>
class NTC_HX711: public NTC_Sensor {
private:

	uint8_t power_pin;
	float pre_resistor;
	HX711Array* hx711;
	uint8_t number;

public:
	NTC_HX711(HX711Array &hx, uint8_t pp, float pr, float nt, uint8_t n=0 );
	NTC_HX711(HX711Array &hx, float pr, float nt, uint8_t n=0 );
	float readResistance(void);
	//Note: getTemp(nr) does not run a ADC conversion
	//call readResistance before!
	float getTemp(uint8_t nr);

};



#endif
