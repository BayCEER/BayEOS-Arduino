#include <NTC.h>
NTC_Sensor ntc(10.0);

void setup(void){
  Serial.begin(9600);

 //Resistor-Values for NTC10 in Ohm
 //transformed into °C
  Serial.println(ntc.R2T(5000));
  Serial.println(ntc.R2T(7000));
  Serial.println(ntc.R2T(10000));
  Serial.println(ntc.R2T(15000));
  Serial.println(ntc.R2T(20000));
}

void loop(void){
  
}

