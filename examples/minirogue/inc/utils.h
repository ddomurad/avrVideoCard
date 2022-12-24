#ifndef __UTILS_H
#define __UTILS_H

#include "defs.h"

typedef struct urect8_t {
    uint8_t x,y,w,h;
} urect8;

typedef struct rect8_t {
    int8_t x,y,w,h;
} rect8;

uint32_t utils_urand_tim();
uint32_t utils_urand(uint32_t seed);
uint32_t utils_urand_ext(uint32_t min, uint32_t max, uint32_t seed);
uint32_t utils_urand_ext_ptr(uint32_t min, uint32_t max, uint32_t *seed);

#endif