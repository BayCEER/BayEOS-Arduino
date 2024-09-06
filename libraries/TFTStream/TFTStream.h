#ifndef TFTStream_h
#define TFTStream_h

#include <Stream.h>

#include <TFT.h>


class TFTStream : virtual public Stream
{
protected:
  char *_tx_buffer;
  uint8_t _rows;
  uint8_t _cols;
  TFT *_tft;
  uint8_t _first_row;
  uint8_t _crow;
  uint8_t _ccol;
  bool _on;

public:
  TFTStream(TFT *tft, char *tx_buffer, uint8_t rows, uint8_t cols);
  void begin(void);
  void end(void);

  size_t write(uint8_t);
  inline size_t write(unsigned long n) { return write((uint8_t)n); }
  inline size_t write(long n) { return write((uint8_t)n); }
  inline size_t write(unsigned int n) { return write((uint8_t)n); }
  inline size_t write(int n) { return write((uint8_t)n); }
  using Print::write; // pull in write(str) and write(buf, size) from Print
  void flush(void);
  bool isOn(void) { return _on; };
};

class TFTStreamDev : public TFTStream
{
public:
  TFTStreamDev(TFT *tft, char *tx_buffer, uint8_t rows, uint8_t cols) : TFTStream(tft, tx_buffer, rows, cols) {};
  int available(void) { return 0; };
  int peek(void) { return -1; };
  int read(void) { return -1; };
};
#endif
