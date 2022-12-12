/****************************************************************

  Serial CSV Sketch

  Connection

  A) ELECTRODES
       |--R_PRE1--A0---E1+ E1- -|
  D6 --|--R_PRE2--A1---E2+ E2- -|-- D4
       |--R_PRE3--A2---E3+ E3- -|

  B) NTC05
       |--R_NRC1--A3--  NTC1 --|
  D3 --|--R_NTC2--A4--  NTC2 --|-- GND
       |--R_NTC3--A5--  NTC3 --|

***************************************************************/

#define R_NTC 10000.0
#define R_PRE 100000.0
#define ADC_COUNT 20
#define WAIT_TIME_MICROS_POWERUP 500
#define WAIT_TIME_MICROS_BETWEEN 1000
#define MILLIS_DELAY_BETWEEN 5000 /*delay in ms between measurements*/

char channel_map[] = "temp1;temp2;temp3;R1+;R2+;R3+;R1-;R2-;R3-";
char unit_map[] = "C;C;C;Ohm;Ohm;Ohm;Ohm;Ohm;Ohm";

#include <NTC.h>

NTC_ADC ntc1(3, A3, R_NTC, 5.0);
NTC_ADC ntc2(3, A4, R_NTC, 5.0);
NTC_ADC ntc3(3, A5, R_NTC, 5.0);







void measure() {
  analogReference(DEFAULT);

  Serial.print(ntc1.getTemp());
  Serial.print(";");
  Serial.print(ntc2.getTemp());
  Serial.print(";");
  Serial.print(ntc3.getTemp());
  Serial.print(";");

  uint16_t adc1_p = 0;
  uint16_t adc2_p = 0;
  uint16_t adc3_p = 0;
  uint16_t adc1_n = 0;
  uint16_t adc2_n = 0;
  uint16_t adc3_n = 0;

  for (int i = 0; i < ADC_COUNT; i++) {
    digitalWrite(6, HIGH);
#if WAIT_TIME_MICROS_POWERUP
    delayMicroseconds(WAIT_TIME_MICROS_POWERUP);
#endif
    adc1_p += analogRead(A0);
    adc2_p += analogRead(A1);
    adc3_p += analogRead(A2);
    digitalWrite(6, LOW);
#if WAIT_TIME_MICROS_BETWEEN
    delayMicroseconds(WAIT_TIME_MICROS_BETWEEN);
#endif
    digitalWrite(4, HIGH);
#if WAIT_TIME_MICROS_POWERUP
    delayMicroseconds(WAIT_TIME_MICROS_POWERUP);
#endif
    adc1_n += analogRead(A0);
    adc2_n += analogRead(A1);
    adc3_n += analogRead(A2);
    digitalWrite(4, LOW);
#if WAIT_TIME_MICROS_BETWEEN
    delayMicroseconds(WAIT_TIME_MICROS_BETWEEN);
#endif
  }
  Serial.print( R_PRE / (1023.0 * ADC_COUNT / adc1_p - 1) );
  Serial.print(";");
  Serial.print( R_PRE / (1023.0 * ADC_COUNT / adc2_p - 1) );
  Serial.print(";");
  Serial.print( R_PRE / (1023.0 * ADC_COUNT / adc3_p - 1) );
  Serial.print(";");
  Serial.print( R_PRE * (1023.0 * ADC_COUNT / adc1_n - 1) );
  Serial.print(";");
  Serial.print( R_PRE * (1023.0 * ADC_COUNT / adc2_n - 1) );
  Serial.print(";");
  Serial.print( R_PRE * (1023.0 * ADC_COUNT / adc3_n - 1) );
  Serial.println();


}



void setup() {
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  Serial.begin(9600);
  Serial.println(channel_map);
}

void loop() {
  measure();
  delay(MILLIS_DELAY_BETWEEN);

}
