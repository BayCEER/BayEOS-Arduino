#ifndef HYT221_h
#define HYT221_h

#define HYT221_ADDR 0x28
#include <Arduino.h>
#include <inttypes.h>

uint8_t hyt221_measure(float* temp,float* hum){
	  Wire.beginTransmission(HYT221_ADDR);
	  Wire.write(0);
	  Wire.available();
	  int Ack = Wire.read(); // receive a byte
	  delay(100);

	Wire.requestFrom(HYT221_ADDR,4);
  if(Wire.available()>3){
     //receive the databytes:

	  unsigned int valuetemp=0;
	  unsigned int valuecap=0;

      *(((uint8_t*)&valuecap)+1)=(Wire.read() & 0x3f); //Mask Status Bits
	  *((uint8_t*)&valuecap)=Wire.read();
	  *(((uint8_t*)&valuetemp)+1)=Wire.read();
	  *((uint8_t*)&valuetemp)=Wire.read();
      Wire.endTransmission();
/*

      *hum=((float)valuecap);
      *temp=((float)valuetemp);
      return 1;
*/
      valuetemp=valuetemp>>2;



      //formate the values
      *hum=((float)valuecap *100/(1<<14));
      *temp=((float)valuetemp *165/(1<<14))-40;
      return 0;
  }
  return 1;
}

#endif
