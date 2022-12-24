#include "../inc/utils.h"

#ifndef USE_SIMULATOR
uint32_t utils_urand_tim()
{
    return utils_urand(TCNT1);   
}
#else
#include <time.h>
uint32_t utils_urand_tim()
{
    clock_t clk = clock();
    return utils_urand((uint32_t)clk);
}
#endif

uint32_t utils_urand(uint32_t seed)
{
    seed += 0xe120fc15;
    uint64_t tmp;
    tmp = (uint64_t)seed * 0x4a39b70d;
    uint32_t m1 = (tmp >> 32) ^ tmp;
    tmp = (uint64_t)m1 * 0x12fad5c9;
    uint32_t m2 = (tmp >> 32) ^ tmp;
    return m2;
}

uint32_t utils_urand_ext(uint32_t min, uint32_t max, uint32_t seed)
{
    return min + utils_urand(seed)%(max-min);
}

uint32_t utils_urand_ext_ptr(uint32_t min, uint32_t max, uint32_t *seed)
{
    if(min == max)
        return min;

    *seed = utils_urand(*seed);
    return min + *seed%(max-min);
}
