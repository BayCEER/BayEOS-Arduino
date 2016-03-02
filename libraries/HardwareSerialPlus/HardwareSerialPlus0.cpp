#include "Arduino.h"
#include "HardwareSerialPlus.h"
#include "HardwareSerialPlus_private.h"

#if defined(HAVE_HWSERIAL0P)

#if defined(USART_RX_vect)
  ISR(USART_RX_vect)
#elif defined(USART0_RX_vect)
  ISR(USART0_RX_vect)
#elif defined(USART_RXC_vect)
  ISR(USART_RXC_vect) // ATmega8
#else
  #error "Don't know what the Data Received vector is called for Serial"
#endif
  {
    SerialPlus._rx_complete_irq();
  }

#if defined(UART0_UDRE_vect)
ISR(UART0_UDRE_vect)
#elif defined(UART_UDRE_vect)
ISR(UART_UDRE_vect)
#elif defined(USART0_UDRE_vect)
ISR(USART0_UDRE_vect)
#elif defined(USART_UDRE_vect)
ISR(USART_UDRE_vect)
#else
  #error "Don't know what the Data Register Empty vector is called for Serial"
#endif
{
  SerialPlus._tx_udr_empty_irq();
}

unsigned char tx_buffer[16];
unsigned char rx_buffer[16];

#if defined(UBRRH) && defined(UBRRL)
  HardwareSerialPlus SerialPlus(&UBRRH, &UBRRL, &UCSRA, &UCSRB, &UCSRC, &UDR,rx_buffer,16,tx_buffer,16);
#else
  HardwareSerialPlus SerialPlus(&UBRR0H, &UBRR0L, &UCSR0A, &UCSR0B, &UCSR0C, &UDR0,rx_buffer,16,tx_buffer,16);
#endif

// Function that can be weakly referenced by serialPlusEventRun to prevent
// pulling in this file if it's not otherwise used.
bool Serial0Plus_available() {
  return SerialPlus.available();
}

#endif // HAVE_HWSERIAL0P
