#ifndef STATUS_H
#define STATUS_H

#include <stdint.h>

// Serial errors
#define DATA_BITS_ERR   (uint8_t)(0x01)
#define STOP_BITS_ERR   (uint8_t)(0x02)
#define PARITY_ERR      (uint8_t)(0x04)

// Memory errors
#define OOM_ERR         (uint8_t)(0x08)

// Frame errors
#define FRAME_SIZE_ERR  (uint8_t)(0x01)
#define FRAME_DELIM_ERR (uint8_t)(0x02)
#define FRAME_SUM_ERR   (uint8_t)(0x04)

#define STATUS0         (uint8_t)(0x01)
#define STATUS1         (uint8_t)(0x02)
#define STATUS2         (uint8_t)(0x04)
#define STATUS3         (uint8_t)(0x08)
#define STATUS4         (uint8_t)(0x10)
#define STATUS5         (uint8_t)(0x20)
#define STATUS6         (uint8_t)(0x40)
#define STATUS7         (uint8_t)(0x80)
#define STATUS_CLR      (uint8_t)(0x00)

//! Initialize GPIO for outputting error values.
void status_pin_init();

void status_toggle(uint8_t pins);

void status_set(uint8_t pins);

void status_clear(uint8_t pins);

void status_or(uint8_t pins);

void status(uint8_t pins);

#endif
