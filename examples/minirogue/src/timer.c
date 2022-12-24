#include "../inc/timer.h"

#ifndef USE_SIMULATOR

volatile uint32_t _ticks = 0;

void timer_init()
{
    _ticks = 0;
    
    TCNT1 = 0;
    OCR1A = TIMER_CLK_OVF;

    //Mode: 15, Fast PWM, TOP: OCR1A, Update at: COTTOM
    //CLK presc: 1024
    //Interrup on ovf
    TCCR1A |= _BV(WGM11) | _BV(WGM10);
    TCCR1B |= _BV(CS12)  | _BV(CS10) | _BV(WGM12) | _BV(WGM13);
    TIMSK1 |= _BV(TOIE1);
    
}

uint16_t timer_get_ticks()
{
    return _ticks;
}

ISR(TIMER1_OVF_vect)
{
    _ticks++;
}

#else

void timer_init()
{
}

uint16_t timer_get_ticks()
{
    return 0;
}

#endif