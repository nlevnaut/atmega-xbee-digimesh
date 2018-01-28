#ifndef RBUF_H
#define RBUF_H

#include <stdint.h>
#include <stdlib.h>

#define MAX_BUF_SIZE (uint8_t)(255)

#define RBUF_WRITE_SUCCESS 0x00
#define RBUF_FULL 0x01
#define RBUF_EMPTY 0x02

typedef struct {
    uint8_t buf[MAX_BUF_SIZE];
    uint8_t start;
    uint8_t end;
} rbuf_t;

uint8_t rbuf_append(volatile rbuf_t *r,
                    uint8_t x);

void rbuf_write(volatile rbuf_t *r,
                uint8_t x,
                uint8_t i);

uint8_t rbuf_read(volatile rbuf_t *r,
                  uint8_t i);

void rbuf_shift(volatile rbuf_t *r,
                uint8_t shamt);

uint8_t rbuf_len(volatile rbuf_t *r);


#endif
