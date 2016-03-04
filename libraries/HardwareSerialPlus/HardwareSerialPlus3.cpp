#include "Arduino.h"
#include "HardwareSerialPlus.h"
#include "HardwareSerialPlus_private.h"


#if defined(HAVE_HWSERIAL3P)

ISR(USART3_RX_vect)
{
  SerialPlus3._rx_complete_irq();
}

ISR(USART3_UDRE_vect)
{
  SerialPlus3._tx_udr_empty_irq();
}

unsigned char tx_buffer3[64];
unsigned char rx_buffer3[64];

HardwareSerialPlus SerialPlus3(&UBRR3H, &UBRR3L, &UCSR3A, &UCSR3B, &UCSR3C, &UDR3,rx_buffer3,64,tx_buffer3,64);

// Function that can be weakly referenced by serialEventRun to prevent
// pulling in this file if it's not otherwise used.
bool SerialPlus3_available() {
  return SerialPlus3.available();
}

#endif // HAVE_HWSERIAL3
