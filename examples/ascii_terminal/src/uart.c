#include "../inc/uart.h"

#define BAUD 9600
#define UBRR (F_CPU/16/BAUD-1)

void uart_init()
{
    UBRR0H = (uint8_t)(UBRR >> 8);
    UBRR0L = (uint8_t)UBRR;

    UCSR0B = (1<<RXEN0)|(1<<TXEN0);
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void uart_send(uint8_t *data, uint8_t size)
{
    /* Wait for empty transmit buffer */
    
    /* Put data into buffer, sends the data */
    while(*data != 0x00 && size > 0){
        while (!(UCSR0A & (1<<UDRE0)));
        UDR0 = *data;

        data++;
        size--;
    }
}