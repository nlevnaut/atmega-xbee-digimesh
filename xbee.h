#ifndef XBEE_H
#define XBEE_H

#include <stdint.h>
#include <avr/interrupt.h>
#include "rbuf.h"

#define TX_INT_ENABLE()     UCSR0B |= (1<<TXCIE0)
#define TX_INT_DISABLE()    UCSR0B &= ~(1<<TXCIE0)

#define RX_INT_ENABLE()     UCSR0B |= (1<<RXCIE0)
#define RX_INT_DISABLE()    UCSR0B &= ~(1<<RXCIE0)

#define STATUS_RX (uint8_t)(0x01)
#define STATUS_FIND_FRAME (uint8_t)(0x02)
#define STATUS_ESCAPE (uint8_t)(0x03)
#define STATUS_VALIDATE_FRAME (uint8_t)(0x04)
#define FRAME_SIZE_ERR (uint8_t)(0x01)
#define FRAME_DELIM_ERR (uint8_t)(0x02)
#define FRAME_SUM_ERR (uint8_t)(0x04)
#define FRAME_RX_INCOMPLETE (uint8_t)(0x08)

#define BROADCAST (uint64_t)(0x000000000000FFFF)
#define MAX_FRAME_SIZE (uint8_t)(MAX_BUF_SIZE - 1)

//! Buffer for serial RX interrupt service routne.
volatile extern rbuf_t rbuf;

// Digimesh frame special characters.
struct special_bytes_t
{
    uint8_t FRAME_DELIM;
    uint8_t ESCAPE;
    uint8_t XON;
    uint8_t XOFF;
};

const static struct special_bytes_t SPECIAL_BYTES;

//! Digimesh frame types.
struct frame_types_t
{
    uint8_t AT;
    uint8_t AT_QPV;
    uint8_t TX;
    uint8_t EXPLICIT_TX;
    uint8_t REMOTE;
    uint8_t AT_RESP;
    uint8_t MODEM_STATUS;
    uint8_t TX_STATUS;
    uint8_t ROUTE_INFO;
    uint8_t RX;
    uint8_t EXPLICIT_RX;
    uint8_t NODE_ID;
    uint8_t REMOTE_RESP;
};

struct frame_t
{
    uint8_t len;
    uint8_t data[MAX_BUF_SIZE];
};

const struct frame_types_t FRAME_TYPES;
const struct frame_types_t FRAME_OHEAD;

//! Initialize buffer and interrupts
void xbee_init();

//! Receive a Digimesh frame, if available in the serial buffer.
uint8_t rx(uint8_t *frame, uint16_t timeout);

//! Transmit a Digimesh frame.
uint8_t tx(uint8_t *data, uint8_t data_len, uint64_t dest, uint8_t opts);

//! Remove ESCAPE bytes and XOR special chars with 0x20.
uint8_t unescape(uint8_t *frame, uint8_t size);

uint8_t validate_frame(uint8_t *frame, uint8_t size);

uint8_t find_frame(volatile rbuf_t *r, uint8_t *frame);

uint8_t shift_frame_out(volatile rbuf_t *r);

uint8_t shift_to_delim(volatile rbuf_t *r);

uint64_t get_source_addr(uint8_t *frame);

uint8_t get_frame_type(uint8_t *frame);

uint8_t get_frame_len(uint8_t *frame);

//! ISR for writing bytes and swapping buffers when frame delimiter received.
ISR(USART_RX_vect);

#endif
