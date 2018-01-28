#include "serial.h"

uint8_t serial_init(uint16_t baudrate, uint8_t bits, uint8_t parity, uint8_t stop)
{
    uint8_t err = 0;
    if ((bits != DATA_BITS_5) &&
        (bits != DATA_BITS_6) &&
        (bits != DATA_BITS_7) &&
        (bits != DATA_BITS_8) &&
        (bits != DATA_BITS_9))
    {
        // data bits arg invalid
        err |= DATA_BITS_ERR;
    }
    if ((parity != PARITY_DISABLED) &&
        (parity != PARITY_EVEN) &&
        (parity != PARITY_ODD))
    {
        // parity arg invalid
        err |= PARITY_ERR;
    }
    if ((stop != STOP_BITS_1) && (stop != STOP_BITS_2))
    {
        // stop bits arg invalid
        err |= STOP_BITS_ERR;
    }
    if (!err)
    {
        // Set register init values
        UBRR0H = baudrate >> 8;
        UBRR0L = baudrate;
        UCSR0C = bits | parity | stop;
        TX_ENABLE();
        RX_ENABLE();
    }
    return err;
}

void put_byte(uint8_t byte)
{
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = byte;
}
