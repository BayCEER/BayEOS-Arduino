/*
 * Header file for scale calibration with temperature compensation
 *
 *
 */
#include <math.h>
#include <HX711Array.h>
#include <EEPROM.h>

#ifndef NTC_ADC_PIN
#define NTC_ADC_PIN A2
#endif

#ifndef NTC_POWER_PIN
#define NTC_POWER_PIN A1
#endif

#ifndef NTC_PRE_RESISTOR
#define NTC_PRE_RESISTOR 20000.0
#endif

#ifndef NTC10_FACTOR
#define NTC10_FACTOR 1
#endif

#ifndef LED_RED
#define LED_RED 5
#endif

#ifndef LED_GREEN
#define LED_GREEN 6
#endif

#ifndef ZERO_PIN
#define ZERO_PIN 3
#endif

#ifndef SCALE_PIN
#define SCALE_PIN 2
#endif

#ifndef MIN_DELTA_T
#define MIN_DELTA_T 1.0
#endif

#ifndef N_REG_POINTS
#define N_REG_POINTS 15
#endif

volatile uint8_t HX711_NTC_mode = 0;

void isr_int0(void) {
	HX711_NTC_mode = 1;
}

void isr_int1(void) {
	HX711_NTC_mode = 2;
}

class Regression {
public:
	float x[N_REG_POINTS + 1];
	long y[N_REG_POINTS + 1];
	float min;
	float max;
	float reg[4];
	uint8_t count,i;
	uint8_t* p;
	int offset; //EEPROM offset for storage
	long default_y;
	uint8_t order[N_REG_POINTS + 1]; //for sorting

	void init(int o,long d){
		offset=o;
		default_y=d;
		readEEPROM();
	}

	void calc(void) {
		if (count < 2)
			return;
		// initialize variables
		float xbar = 0;
		float ybar = 0;
		float covar = 0;
		float var_x = 0;
		float var_y = 0;
		min = 200;
		max = -200;

		// calculations required for linear regression
		for (i = 0; i < count; i++) {
			if (x[i] > max)
				max = x[i];
			if (x[i] < min)
				min = x[i];

			xbar += x[i];
			ybar += y[i];
		}
		xbar /= count;
		ybar /= count;
		float dx,dy;
		for(i = 0; i < count; i++){
			dx=x[i]-xbar;
			dy=y[i]-ybar;
			covar += dx*dy;
			var_x += dx*dx;
			var_y += dy*dy;
		}
		covar /= count;
		var_x /= count;
		var_y /= count;
		// simple linear regression algorithm
		reg[0] = covar/var_x;
		reg[1] = ybar - reg[0] * xbar;
		reg[2] = covar*covar/var_x/var_y;
		reg[3] = ybar;
/*		if ((max - min) < MIN_DELTA_T) {
			//user average
			reg[0] = 0;
			reg[1] = ybar;
		}*/
	}

	void updateEEPROM(void){
		p = (uint8_t*) x;
		for (i = 0; i < 4 * count; i++) {
			EEPROM.update(offset+i, *p);
			p++;
		}
		p = (uint8_t*) y;
		for (i = 0; i < 4 * count; i++) {
			EEPROM.update(offset+i + N_REG_POINTS * 4, *p);
			p++;
		}
	}

	void readEEPROM(void){
		p = (uint8_t*) x;
		for (i = 0; i < 4*N_REG_POINTS; i++) {
			*p = EEPROM.read(offset + i);
			p++;
		}
		p = (uint8_t*) y;
		for (i = 0; i < 4*N_REG_POINTS; i++) {
			*p = EEPROM.read(offset + i + N_REG_POINTS * 4);
			p++;
		}
		count=0;
		while (x[count] > -30 && x[count] < 60
				&& count < N_REG_POINTS) {
			count++;
		}
		calc();

	}

	void resetEERPOM(void){
		for (i = 0; i < N_REG_POINTS; i++) {
			x[i] = -1000;
		}
		updateEEPROM();
		count = 0;

	}

	long get_y(float xx){
		if (count == 0)
			return default_y;
		if (count == 1)
			return y[0];
		if (max-min<MIN_DELTA_T) return reg[3]; //average
		return reg[0] * xx + reg[1];
	}

	void add(float xx, long yy,uint8_t save=1) {
		x[count] = xx;
		y[count] = yy;
		if (count < N_REG_POINTS) {
			count++;
			if(save) updateEEPROM();
			calc();
			return;
		}
		//we have to overwrite a value - look for the best value
		//create a order array pointer
		uint8_t j, t;
		for (i = 0; i <= count; i++) {
			order[i] = i;
			j = i;
			while (j && x[order[j]] < x[order[j - 1]]) {
				t = order[j - 1];
				order[j - 1] = order[j];
				order[j] = t;
				j--;
			}
		}
		//search minimum difference
		float diff = max - min;
		for (i = 1; i <= count; i++) {
			if ((x[order[i]] - x[order[i - 1]]) < diff) {
				diff = x[order[i]] - x[order[i - 1]];
				j = i; //save i in j
			}
		}
		t=j-1; //index do take the average!
		//we have the minimum distance
		if ((j < count && j > 1
				&& (x[order[j-1]] - x[order[j - 2]])
						< (x[order[j + 1]] - x[order[j]]))
				|| j == count) {
			j--;
			t++;
		}
		//overwrite index t with average!
		x[order[t]] = (x[order[t]]+x[order[j]])/2;
		y[order[t]] = (y[order[t]]+y[order[j]])/2;
		//save new values in j
		x[order[j]] = xx;
		y[order[j]] = yy;
		if(save) updateEEPROM();
		calc();
	}

};


class HX711_NTC {
private:
	HX711Array* _scale;
	Regression z; //keeps values of zero regression
	Regression w; //keeps values of scale weight regression
	long offset; //keeps current offset
	float offset_temp; //keeps temperatur for current offset
	float slope_weight; //calibration weight
	uint8_t i;

public:
	uint8_t cal_mode; // 0: off, 1: zero, 2: weight
	long max_deviation;


	HX711_NTC(HX711Array &scale) {
		_scale = &scale;
	}

	float ntc10_R2T(float r) {
		float log_r = log(r);
		return 440.61073 - 75.69303 * log_r + 4.20199 * log_r * log_r
				- 0.09586 * log_r * log_r * log_r;
	}

	float readNTC(void){
		analogReference (DEFAULT);
		pinMode(NTC_POWER_PIN, OUTPUT);
		digitalWrite(NTC_POWER_PIN, HIGH);
		int adc = analogRead(NTC_ADC_PIN);
		digitalWrite(NTC_POWER_PIN, LOW);
		pinMode(NTC_POWER_PIN, INPUT);
		return ntc10_R2T(NTC10_FACTOR * NTC_PRE_RESISTOR * adc / (1023 - adc));
	}

	void init(long zz, long ww, float sw,uint8_t with_interrupts=1){
		if(with_interrupts){
			digitalWrite(ZERO_PIN, HIGH);
			digitalWrite(SCALE_PIN, HIGH);
			attachInterrupt(digitalPinToInterrupt(ZERO_PIN), isr_int0, FALLING);
			attachInterrupt(digitalPinToInterrupt(SCALE_PIN), isr_int1, FALLING);
		}
		pinMode(LED_RED, OUTPUT);
		pinMode(LED_GREEN, OUTPUT);

		z.init(0,zz);
		w.init(N_REG_POINTS*4*2,ww);
		slope_weight=sw;
		HX711_NTC_mode = 0;
		max_deviation=abs(ww-zz)/1000;
	}


	float get_slope(float temp){
		return (w.get_y(temp)-z.get_y(temp))/slope_weight;
	}

	uint8_t single_cal(float temp, uint8_t save){
		if(! cal_mode) return 3; //no calibration selected
	    digitalWrite(LED_GREEN, HIGH);
	    digitalWrite(LED_RED, HIGH);
	    delay(100);
	    digitalWrite(LED_GREEN, LOW);
	    digitalWrite(LED_RED, LOW);
		float slope = get_slope(temp);
		_scale->power_up();
		long readings[4];
		long min, max;
		long adc;
		max = -2000000000L;
		min = 2000000000L;
		for (i = 0; i < 4; i++) {
			readings[i] = _scale->read_average(10);
			if (readings[i] > max)
				max = readings[i];
			if (readings[i] < min)
				min = readings[i];
		}
		_scale->power_down();
		if ((max - min) > max_deviation) {
			for(i=0;i<2;i++){
			    digitalWrite(LED_RED, HIGH);
			    delay(300);
			    digitalWrite(LED_RED, LOW);
				delay(300);
			}
			return 2; //to much deviation
		}
		adc=0;
		for (i = 0; i < 4; i++) {
			adc+=readings[i];
		}
		adc /= 4;
		min=(cal_mode==2?z.get_y(temp):w.get_y(temp))-(slope<0?-1:1)*slope*slope_weight/20;
		max=(cal_mode==2?z.get_y(temp):w.get_y(temp))+(slope<0?-1:1)*slope*slope_weight/20;

		if(adc<min || adc>max){
		    digitalWrite(LED_RED, HIGH);
		    delay(300);
		    digitalWrite(LED_RED, LOW);
			return 1; //not in 5% range of expected reading!
		}

	    digitalWrite(LED_GREEN, HIGH);
	    delay(300);
	    digitalWrite(LED_GREEN, LOW);

		if(cal_mode==2) z.add(temp,adc,save);
		else w.add(temp,adc,save);
		return 0;
	}

	void handleINT(void){
		i=0;
		while(! digitalRead(ZERO_PIN) && ! digitalRead(SCALE_PIN)){
			i++;
			delay(50);
			if(i>100){
				reset();
		        for (i = 0; i < 5; i++) {
			          digitalWrite(LED_GREEN, HIGH);
			          digitalWrite(LED_RED, LOW);
		          delay(300);
		          digitalWrite(LED_RED, HIGH);
		          digitalWrite(LED_GREEN, LOW);
		          delay(300);
		        }
				digitalWrite(LED_RED,LOW);
				cal_mode=0;
			    HX711_NTC_mode = 0;
			    return;
			}
		}
	    if (! digitalRead(ZERO_PIN) && HX711_NTC_mode == 2) cal_mode = 2;
	    else if (! digitalRead(SCALE_PIN) && HX711_NTC_mode == 1) cal_mode = 1;
	    else {
	      if (cal_mode) {
	        save();
	        for (i = 0; i < 5; i++) {
	          digitalWrite(LED_GREEN, HIGH);
	          delay(300);
	          digitalWrite(LED_GREEN, LOW);
	          delay(300);
	        }

	      }
	      cal_mode = 0; //end calibration!
	    }
	    if (cal_mode) {
	      for (uint8_t i = 0; i < cal_mode*3; i++) {
	        digitalWrite(LED_RED, HIGH);
	        delay(300);
	        digitalWrite(LED_RED, LOW);
	        delay(300);
	      }

	    }
	    HX711_NTC_mode = 0;

	}

	void cal(float temp){
		float slope = get_slope(temp);
		//find current zero
		digitalWrite(LED_RED, HIGH);
		_scale->power_up();
		long readings[4];
		long min, max;
		long zero, weight;
		max = -2000000000L;
		min = 2000000000L;
		for (i = 0; i < 4; i++) {
			readings[i] = _scale->read_average(10);
			if (readings[i] > max)
				max = readings[i];
			if (readings[i] < min)
				min = readings[i];
		}
		if ((max - min) > max_deviation) {
			for (i = 0; i < 3; i++) {
				digitalWrite(LED_RED, LOW);
				delay(300);
				digitalWrite(LED_RED, HIGH);
				delay(300);
			}
			digitalWrite(LED_RED, LOW);
			HX711_NTC_mode = 0;
			_scale->power_down();
			return;
		}

		//Zero ok!
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_GREEN, HIGH);
		delay(300);
		digitalWrite(LED_GREEN, LOW);
		delay(300);
		zero=0;
		for(i=0;i<4;i++) zero+=readings[i];
		zero/=4;
		if (HX711_NTC_mode == 1) {
			offset=zero-z.get_y(temp);
			offset_temp=temp;
			HX711_NTC_mode = 0;
			_scale->power_down();
			return;
		}

		//wait for calibration weight
		digitalWrite(LED_RED, HIGH);
		unsigned long start = millis();
		max = -2000000000L;
		min = 2000000000L;
		uint8_t in_target = 0;

		while (abs(max - min) > max_deviation || !in_target) {
			max = -2000000000L;
			min = 2000000000L;
			for (i = 0; i < 4; i++) {
				digitalWrite(LED_RED, HIGH);
				readings[i] = _scale->read_average(20);
				digitalWrite(LED_RED, LOW);
				if (readings[i] > max)
					max = readings[i];
				if (readings[i] < min)
					min = readings[i];
				delay(300);
			}

			if ((millis() - start) > 15000) {
				delay(1000);
				for (i = 0; i < 3; i++) {
					digitalWrite(LED_RED, LOW);
					delay(300);
					digitalWrite(LED_RED, HIGH);
					delay(300);
				}
				digitalWrite(LED_RED, LOW);
				HX711_NTC_mode = 0;
				_scale->power_down();
				return;
			}

			if (slope > 0) {
				in_target = (min > slope_weight * (slope * 0.9) + zero
						&& max < slope_weight * (slope * 1.1) + zero);
			} else {
				in_target = (max < slope_weight * (slope * 0.9) + zero
						&& min > slope_weight * (slope * 1.1) + zero);
			}

		}
		//Slope ok
		_scale->power_down();
		digitalWrite(LED_RED, LOW);
		digitalWrite(LED_GREEN, HIGH);
		delay(300);
		digitalWrite(LED_GREEN, LOW);
		delay(300);
		weight = 0;
		for(i=0;i<4;i++) weight+=readings[i];
		weight/=4;

		w.add(temp,weight);
		z.add(temp,zero);
		HX711_NTC_mode = 0;
	}

	float adc2weight(long adc,float temp){
		return (adc-z.get_y(temp))/get_slope(temp)-
				(offset)/get_slope(offset_temp);

	}

	float read(float temp,uint8_t count = 20) {
		_scale->power_up();
		long adc = _scale->read_average(count);
		_scale->power_down();
		return adc2weight(adc,temp);
	}

	void reset(void){
		z.resetEERPOM();
		w.resetEERPOM();
	}
	void save(void){
		z.updateEEPROM();
		w.updateEEPROM();
	}

	void printCalData(void){
		Serial.print("Slope: ");
		Serial.println(get_slope(20.0));

		Serial.println("Offset:");
		Serial.print(offset_temp);
		Serial.print("\t");
		Serial.println(offset);
		Serial.println("EEPROM Weight:");
		for (i = 0; i < w.count; i++) {
			Serial.print(w.x[i]);
			Serial.print("\t");
			Serial.println(w.y[i]);
			delay(20);
		}
		Serial.println("Reg:");
		Serial.print(w.reg[0]);
		Serial.print("\t");
		Serial.print(w.reg[1]);
		Serial.print("\t");
		Serial.println(w.reg[2]);
		Serial.println("EEPROM Zero:");
		for (i = 0; i < z.count; i++) {
			Serial.print(z.x[i]);
			Serial.print("\t");
			Serial.println(z.y[i]);
			delay(20);
		}
		Serial.println("Reg:");
		Serial.print(z.reg[0]);
		Serial.print("\t");
		Serial.print(z.reg[1]);
		Serial.print("\t");
		Serial.println(z.reg[2]);
		Serial.println();
		delay(100);

	}

	void add(float tt,long zz, long ww){
		w.add(tt,ww);
		z.add(tt,zz);
	}

};
