#include <util/parity.h> 

#include "../inc/kb.h"

key_handler_ptr _kb_handler;


void kb_init(key_handler_ptr handler)
{
    _kb_handler = handler;

    // SET ALL PS2 PINS as INPUT, NO PULLUP
    PS2_DDR &= ~_BV(PS2_CLK_PIN);
    PS2_DDR &= ~_BV(PS2_DATA_PIN);

    PS2_PORT &= ~_BV(PS2_CLK_PIN);
    PS2_PORT &= ~_BV(PS2_DATA_PIN);

    EICRA |= _BV(ISC01); // trigger INT0 on falling edge
    EIMSK |= _BV(INT0);  // enable INT0 interrupts

}

uint8_t _wait_for_clk_low()
{
    for(uint8_t i=0;i<250;i++)
    {
        if(PS2_PIN & _BV(PS2_CLK_PIN))
            continue;

        return 0x01;
    }

    return 0x00;
}

uint8_t _wait_for_clk_high()
{
    for(uint8_t i=0;i<250;i++)
    {
        if(PS2_PIN & _BV(PS2_CLK_PIN))
            return 0x01;
    }

    return 0x00;
}

void kb_send_cmd(uint8_t cmd)
{
    uint8_t parity_bit = parity_even_bit(cmd);

    cli();
    
    // pull CLK low
    PS2_PORT &= ~_BV(PS2_CLK_PIN);
    PS2_DDR |= _BV(PS2_CLK_PIN);
    _delay_us(100);
    
    // pull DATA low
    PS2_PORT &= ~_BV(PS2_DATA_PIN);
    PS2_DDR |= _BV(PS2_DATA_PIN);
    
    // release CLK
    PS2_DDR &= ~_BV(PS2_CLK_PIN);

    _wait_for_clk_low();
    for(uint8_t i=0;i<8;i++)
    {
        if(cmd & 0x01)
            PS2_PORT |= _BV(PS2_DATA_PIN);
        else
            PS2_PORT &= ~_BV(PS2_DATA_PIN);

        cmd >>= 1;

        _wait_for_clk_high();
        _wait_for_clk_low();
    }

    if(parity_bit)
        PS2_PORT |= _BV(PS2_DATA_PIN);
    else
        PS2_PORT &= ~_BV(PS2_DATA_PIN);

    _wait_for_clk_high();
    _wait_for_clk_low();

    PS2_DDR &= ~_BV(PS2_DATA_PIN);
    PS2_PORT &= ~_BV(PS2_DATA_PIN);

    
    sei();
}

void kb_lock() 
{

}

void kb_unlock() 
{

}

ISR(INT0_vect)
{
    static uint8_t ps2_clock = 0;
    static uint8_t scan_code_acc = 0x00;
    static uint8_t parity_bit = 0x00;
    
    uint8_t data_pin = PS2_PIN & _BV(PS2_DATA_PIN);

    switch(ps2_clock)
    {
        case 0:     // start bit
            if(!data_pin) 
                ps2_clock++;
            break;
        case 9:     // parity bit
            parity_bit = parity_even_bit(scan_code_acc);
            if (!data_pin  ==  !parity_bit) {
                ps2_clock = 0; 
                scan_code_acc = 0x00;
            }
            else
            {
                ps2_clock++;
            }
            break;
        case 10:    // stop bit
            if(data_pin)
            {
                _kb_handler(scan_code_acc);
                ps2_clock = 0;
                scan_code_acc = 0x00;                
            }
            else
            {
                ps2_clock = 0;
                scan_code_acc = 0x00;
            }
            ps2_clock = 0;
            break;
        default: // 1-8 (bits 0-7)
            scan_code_acc >>= 1;
            if (PS2_PIN & _BV(PS2_DATA_PIN))
                scan_code_acc |= 0x80;
            ps2_clock++;
    }

}