# atmega-xbee-digimesh
TX and RX with a Digimesh XBee over the ATMega328's UART.

Example usage:
```
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "serial.h"
#include "xbee.h"
#include "status.h"

#define USART_BAUDRATE 1200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define RX_TIMEOUT 500 //ms
#define TX_OPTS 0x00

uint8_t rx_status = 0;
uint8_t frame[MAX_BUF_SIZE];

tim_init();
serial_init(BAUD_PRESCALE, DATA_BITS_8, STOP_BITS_1, PARITY_DISABLED);
xbee_init();
sei();

// Broadcast a message
tx("Hello world", 11, BROADCAST, TX_OPTS);

// Receive messages
while(1)
{
    rx_status = rx(frame, RX_TIMEOUT);
    if (rx_status)
    {
        // Do something with frame here
    }
}
```
