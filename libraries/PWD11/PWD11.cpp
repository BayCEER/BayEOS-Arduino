/* Simple Class to read PWD11 values
*/
#include <SoftwareSerial.h> 
#include <PWD11.h>
#include <inttypes.h>
#include <Arduino.h>

PWD11::PWD11(uint8_t rx, uint8_t tx):SoftwareSerial(rx,tx)
{
} 

void PWD11::begin(){
  SoftwareSerial::begin(9600); 
}

float* PWD11::readMessage2(){
	int i=0;
	char buffer[46];
	static float a[10];		
	
	while(true){
		while(SoftwareSerial::available()>0){
			byte b = SoftwareSerial::read() & 0x7f; 
			if (b == 0x2){  // Start of Text
				i = 0;
			} else if((b == 0x3) && (i == 46)){ 		// End of Text 						
				a[0] = getFloat(buffer,0,1); 			// Vis Alaram
				a[1] = getFloat(buffer,1,1); 			// Hardware Status 
				a[2] = getFloat(buffer,2,6);        	// Vis 1min 0..2000 [m]
				a[3] = getFloat(buffer,8,6);       		// Vis 10min 0..2000 [m]
														// NWS Code skipped 
				a[4] = getFloat(buffer,18,3); 			// IPW 0..99
				a[5] = getFloat(buffer,21,3);       	// IPW 15min 0..99
				a[6] = getFloat(buffer,24,3);       	// IPW 1h 0..99
				a[7] = getFloat(buffer,27,7);   		// WInt 1min [mm/h]
				a[8] = getFloat(buffer,34,7);       	// WSum 0..99.99 [mm]
				a[9] = getFloat(buffer,41,5);      		// Snow Sum 
				return a;					 
			} else { // Fill buffer 			
			if (i<46){
				buffer[i] = (char) b;    
			}
			i++; 
			}
		}
	delay(100);
	}
	
}

float PWD11::getFloat(char* buffer,int startIndex, int length){
  char val[length];  
  for(int c=0;c<length;c++){       
       val[c] = buffer[startIndex+c];               
       if (val[c] == '/'){
         return -1;
       }
  }  
  return atof(val);
}

