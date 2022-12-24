#ifndef __DBG_H
#define __DBG_H

#include "../inc/defs.h"

// Used to arbitrary display a formatted string on the screen.
// dbg message will be displayed in the simulator window or on connected monitor
void dbg_print(uint8_t x, uint8_t y, const char *fmt, ...);

#endif