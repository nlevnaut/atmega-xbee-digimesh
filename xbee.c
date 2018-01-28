#include "xbee.h"
#include "rbuf.h"
#include "status.h"
#include "serial.h"
#include "controller.h"
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>

volatile rbuf_t rbuf;

// Digimesh frame types. Typically the 4th byte in a frame.
const struct frame_types_t FRAME_TYPES =
{
    .AT             = (uint8_t)0x08,
    .AT_QPV         = (uint8_t)0x09,
    .TX             = (uint8_t)0x10,
    .EXPLICIT_TX    = (uint8_t)0x11,
    .REMOTE         = (uint8_t)0x17,
    .AT_RESP        = (uint8_t)0x88,
    .MODEM_STATUS   = (uint8_t)0x8A,
    .TX_STATUS      = (uint8_t)0x8B,
    .ROUTE_INFO     = (uint8_t)0x8D,
    .RX             = (uint8_t)0x90,
    .EXPLICIT_RX    = (uint8_t)0x91,
    .NODE_ID        = (uint8_t)0x95,
    .REMOTE_RESP    = (uint8_t)0x97
};  

// Number of overhead bytes for certain frame types.
const struct frame_types_t FRAME_OHEAD =
{
    .TX             = (uint8_t)18,
    .RX             = (uint8_t)16,
};

// Special bytes that need to be escaped in data.
const static struct special_bytes_t SPECIAL_BYTES =
{
    .FRAME_DELIM = (uint8_t)0x7E,
    .ESCAPE      = (uint8_t)0x7D,
    .XON         = (uint8_t)0x11,
    .XOFF        = (uint8_t)0x13
};

// Initialize Xbee buffers and enable rx interrupt.
void xbee_init()
{
    rbuf.start = 0;
    rbuf.end = 0;
    // clear the buffer when initializing
    for (int i=0; i < MAX_BUF_SIZE; i++)
        rbuf.buf[i] = 0;
    /* TX_INT_ENABLE(); */
    RX_INT_ENABLE();
}

// Transmit data of length data_len to destination address dest.
uint8_t tx(uint8_t *data, uint8_t data_len, uint64_t dest, uint8_t opts)
{
    status_clear(STATUS1);
    uint8_t frame[MAX_FRAME_SIZE];
    /* uint8_t frame[MAX_BUF_SIZE + 18]; */

    // TX frame has 14 bytes overhead
    // does not include delimiter or length or checksum
    data_len += 14;
    // add delimiter and length and checksum
    uint8_t frame_len = data_len + 4;
    uint8_t sum = 0;

    if (frame_len > MAX_FRAME_SIZE)
        return FRAME_SIZE_ERR;

    frame[0] = 0x7E;
    frame[1] = (uint8_t)(data_len >> 8);
    frame[2] = (uint8_t)(data_len);
    frame[3] = FRAME_TYPES.TX;
    frame[4] = 0x00;    // disable TX STATUS response
    frame[5] = (uint8_t)(dest >> 56);
    frame[6] = (uint8_t)(dest >> 48);
    frame[7] = (uint8_t)(dest >> 40);
    frame[8] = (uint8_t)(dest >> 32);
    frame[9] = (uint8_t)(dest >> 24);
    frame[10] = (uint8_t)(dest >> 16);
    frame[11] = (uint8_t)(dest >> 8);
    frame[12] = (uint8_t)(dest);
    frame[13] = 0xFF;   // reserved
    frame[14] = 0xFE;   // reserved
    frame[15] = 0x00;   // broadcast radius (default 0x00 for radius=max hops)
    frame[16] = opts;   // tx options (probably should just be 0x00)

    // compute first part of checksum.
    for (int i=3; i<17; i++)
    {
        sum += frame[i];
    }

    // append data and sum it
    for (int i=17; i < (frame_len - 1); i++)
    {
        frame[i] = data[i - 17];
        sum += data[i - 17];
    }

    // put checksum at the end
    frame[frame_len - 1] = 0xFF - sum;

    // send it
    // escape it if we have to, but not the frame delimiter
    put_byte(frame[0]);
    for (int i=1; i < frame_len; i++)
    {
        if (frame[i] == SPECIAL_BYTES.FRAME_DELIM ||
            frame[i] == SPECIAL_BYTES.ESCAPE      ||
            frame[i] == SPECIAL_BYTES.XON         ||
            frame[i] == SPECIAL_BYTES.XOFF)
        {
            put_byte(SPECIAL_BYTES.ESCAPE);
            put_byte(frame[i] ^ 0x20);
        }
        else
        {
            put_byte(frame[i]);
        }
    }
            
    status_set(STATUS1);
    return 0;
}

//! rx(frame) assumes frame has MAX_FRAME_SIZE bytes allocated already.
//! DO NOT use this if frame is unallocated.
uint8_t rx(uint8_t *frame, uint16_t timeout)
{
    uint8_t ret = 1;
    do
    {
        // zero-out the frame buffer before each check.
        for (int i=0; i<MAX_FRAME_SIZE; i++)
            frame[i] = 0x00;
        // find a valid frame in rbuf.
        ret = find_frame(&rbuf, frame);
    }
    while ((ret != 0) && (timeout > timer_1));
    return ret;
}


//! Checks the receive buffer for any potential frames. 
//! Try validation, and then shift out of the buffer if validated.
//! It is important that no interrupts call rbuf_shift()
//! while this function is executing.
uint8_t find_frame(volatile rbuf_t *r, uint8_t *frame)
{
    uint8_t buf_len;
    uint8_t ret;
    uint8_t num_unescaped;
    // Check that the first byte is a frame delimiter.
    // If not, shift out bytes until we hit one.
    buf_len = rbuf_len(r);

    if (buf_len == 0)
        return RBUF_EMPTY;

    shift_to_delim(r);
    if (rbuf_read(r, 0) == SPECIAL_BYTES.FRAME_DELIM)
    {
        buf_len = rbuf_len(r);
        if (buf_len > MAX_FRAME_SIZE)
        {
            // Buffer longer than max frame. Throw it away.
            shift_frame_out(&rbuf);
            return FRAME_SIZE_ERR;
        }
        for (int i=0; i < buf_len; i++)
        {
            frame[i] = rbuf_read(r, i);
        }

        num_unescaped = unescape(frame, buf_len);

        ret = validate_frame(frame, buf_len - num_unescaped);
    }
    else
    {
        // could not find frame delimiter.
        ret = FRAME_DELIM_ERR;
    }
    return ret;
}

//! Check the checksum. 
uint8_t validate_frame(uint8_t *frame, uint8_t buf_len)
{
    uint8_t ret = 0;
    uint8_t sum = 0;
    uint8_t data_len, frame_len;

    // check that we have at least frame_len # of bytes in the buffer.
    data_len = ((uint8_t)frame[1] << 8) | (uint8_t)frame[2];
    frame_len = data_len + 4;
    if (frame_len > buf_len)
    {
        if (frame_len > MAX_FRAME_SIZE)
        {
            // Frame too large for the buffer.
            shift_frame_out(&rbuf);
            ret = FRAME_SIZE_ERR;
        }
        else
        {
            // Not done receiving this frame.
            ret = FRAME_RX_INCOMPLETE;
        }
    }
    else
    {
        // Sum from byte 3 to the end of the frame, including checksum.
        for (int i=3; i < (frame_len); i++)
        {
            sum += frame[i];
        }
        // Make sure they add to 0xFF.
        if ((uint8_t)(sum & 0xFF) != (uint8_t)0xFF)
        {
            ret = FRAME_SUM_ERR;
        }
        // Shift it out of the buffer, whether it's good or not.
        /* rbuf_shift(&rbuf, frame_len); */
        shift_frame_out(&rbuf);
    }
    return ret;
}

// This will shift up to the frame delimiter, or shift
// everything out if one was not found.
uint8_t shift_to_delim(volatile rbuf_t *r)
{
    uint8_t i;
    uint8_t found = 0;
    uint8_t buf_len = rbuf_len(r);

    if (rbuf_read(r, 0) != SPECIAL_BYTES.FRAME_DELIM)
    {
        // Find the next frame delimiter.
        for (i=1; i<buf_len; i++)
        {
            if (rbuf_read(r, i) == SPECIAL_BYTES.FRAME_DELIM)
            {
                found = 1;
                rbuf_shift(r, i);
                break;
            }
        }
        if (!found)
            rbuf_shift(r, buf_len);
    }
    else
        found = 2;
    return found;
}

// The same as shift_to_delim, but shifts the current frame out
// no matter what.
uint8_t shift_frame_out(volatile rbuf_t *r)
{
    uint8_t i;
    uint8_t found = 0;
    uint8_t buf_len = rbuf_len(r);

    // Find the next frame delimiter.
    for (i=1; i<buf_len; i++)
    {
        if (rbuf_read(r, i) == SPECIAL_BYTES.FRAME_DELIM)
        {
            found = 1;
            rbuf_shift(r, i);
            break;
        }
    }
    if (!found)
        rbuf_shift(r, buf_len);
    return found;
}
//! Loops through the frame, unescaping any escaped bytes.
//! Could be done in find_frame and save a loop, but let's see if
//! that's necessary before premature optimization...
uint8_t unescape(uint8_t *frame, uint8_t frame_len)
{
    uint8_t i;
    uint8_t j = 0;
    // stop if we reach the end of the array. 
    /* while (i + j < frame_len) */
    for (i=1; i + j < frame_len; i++)
    {
        // Check that we reached an escape byte.
        if (frame[i+j] == SPECIAL_BYTES.ESCAPE)
        {
            j++;
            frame[i] = frame[i + j] ^ 0x20;
        }
        else
        {
            frame[i] = frame[i + j];
        }
        /* i++; */
    }
    return j;
}

uint64_t get_source_addr(uint8_t *frame)
{
    if (get_frame_type(frame) == FRAME_TYPES.RX)
    {
        return  (uint64_t)frame[4] << 56 |
                (uint64_t)frame[5] << 48 |
                (uint64_t)frame[6] << 40 |
                (uint64_t)frame[7] << 32 |
                (uint64_t)frame[8] << 24 |
                (uint64_t)frame[9] << 16 |
                (uint64_t)frame[10] << 8 |
                (uint64_t)frame[11];
    }
    else
    {
        return (uint64_t)1;
    }
}

uint8_t get_frame_type(uint8_t *frame)
{
    return frame[3];
}

uint8_t get_frame_len(uint8_t *frame)
{
    return (((uint8_t)frame[1]<<8) | (uint8_t)frame[2]) + 4;
}


ISR(USART_RX_vect)
{
    uint8_t d; // to hold current UDR0 value
    while(!(UCSR0A & (1<<RXC0)));
    //if (rbuf.end >= MAX_BUF_SIZE)
    //    rbuf.end = 0;
    d = UDR0;
    rbuf_append(&rbuf, d);
    //rbuf.end++;
    //rbuf.buf[rbuf.end] = UDR0;
}
