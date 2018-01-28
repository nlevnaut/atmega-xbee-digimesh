#include "status.h"
#include <avr/io.h>

void status_pin_init()
{
    uint8_t portc_out = (1<<PC0)|(1<<PC1)|(1<<PC2)|(1<<PC3)|(1<<PC4)|(1<<PC5);
    uint8_t portd_out = (1<<PD3)|(1<<PD4)|(1<<PD5);
    PORTC &= ~portc_out;
    DDRC |= portc_out;
    PORTD &= ~portd_out;
    DDRD |= portd_out;
}

// Toggle these pins. Don't touch other pins.
void status_toggle(uint8_t pins)
{
    if (pins & STATUS0)
        PORTC ^= 1 << PC0;
    if (pins & STATUS1)
        PORTC ^= 1 << PC1;
    if (pins & STATUS2)
        PORTC ^= 1 << PC2;
    if (pins & STATUS3)
        PORTC ^= 1 << PC3;
    if (pins & STATUS4)
        PORTC ^= 1 << PC4;
    if (pins & STATUS5)
        PORTC ^= 1 << PC5;
    if (pins & STATUS6)
        PORTD ^= 1 << PD4;
    if (pins & STATUS7)
        PORTD ^= 1 << PD3;
    
}

// Set these pins to high. Don't touch other pins.
void status_set(uint8_t pins)
{
    if (pins & STATUS0)
        PORTC |= 1 << PC0;
    if (pins & STATUS1)
        PORTC |= 1 << PC1;
    if (pins & STATUS2)
        PORTC |= 1 << PC2;
    if (pins & STATUS3)
        PORTC |= 1 << PC3;
    if (pins & STATUS4)
        PORTC |= 1 << PC4;
    if (pins & STATUS5)
        PORTC |= 1 << PC5;
    if (pins & STATUS6)
        PORTD |= 1 << PD4;
    if (pins & STATUS7)
        PORTD |= 1 << PD3;
}

// Set these pins to low. Don't touch other pins.
void status_clear(uint8_t pins)
{
    if (pins & STATUS0)
        PORTC &= ~(1 << PC0);
    if (pins & STATUS1)
        PORTC &= ~(1 << PC1);
    if (pins & STATUS2)
        PORTC &= ~(1 << PC2);
    if (pins & STATUS3)
        PORTC &= ~(1 << PC3);
    if (pins & STATUS4)
        PORTC &= ~(1 << PC4);
    if (pins & STATUS5)
        PORTC &= ~(1 << PC5);
    if (pins & STATUS6)
        PORTD &= ~(1 << PD4);
    if (pins & STATUS7)
        PORTD &= ~(1 << PD3);
    
}

// Set only these pins to high. Other pins to low.
void status(uint8_t pins)
{
    if (pins & STATUS0)
        PORTC |= 1 << PC0;
    else
        PORTC &= ~(1 << PC0);
    if (pins & STATUS1)
        PORTC |= 1 << PC1;
    else
        PORTC &= ~(1 << PC1);
    if (pins & STATUS2)
        PORTC |= 1 << PC2;
    else
        PORTC &= ~(1 << PC2);
    if (pins & STATUS3)
        PORTC |= 1 << PC3;
    else
        PORTC &= ~(1 << PC3);
    if (pins & STATUS4)
        PORTC |= 1 << PC4;
    else
        PORTC &= ~(1 << PC4);
    if (pins & STATUS5)
        PORTC |= 1 << PC5;
    else
        PORTC &= ~(1 << PC5);
    if (pins & STATUS6)
        PORTD |= 1 << PD4;
    else
        PORTD &= ~(1 << PD4);
    if (pins & STATUS7)
        PORTD |= 1 << PD3;
    else
        PORTD &= ~(1 << PD3);
}
