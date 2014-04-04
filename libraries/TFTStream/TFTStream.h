#ifndef TFTStream_h
#define TFTStream_h

#include <inttypes.h>

#include <UTFT.h>
#include "Stream.h"

extern uint8_t SmallFont[];


class TFTStream : virtual public Stream
{
  protected:
    char *_tx_buffer;
    uint8_t _rows;
    uint8_t _cols;
    UTFT * _utft;
    uint8_t _first_row;
    uint8_t _crow;
    uint8_t _ccol;
    bool _on;

  public:
    TFTStream(UTFT *utft, char *tx_buffer, uint8_t rows, uint8_t cols);
    void begin(void);
    void end(void);
    void lcdOff(void){_utft->lcdOff();};
    void lcdOn(void){_utft->lcdOn();};


    virtual size_t write(uint8_t);
    using Print::write; // pull in write(str) and write(buf, size) from Print
    void flush(void);
    bool isOn(void){return _on;};
};

class TFTStreamDev : public TFTStream
{
	public:
	TFTStreamDev(UTFT *utft, char *tx_buffer, uint8_t rows, uint8_t cols):
		TFTStream(utft, tx_buffer, rows, cols){};
	int available(void){return 0;};
    int peek(void){return 0;};
    int read(void){return 0;};

};
#endif
