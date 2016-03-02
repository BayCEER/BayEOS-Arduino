#include "Arduino.h"
#include "HardwareSerialPlus.h"
#include "HardwareSerialPlus_private.h"


#if defined(HAVE_HWSERIAL1P)

#if defined(UART1_RX_vect)
ISR(UART1_RX_vect)
#elif defined(USART1_RX_vect)
ISR(USART1_RX_vect)
#else
#error "Don't know what the Data Register Empty vector is called for Serial1"
#endif
{
  SerialPlus1._rx_complete_irq();
}

#if defined(UART1_UDRE_vect)
ISR(UART1_UDRE_vect)
#elif defined(USART1_UDRE_vect)
ISR(USART1_UDRE_vect)
#else
#error "Don't know what the Data Register Empty vector is called for Serial1"
#endif
{
  SerialPlus1._tx_udr_empty_irq();
}
unsigned char tx_buffer1[16];
unsigned char rx_buffer1[16];

HardwareSerialPlus SerialPlus1(&UBRR1H, &UBRR1L, &UCSR1A, &UCSR1B, &UCSR1C, &UDR1,rx_buffer1,16,tx_buffer1,16);

// Function that can be weakly referenced by serialEventRun to prevent
// pulling in this file if it's not otherwise used.
bool SerialPlus1_available() {
  return SerialPlus1.available();
}

#endif // HAVE_HWSERIAL1

