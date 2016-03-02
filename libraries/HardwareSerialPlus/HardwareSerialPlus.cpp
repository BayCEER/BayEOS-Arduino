#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

#include "HardwareSerialPlus.h"
#include "HardwareSerialPlus_private.h"

// SerialEvent functions are weak, so when the user doesn't define them,
// the linker just sets their address to 0 (which is checked below).
// The Serialx_available is just a wrapper around Serialx.available(),
// but we can refer to it weakly so we don't pull in the entire
// HardwareSerial instance if the user doesn't also refer to it.
#if defined(HAVE_HWSERIAL0P)
  void serialPlusEvent() __attribute__((weak));
  bool Serial0Plus_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL1P)
  void serialPlusEvent1() __attribute__((weak));
  bool SerialPlus1_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL2P)
  void serialPlusEvent2() __attribute__((weak));
  bool SerialPlus2_available() __attribute__((weak));
#endif

#if defined(HAVE_HWSERIAL3P)
  void serialPlusEvent3() __attribute__((weak));
  bool SerialPlus3_available() __attribute__((weak));
#endif

void serialEventRun(void)
{
#if defined(HAVE_HWSERIAL0P)
  if (Serial0Plus_available && serialPlusEvent && Serial0Plus_available()) serialPlusEvent();
#endif
#if defined(HAVE_HWSERIAL1P)
  if (SerialPlus1_available && serialPlusEvent1 && SerialPlus1_available()) serialPlusEvent1();
#endif
#if defined(HAVE_HWSERIAL2P)
  if (SerialPlus2_available && serialPlusEvent2 && SerialPlus2_available()) serialPlusEvent2();
#endif
#if defined(HAVE_HWSERIAL3P)
  if (SerialPlus3_available && serialPlusEvent3 && SerialPlus3_available()) serialPlusEvent3();
#endif
}



void HardwareSerialPlus::setRxBuffer(unsigned char* buffer, uint16_t size){
	  _rx_buffer = buffer;
	  _rx_buffer_size = size;
}

void HardwareSerialPlus::setTxBuffer(unsigned char* buffer, uint16_t size){
	  _tx_buffer = buffer;
	  _tx_buffer_size = size;
}




// Actual interrupt handlers //////////////////////////////////////////////////////////////


void HardwareSerialPlus::_tx_udr_empty_irq(void)
{
  // If interrupts are enabled, there must be more data in the output
  // buffer. Send the next byte
  unsigned char c = _tx_buffer[_tx_buffer_tail];
  _tx_buffer_tail = (_tx_buffer_tail + 1) % _tx_buffer_size;

  *_udr = c;

  // clear the TXC bit -- "can be cleared by writing a one to its bit
  // location". This makes sure flush() won't return until the bytes
  // actually got written
  sbi(*_ucsra, TXC0);

  if (_tx_buffer_head == _tx_buffer_tail) {
    // Buffer empty, so disable interrupts
    cbi(*_ucsrb, UDRIE0);
  }
}

void HardwareSerialPlus::begin(unsigned long baud, byte config)
{
  // Try u2x mode first
  uint16_t baud_setting = (F_CPU / 4 / baud - 1) / 2;
  *_ucsra = 1 << U2X0;

  // hardcoded exception for 57600 for compatibility with the bootloader
  // shipped with the Duemilanove and previous boards and the firmware
  // on the 8U2 on the Uno and Mega 2560. Also, The baud_setting cannot
  // be > 4095, so switch back to non-u2x mode if the baud rate is too
  // low.
  if (((F_CPU == 16000000UL) && (baud == 57600)) || (baud_setting >4095))
  {
    *_ucsra = 0;
    baud_setting = (F_CPU / 8 / baud - 1) / 2;
  }

  // assign the baud_setting, a.k.a. ubrr (USART Baud Rate Register)
  *_ubrrh = baud_setting >> 8;
  *_ubrrl = baud_setting;

  _written = false;

  //set the data bits, parity, and stop bits
#if defined(__AVR_ATmega8__)
  config |= 0x80; // select UCSRC register (shared with UBRRH)
#endif
  *_ucsrc = config;

  sbi(*_ucsrb, RXEN0);
  sbi(*_ucsrb, TXEN0);
  sbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);
}

void HardwareSerialPlus::end()
{
  // wait for transmission of outgoing data
  flush();

  cbi(*_ucsrb, RXEN0);
  cbi(*_ucsrb, TXEN0);
  cbi(*_ucsrb, RXCIE0);
  cbi(*_ucsrb, UDRIE0);

  // clear any received data
  _rx_buffer_head = _rx_buffer_tail;
}

int HardwareSerialPlus::available(void)
{
  return ((unsigned int)(_rx_buffer_size + _rx_buffer_head - _rx_buffer_tail)) % _rx_buffer_size;
}

int HardwareSerialPlus::peek(void)
{
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    return _rx_buffer[_rx_buffer_tail];
  }
}


int HardwareSerialPlus::read(void)
{
  // if the head isn't ahead of the tail, we don't have any characters
  if (_rx_buffer_head == _rx_buffer_tail) {
    return -1;
  } else {
    unsigned char c = _rx_buffer[_rx_buffer_tail];
    _rx_buffer_tail = (_rx_buffer_tail + 1) % _rx_buffer_size;
    return c;
  }
}

int HardwareSerialPlus::availableForWrite(void)
{
  uint8_t oldSREG = SREG;
  cli();
  uint16_t head = _tx_buffer_head;
  uint16_t tail = _tx_buffer_tail;
  SREG = oldSREG;
  if (head >= tail) return _tx_buffer_size - 1 - head + tail;
  return tail - head - 1;
}

void HardwareSerialPlus::flush()
{
  // If we have never written a byte, no need to flush. This special
  // case is needed since there is no way to force the TXC (transmit
  // complete) bit to 1 during initialization
  if (!_written)
    return;

  while (bit_is_set(*_ucsrb, UDRIE0) || bit_is_clear(*_ucsra, TXC0)) {
    if (bit_is_clear(SREG, SREG_I) && bit_is_set(*_ucsrb, UDRIE0))
	// Interrupts are globally disabled, but the DR empty
	// interrupt should be enabled, so poll the DR empty flag to
	// prevent deadlock
	if (bit_is_set(*_ucsra, UDRE0))
	  _tx_udr_empty_irq();
  }
  // If we get here, nothing is queued anymore (DRIE is disabled) and
  // the hardware finished tranmission (TXC is set).
}

size_t HardwareSerialPlus::write(uint8_t c)
{
  _written = true;
  // If the buffer and the data register is empty, just write the byte
  // to the data register and be done. This shortcut helps
  // significantly improve the effective datarate at high (>
  // 500kbit/s) bitrates, where interrupt overhead becomes a slowdown.
  if (_tx_buffer_head == _tx_buffer_tail && bit_is_set(*_ucsra, UDRE0)) {
    *_udr = c;
    sbi(*_ucsra, TXC0);
    return 1;
  }
  uint16_t i = (_tx_buffer_head + 1) % _tx_buffer_size;

  // If the output buffer is full, there's nothing for it other than to
  // wait for the interrupt handler to empty it a bit
  while (i == _tx_buffer_tail) {
    if (bit_is_clear(SREG, SREG_I)) {
      // Interrupts are disabled, so we'll have to poll the data
      // register empty flag ourselves. If it is set, pretend an
      // interrupt has happened and call the handler to free up
      // space for us.
      if(bit_is_set(*_ucsra, UDRE0))
	_tx_udr_empty_irq();
    } else {
      // nop, the interrupt handler will free up space for us
    }
  }

  _tx_buffer[_tx_buffer_head] = c;
  _tx_buffer_head = i;

  sbi(*_ucsrb, UDRIE0);

  return 1;
}

