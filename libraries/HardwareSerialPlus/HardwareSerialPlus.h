#ifndef HardwareSerialPlus_h
#define HardwareSerialPlus_h
#include <inttypes.h>

#include "Stream.h"
// Define config for Serial.begin(baud, config);
#define SERIAL_5N1 0x00
#define SERIAL_6N1 0x02
#define SERIAL_7N1 0x04
#define SERIAL_8N1 0x06
#define SERIAL_5N2 0x08
#define SERIAL_6N2 0x0A
#define SERIAL_7N2 0x0C
#define SERIAL_8N2 0x0E
#define SERIAL_5E1 0x20
#define SERIAL_6E1 0x22
#define SERIAL_7E1 0x24
#define SERIAL_8E1 0x26
#define SERIAL_5E2 0x28
#define SERIAL_6E2 0x2A
#define SERIAL_7E2 0x2C
#define SERIAL_8E2 0x2E
#define SERIAL_5O1 0x30
#define SERIAL_6O1 0x32
#define SERIAL_7O1 0x34
#define SERIAL_8O1 0x36
#define SERIAL_5O2 0x38
#define SERIAL_6O2 0x3A
#define SERIAL_7O2 0x3C
#define SERIAL_8O2 0x3E

class HardwareSerialPlus : public Stream
{
protected:
    volatile uint8_t * const _ubrrh;
    volatile uint8_t * const _ubrrl;
    volatile uint8_t * const _ucsra;
    volatile uint8_t * const _ucsrb;
    volatile uint8_t * const _ucsrc;
    volatile uint8_t * const _udr;

    // Has any byte been written to the UART since begin()
    bool _written;
    volatile uint16_t _rx_buffer_head;
    volatile uint16_t _rx_buffer_tail;
    volatile uint16_t _tx_buffer_head;
    volatile uint16_t _tx_buffer_tail;

    uint16_t _rx_buffer_size;
    uint16_t _tx_buffer_size;
    unsigned char* _rx_buffer;
    unsigned char* _tx_buffer;
public:
    inline HardwareSerialPlus(
      volatile uint8_t *ubrrh, volatile uint8_t *ubrrl,
      volatile uint8_t *ucsra, volatile uint8_t *ucsrb,
      volatile uint8_t *ucsrc, volatile uint8_t *udr,
      unsigned char *rxbuffer, uint16_t rxsize,unsigned char *txbuffer, uint16_t txsize);
    void setRxBuffer(unsigned char *buffer, uint16_t size);
    void setTxBuffer(unsigned char *buffer, uint16_t size);
    void begin(unsigned long baud) { begin(baud, SERIAL_8N1); }
    void begin(unsigned long, uint8_t);
    void end();
    int available(void);
    int peek(void);
    void flush(void);
    int read(void);
    int availableForWrite(void);
    size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write; // pull in write(str) and write(buf, size) from Print
    operator bool() { return true; }
    inline void _rx_complete_irq(void);
    void _tx_udr_empty_irq(void);
};

#if defined(UBRRH) || defined(UBRR0H)
  extern HardwareSerialPlus SerialPlus;
  #define HAVE_HWSERIAL0P
#endif
#if defined(UBRR1H)
  extern HardwareSerialPlus SerialPlus1;
  #define HAVE_HWSERIAL1P
#endif
#if defined(UBRR2H)
  extern HardwareSerialPlus SerialPlus2;
  #define HAVE_HWSERIAL2P
#endif
#if defined(UBRR3H)
  extern HardwareSerialPlus SerialPlus3;
  #define HAVE_HWSERIAL3P
#endif

extern void serialPlusEventRun(void) __attribute__((weak));


#endif
