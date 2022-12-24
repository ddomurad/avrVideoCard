#ifndef __TIMER_H
#define __TIMER_H

#include "defs.h"

void        timer_init();
uint16_t    timer_get_ticks();

#ifndef USE_SIMULATOR
    #define timer_delay_ms(x) _delay_ms(x)
#else
    #define timer_delay_ms(x) __ENFORCE_COMMA()
#endif
#endif