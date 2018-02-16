#include "NTC.h"
#include <Sleep.h>
#include <math.h>


float NTC_Sensor::getTemp(void){
	float log_r = log(readResistance() / ntc_type * 10);
	return 440.61073 - 75.69303 * log_r + 4.20199 * log_r * log_r
			- 0.09586 * log_r * log_r * log_r;

}


NTC_ADC::NTC_ADC(uint8_t pp,uint8_t ap,float pr,float nt){
	ntc_type=nt;
	power_pin=pp;
	adc_pin=ap;
	pre_resistor=pr;
}

float NTC_ADC::readResistance(void){
	pinMode(power_pin,OUTPUT);
	analogReference(DEFAULT);
	digitalWrite(power_pin,HIGH);
	int adc=analogRead(adc_pin);
	digitalWrite(power_pin,LOW);
	pinMode(power_pin,INPUT);
	return pre_resistor * adc / (1023-adc);

}
NTC_MCP342x::NTC_MCP342x(MCP342x &m, uint8_t pp, uint8_t ch_r, uint8_t ch_n, float pr,float nt){
	 power_pin=pp;
	 mcp=&m;
	 pre_resistor=pr;
	 ntc_type=nt;
	 adc_ch_r=ch_r;
	 adc_ch_ntc=ch_n;
}

float NTC_MCP342x::readResistance(void){
	float U;
	uint8_t i;
	pinMode(power_pin,OUTPUT);
	digitalWrite(power_pin,HIGH);
	delay(1);
	mcp->setConf(0, 1, adc_ch_r, 0, 2, 0);
	if(sleep_count){
	  for(i=0;i<sleep_count;i++){
		 Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE); // sleep function called here
	  }
	} else delay(100);
	U=mcp->getData(0);
	mcp->setConf(0, 1, adc_ch_ntc, 0, 2, 0);
	if(sleep_count){
	  for(i=0;i<sleep_count;i++){
		 Sleep.sleep(TIMER2_ON, SLEEP_MODE_PWR_SAVE); // sleep function called here
	  }
	} else delay(100);

	digitalWrite(power_pin,LOW);
	pinMode(power_pin,INPUT);
	return mcp->getData(0)/U*pre_resistor;
}

NTC_HX711::NTC_HX711(HX711Array &hx, uint8_t pp, float pr, float nt, uint8_t n){
	 power_pin=pp;
	 hx711=&hx;
	 pre_resistor=pr;
	 ntc_type=nt;
	 number=n;
}

float NTC_HX711::readResistance(void){
	pinMode(power_pin,OUTPUT);
	digitalWrite(power_pin,HIGH);
	delay(1);
	hx711->power_up();
	hx711->set_gain(32);
	hx711->read();
	float adc=hx711->get_single_readings()[number];
	adc=adc/32/256/256/256;
	hx711->set_gain(128);
	hx711->power_down();
	digitalWrite(power_pin,LOW);
	pinMode(power_pin,INPUT);
	return adc/(1-adc)*pre_resistor;
}

