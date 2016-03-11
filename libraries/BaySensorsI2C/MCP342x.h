/**
  MCP342x.h - library for MCP3422.h AD-Controller
*/

// ensure this library description is only included once
#ifndef MCP342x_h
#define MCP342x_h

// include types & constants of Wiring core API
#include <Arduino.h>

#define MCP342x_ADR B01101000

// library interface description
class MCP342x
{
  public:
    /**
     * Constructor
    */
    MCP342x();
    /**
     * get ADC value in V
    */
    float getData(byte adc_addr);
    /**
     * set configuration as binary
    */
    void setConf(uint8_t adc_addr, uint8_t conf);
    /**
     * set configuration
     * 	rdy	-	ready bit ->	in continuous mode has no effect
     * 	in one-shot mode	rdy=0 -> no efect
     * 						rdy=1 -> initiate a new conversion
     *
     * 	ch - channel selection	ch = 0-3
     * 	mode -	mode = 1 -> continuous mode: the ADC performs data conversion continuously
     * 			mode = 0 -> one-shot mode:	the ADC performs a single conversion
     * 			  						and enters standby mode
     * 	rate -	sample rate selection:	rate = 0 240 SPS 		- 12-bits conversion
     * 									rate = 1  60 SPS 		- 14-bits conversion
     * 									rate = 2  15 SPS 		-	16-bits conversion
     * 									rate = 3   3.75 SPS -	18-bits conversion
     * 	gain -	amplifier gain selection:
     * 				gain = 0  ->  x1 gain
     * 				gain = 1  ->  x2 gain
     * 				gain = 2  ->  x4 gain
     * 				gain = 3  ->  x8 gain     * adc_addr: 0-3
    */
	void setConf(byte adc_addr, byte rdy, byte ch, byte mode, byte rate, byte gain);
  private:
//		void print_binary(byte);
//    byte confByte;
//		byte address;
    byte resolution;
    byte gain;
    long zahl;
    byte output_bcd[5];

    void readOutputRegister(byte adc_addr);
//    void saveData(void);
};

#endif
 

