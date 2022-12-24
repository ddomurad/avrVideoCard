#ifndef __SPI_H
#define __SPI_H

#include "defs.h"

void    spi_init();
uint8_t spi_transfer(uint8_t b);
void    spi_send_cmd(uint8_t c, uint8_t d);

#endif