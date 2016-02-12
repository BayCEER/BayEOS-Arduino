#ifndef SENSORCALC_H
#define SENSORCALC_H

#include <math.h>

float ntc10_20(int adc){
	  float t,res;
	  t=adc;
	  res=8.195548e+01-2.523585e-01*t;
	  t*=adc;
	  res+=3.121114e-04*t;
	  t*=adc;
	  res+=-1.842112e-07*t;
	  return res;
}


float hit503x(int adc,float T){
	return ((((float) adc)/1023-0.1515)/0.00636)/(1.0546-0.00216*T);
}

float hr202l_100(int adc,float T){
	  float a,b,c;
	  float tt=T*T;
	  a=112.016090 - 0.360150168*T + 1.156667e-03*tt;
	  b=-12.725041 - 0.066866381*T - 1.365699e-04*tt;
	  c=0.373017 - 0.006363128*T + 5.289157e-05*tt;
	  float logR;
	  logR=log((float)100/((float)1023/((float) adc) - 1));
	  return a+b*logR+c*logR*logR;
}

//avarage wind direction
/*
 * N: 	0		10000
 * NO: 	7071	7071
 * O	10000	0
 * SO:	7071	-7071
 * S	0		-10000
 * SW	-7071	-7071
 * W	-10000	0
 * NW	-7071	7071
 *
 *
 *
 */


#endif
