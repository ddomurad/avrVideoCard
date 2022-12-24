#ifndef __UART_H
#define __UART_H

#include "../inc/defs.h"

void uart_init();
void uart_send(uint8_t * data, uint8_t size);

#endif