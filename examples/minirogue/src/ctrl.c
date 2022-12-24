#include "../inc/ctrl.h"
#include "../inc/timer.h"

#define PS2_KEY_RELEASE       0xf0    // key release scan code

uint8_t _ps2_state          = 0; // n-bit to be read of the ps2 protocol
uint8_t _ps2_data_acc       = 0; // data accumulator
volatile uint8_t _ps2_data  = 0; // data latch (after all bits are accumulated)
uint8_t _ps2_key_release    = 0; // key release active
uint16_t _ps2_last_clk_time = 0; // last ps2 clock pulse time

#ifndef USE_SIMULATOR

void ctrl_init()
{
    // SET ALL PS2 PINS as INPUT, NO PULLUP
    PS2_DDR &= ~_BV(PS2_CLK_PIN);
    PS2_DDR &= ~_BV(PS2_DATA_PIN);

    PS2_PORT &= ~_BV(PS2_CLK_PIN);
    PS2_PORT &= ~_BV(PS2_DATA_PIN);

    EICRA |= _BV(ISC01); // trigger INT0 on falling edge
    EIMSK |= _BV(INT0);  // enable INT0 interrupts

    _ps2_state = 0;
    _ps2_data_acc = 0;
    _ps2_data = 0;
    _ps2_key_release = 0;
}

uint8_t ctrl_pop_key()
{   
    uint8_t d = _ps2_data;
     _ps2_data = 0;
    return d;
}

ISR(INT0_vect)
{
    uint16_t now = timer_get_ticks();

    // if the last clock interrupt happend more than 20ms
    // then reset the state of the protocol 
    if(now - _ps2_last_clk_time > 2)
    {
        _ps2_state = 0;
        _ps2_data_acc = 0;
        _ps2_key_release = 0;
    }
    
    _ps2_last_clk_time = now;

    // accumulate bits [1, 8]
    if (_ps2_state > 0 && _ps2_state <= 8)
    {
        _ps2_data_acc = _ps2_data_acc >> 1;
        if (PS2_PIN & _BV(PS2_DATA_PIN))
            _ps2_data_acc |= 0x80;
    }

    _ps2_state++;

    // if this was the last bit
    // then latch the data, and reset the protocol status
    if (_ps2_state > 10)
    {
        _ps2_state = 0;

        // if accumulated data was a release scan code
        // then set _ps2_key_release
        if(_ps2_data_acc == PS2_KEY_RELEASE) {
            _ps2_key_release = 1;
            _ps2_data_acc = 0;
            return;
        }

        // if previouse scan code was a release,
        //  then ignore current one
        if(_ps2_key_release)
        {
            _ps2_key_release = 0;
            _ps2_data_acc = 0;
            return;
        }

        // at last, latch the data
        _ps2_data = _ps2_data_acc;
        _ps2_data_acc = 0;
    }
}

#else
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

void ctrl_init()
{}

// in simulation mode, we just read the stdio
// and map input to scan codes 
uint8_t ctrl_pop_key()
{
    uint8_t b = 0;
    read(0, &b, 1);
    
    if(b == '7') 
        return CTRL_UP_LEFT;

    if(b == '8') 
        return CTRL_UP;

    if(b == '9') 
        return CTRL_UP_RIGHT;

    if(b == '4') 
        return CTRL_LEFT;

    if(b == '6') 
        return CTRL_RIGHT;

    if(b == '1') 
        return CTRL_DOWN_LEFT;

    if(b == '2') 
        return CTRL_DOWN;

    if(b == '3') 
        return CTRL_DOWN_RIGHT;

    if(b == 'r') 
        return CTRL_F5;

    if(b == 't') 
        return CTRL_F6;

    if(b == '0') 
        return CTRL_TAKE_ITEM;    

    return 0;
}

#endif