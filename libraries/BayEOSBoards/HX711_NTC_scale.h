/*
 * Header file for scale calibration with temperature compensation
 *
 *
 */

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

#ifndef SLOPE_WEIGHT
#define SLOPE_WEIGHT 1467.0
#endif

#ifndef SLOPE_VALUE
#define SLOPE_VALUE 210.0
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
#define MIN_DELTA_T 2.0
#endif

#ifndef N_REG_POINTS
#define N_REG_POINTS 10
#endif

volatile uint8_t mode = 0;

void isr_int0(void) {
	mode = 1;
}

void isr_int1(void) {
	mode = 2;
}

long offset; //keeps current offset
float offset_temp; //keeps temperatur for current offset
float t_values[N_REG_POINTS + 1];
long w_values[N_REG_POINTS + 1]; //keeps values with scale weight
long z_values[N_REG_POINTS + 1]; //keeps readings with zero weight
float w_reg[3]; //regression line for s {slope,intercept,r²}
float z_reg[3]; //regression line for z {slope,intercept,r²}
uint8_t s_count, i;
float t_min = 200;
float t_max = -200;

#include <math.h>
float ntc10_R2T(float r) {
	float log_r = log(r);
	return 440.61073 - 75.69303 * log_r + 4.20199 * log_r * log_r
			- 0.09586 * log_r * log_r * log_r;
}

void HX711_NTC_calc_reg(void) {
	if (s_count < 2)
		return;

	// initialize variables
	float xbar = 0;
	float wbar = 0;
	float zbar = 0;
	float covar_w = 0;
	float covar_z = 0;
	float var_x = 0;
	float var_w = 0;
	float var_z = 0;
	t_min = 200;
	t_max = -200;

	// calculations required for linear regression
	for (i = 0; i < s_count; i++) {
		if (t_values[i] > t_max)
			t_max = t_values[i];
		if (t_values[i] < t_min)
			t_min = t_values[i];

		xbar += t_values[i];
		wbar += w_values[i];
		zbar += z_values[i];
	}
	xbar /= s_count;
	wbar /= s_count;
	zbar /= s_count;
	float dx,dw,dz;
	for(i = 0; i < s_count; i++){
		dx=t_values[i]-xbar;
		dw=w_values[i]-wbar;
		dz=z_values[i]-zbar;
		covar_w += dx*dw;
		covar_z += dx*dz;
		var_x += dx*dx;
		var_w += dw*dw;
		var_z += dz*dz;

	}
	covar_w /= s_count;
	covar_z /= s_count;
	var_x /= s_count;
	var_w /= s_count;
	var_z /= s_count;
	// simple linear regression algorithm
	w_reg[0] = covar_w/var_x;
	w_reg[1] = wbar - w_reg[0] * xbar;
	w_reg[2] = covar_w*covar_w/var_x/var_w;
	z_reg[0] = covar_z/var_x;
	z_reg[1] = zbar - z_reg[0] * xbar;
	z_reg[2] = covar_z*covar_z/var_x/var_z;
	if ((t_max - t_min) < MIN_DELTA_T) {
		//user average
		w_reg[0] = 0;
		w_reg[1] = wbar;
		z_reg[0] = 0;
		z_reg[1] = zbar;
	}
}

void HX711_NTC_init(void) {
	scale.begin(dout, 1, sck); //start HX711Array with 1 ADCs

	digitalWrite(ZERO_PIN, HIGH);
	digitalWrite(SCALE_PIN, HIGH);
	attachInterrupt(digitalPinToInterrupt(ZERO_PIN), isr_int0, FALLING);
	attachInterrupt(digitalPinToInterrupt(SCALE_PIN), isr_int1, FALLING);
	pinMode(LED_RED, OUTPUT);
	pinMode(LED_GREEN, OUTPUT);

	uint8_t* p;
	p = (uint8_t*) t_values;
	for (i = 0; i < 4*N_REG_POINTS; i++) {
		*p = EEPROM.read(i);
		p++;
	}
	p = (uint8_t*) w_values;
	for (i = 0; i < 4*N_REG_POINTS; i++) {
		*p = EEPROM.read(i + N_REG_POINTS * 4);
		p++;
	}
	p = (uint8_t*) z_values;
	for (i = 0; i < 4*N_REG_POINTS; i++) {
		*p = EEPROM.read(i + N_REG_POINTS * 4*2);
		p++;
	}
	s_count=0;
	while (t_values[s_count] > -30 && t_values[s_count] < 60
			&& s_count < N_REG_POINTS) {
		s_count++;
	}
	HX711_NTC_calc_reg();
	mode = 0;

}

void HX711_NTC_update_EEPROM(void) {
	uint8_t* p;
	p = (uint8_t*) t_values;
	for (i = 0; i < 4 * s_count; i++) {
		EEPROM.update(i, *p);
		p++;
	}
	p = (uint8_t*) w_values;
	for (i = 0; i < 4 * s_count; i++) {
		EEPROM.update(i + N_REG_POINTS * 4, *p);
		p++;
	}
	p = (uint8_t*) z_values;
	for (i = 0; i < 4 * s_count; i++) {
		EEPROM.update(i + N_REG_POINTS * 4*2, *p);
		p++;
	}
}

void HX711_NTC_clear_EEPROM(void) {
	for (i = 0; i < N_REG_POINTS; i++) {
		t_values[i] = -1000;
	}
	HX711_NTC_update_EEPROM();
	s_count = 0;
}

float HX711_NTC_read_NTC(void) {
	analogReference (DEFAULT);
	pinMode(NTC_POWER_PIN, OUTPUT);
	digitalWrite(NTC_POWER_PIN, HIGH);
	int adc = analogRead(NTC_ADC_PIN);
	digitalWrite(NTC_POWER_PIN, LOW);
	pinMode(NTC_POWER_PIN, INPUT);
	return ntc10_R2T(NTC10_FACTOR * NTC_PRE_RESISTOR * adc / (1023 - adc));
}

long HX711_NTC_get_zero(float temp){
	if (s_count == 0)
		return 0;
	if (s_count == 1)
		return z_values[0];
	return z_reg[0] * temp + z_reg[1];
}

float HX711_NTC_get_slope(float temp) {
	if (s_count == 0)
		return SLOPE_VALUE;
	if (s_count == 1)
		return (w_values[0]-z_values[0])/SLOPE_WEIGHT;
	return ((w_reg[0] * temp + w_reg[1])-(z_reg[0] * temp + z_reg[1]))/SLOPE_WEIGHT;
}

void HX711_NTC_add_cal_value(float temp, long zero, long weight) {
	t_values[s_count] = temp;
	w_values[s_count] = weight;
	z_values[s_count] = zero;
	if (s_count < N_REG_POINTS) {
		s_count++;
		return;
	}
	//we have to overwrite a value - look for the best value
	//create a order array pointer
	uint8_t order[N_REG_POINTS + 1];
	uint8_t j, t;
	for (i = 0; i <= s_count; i++) {
		order[i] = i;
		j = i;
		while (j && t_values[order[j]] < t_values[order[j - 1]]) {
			t = order[j - 1];
			order[j - 1] = order[j];
			order[j] = t;
			j--;
		}
	}
	//search minimum difference
	float diff = t_max - t_min;
	for (i = 1; i <= s_count; i++) {
		if ((t_values[order[i]] - t_values[order[i - 1]]) < diff) {
			diff = t_values[order[i]] - t_values[order[i - 1]];
			j = i; //save i in j
		}
	}
	//we have the minimum distance
	if ((j < s_count && j > 1
			&& (t_values[order[j]] - t_values[order[j - 1]])
					< (t_values[order[j + 1]] - t_values[order[j]]))
			|| j == s_count) {
		j--;
	}
	t_values[order[j]] = temp;
	w_values[order[j]] = weight;
	z_values[order[j]] = zero;

}

void HX711_NTC_cal(void) {
	float temp=HX711_NTC_read_NTC();
	float slope = HX711_NTC_get_slope(temp);
	//find current zero
	digitalWrite(LED_RED, HIGH);
	scale.power_up();
	long readings[4];
	long min, max;
	long zero, weight;
	max = -2000000000L;
	min = 2000000000L;
	for (i = 0; i < 4; i++) {
		readings[i] = scale.read_average(10);
		if (readings[i] > max)
			max = readings[i];
		if (readings[i] < min)
			min = readings[i];
	}
	if ((max - min) > abs(slope * SLOPE_WEIGHT/1000)) {
		Serial.println(min);
		Serial.println(max);
		for (i = 0; i < 3; i++) {
			digitalWrite(LED_RED, LOW);
			delay(300);
			digitalWrite(LED_RED, HIGH);
			delay(300);
		}
		digitalWrite(LED_RED, LOW);
		mode = 0;
		scale.power_down();
		return;
	}

	//Zero ok!
	digitalWrite(LED_RED, LOW);
	digitalWrite(LED_GREEN, HIGH);
	delay(300);
	digitalWrite(LED_GREEN, LOW);
	delay(300);
	zero = (max + min) / 2;
	if (mode == 1) {
		offset=zero-HX711_NTC_get_zero(temp);
		offset_temp=temp;
		mode = 0;
		scale.power_down();
		return;
	}

	//wait for calibration weight
	digitalWrite(LED_RED, HIGH);
	unsigned long start = millis();
	max = -2000000000L;
	min = 2000000000L;
	uint8_t in_target = 0;

	while (abs(max - min) > abs(slope * SLOPE_WEIGHT/500) || !in_target) {
		max = -2000000000L;
		min = 2000000000L;
		for (i = 0; i < 4; i++) {
			digitalWrite(LED_RED, HIGH);
			readings[i] = scale.read_average(20);
			digitalWrite(LED_RED, LOW);
			if (readings[i] > max)
				max = readings[i];
			if (readings[i] < min)
				min = readings[i];
			delay(300);
		}

		if ((millis() - start) > 30000) {
			delay(1000);
			for (i = 0; i < 3; i++) {
				digitalWrite(LED_RED, LOW);
				delay(300);
				digitalWrite(LED_RED, HIGH);
				delay(300);
			}
			digitalWrite(LED_RED, LOW);
			mode = 0;
			scale.power_down();
			return;
		}

		if (slope > 0) {
			in_target = (min > SLOPE_WEIGHT * (slope * 0.9) + zero
					&& max < SLOPE_WEIGHT * (slope * 1.1) + zero);
		} else {
			in_target = (max < SLOPE_WEIGHT * (slope * 0.9) + zero
					&& min > SLOPE_WEIGHT * (slope * 1.1) + zero);
		}

	}
	//Slope ok
	scale.power_down();
	digitalWrite(LED_RED, LOW);
	digitalWrite(LED_GREEN, HIGH);
	delay(300);
	digitalWrite(LED_GREEN, LOW);
	delay(300);
	weight = (max + min) / 2;

	HX711_NTC_add_cal_value(temp,zero,weight);
	HX711_NTC_update_EEPROM();
	HX711_NTC_calc_reg();

	mode = 0;
}

float HX711_NTC_read(uint8_t count = 20) {
	float temp=HX711_NTC_read_NTC();
	float slope = HX711_NTC_get_slope(temp);
	scale.power_up();
	long adc = scale.read_average(count);
	scale.power_down();
	return (adc-HX711_NTC_get_zero(temp))/slope-
			(offset)/HX711_NTC_get_slope(offset_temp);
}

void HX711_NTC_print_debug(void) {
	Serial.println("Offset:");
	Serial.print(offset_temp);
	Serial.print("\t");
	Serial.println(offset);
	Serial.println("EEPROM:");
	for (i = 0; i < s_count; i++) {
		Serial.print(t_values[i]);
		Serial.print("\t");
		Serial.print(w_values[i]);
		Serial.print("\t");
		Serial.println(z_values[i]);
		delay(20);
	}
	Serial.println("Reg:");
	Serial.print(w_reg[0]);
	Serial.print("\t");
	Serial.print(w_reg[1]);
	Serial.print("\t");
	Serial.println(w_reg[2]);
	Serial.print(z_reg[0]);
	Serial.print("\t");
	Serial.print(z_reg[1]);
	Serial.print("\t");
	Serial.println(z_reg[2]);
	Serial.println();
	delay(100);
}
