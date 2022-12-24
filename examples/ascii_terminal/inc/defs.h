#ifndef __DEFS_H
#define __DEFS_H

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>

#define __ENFORCE_COMMA(__EXPR) do{__EXPR}while(0)

// SPI definitions
#define SPI_DDR DDRB
#define SPI_PORT PORTB
#define SPI_SS_PIN PINB2
#define SPI_MOSI_PIN PINB3
#define SPI_MISO_PIN PINB4
#define SPI_CLK_PIN PINB5

//video card select pin
#define SPI_VC_SELECT_DDR DDRD
#define SPI_VC_SELECT_PORT PORTD
#define SPI_VC_SELECT_PIN PIND7

#define SPI_TRANSFER_DELAY_US 46
#define SPI_EXTENDED_CMD_DELAY_MS 46

//PS2 keyboard
#define PS2_DDR       DDRD
#define PS2_PORT      PORTD
#define PS2_PIN       PIND
#define PS2_CLK_PIN   PIND2
#define PS2_DATA_PIN  PIND4

//Timer ticks ovf limit
#define TIMER_CLK_OVF   156  // (16Mhz/1024)/100 -> ~10ms per tick

#endif