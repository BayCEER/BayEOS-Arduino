/*
 * TFT Debugging Class
 *
 * S.Holzheu (holzheu@bayceer.uni-bayreuth.de)
 *
 * This is a Class sending BayEOS-Frames in a human readable form
 * helps debugging Arduino-BayEOS-Projects...
 *
 */

#ifndef BayDEBUGTFT_h
#define BayDEBUGTFT_h

#include <BayDebug.h>
#include <TFTStream.h>


class BayTFTDebug : public TFTStream, public BayEOSDebugInterface {
public:
	/**
	 * Constructor
	 */
	BayTFTDebug(UTFT *utft, char *tx_buffer, uint8_t rows, uint8_t cols);
	void flush(void){TFTStream::flush();};
	using Print::println;
	using Print::print;
	using Print::write;
	size_t write(uint8_t b){return TFTStream::write(b);}
private:
	int available(void){return TFTStream::available();}
	int read(void){return TFTStream::read();}
    int peek(void){return TFTStream::peek();};

};



#endif
