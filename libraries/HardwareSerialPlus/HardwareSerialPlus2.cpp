#include "Arduino.h"
#include "HardwareSerialPlus.h"
#include "HardwareSerialPlus_private.h"


#if defined(HAVE_HWSERIAL2P)

ISR(USART2_RX_vect)
{
  SerialPlus2._rx_complete_irq();
}

ISR(USART2_UDRE_vect)
{
  SerialPlus2._tx_udr_empty_irq();
}
unsigned char tx_buffer2[64];
unsigned char rx_buffer2[64];

HardwareSerialPlus SerialPlus2(&UBRR2H, &UBRR2L, &UCSR2A, &UCSR2B, &UCSR2C, &UDR2,rx_buffer2,64,tx_buffer2,64);

// Function that can be weakly referenced by serialEventRun to prevent
// pulling in this file if it's not otherwise used.
bool SerialPlus2_available() {
  return SerialPlus2.available();
}

#endif // HAVE_HWSERIAL2
