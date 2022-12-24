#include <stdio.h>
#include <stdarg.h>
#include "../inc/dbg.h"

#include "../inc/gfx.h"

char buffer[64];

void dbg_print(uint8_t x, uint8_t y, const char *fmt, ...)
{
    va_list argptr;

    va_start(argptr, fmt);
    vsnprintf(buffer, 64, fmt, argptr);
    va_end(argptr);

    gfx_set_glyph_addr(x, y);
    gfx_put_string(buffer);
}