#include "../inc/spi.h"

void spi_init()
{
    SPI_DDR |= _BV(SPI_SS_PIN);
    SPI_DDR |= _BV(SPI_MOSI_PIN);
    SPI_DDR &= ~_BV(SPI_MISO_PIN);
    SPI_DDR |= _BV(SPI_CLK_PIN);

    SPI_PORT |= _BV(SPI_SS_PIN);

    SPI_VC_SELECT_DDR |= _BV(SPI_VC_SELECT_PIN);
    SPI_VC_SELECT_PORT |= _BV(SPI_VC_SELECT_PIN);

    // enable SPI, set master mode, CPHA=1, SCK = f_osc/16
    SPCR |= _BV(SPE) | _BV(MSTR) | _BV(CPHA) | _BV(SPR0);
}

uint8_t spi_transfer(uint8_t b)
{
    SPDR = b;
    _delay_us(SPI_TRANSFER_DELAY_US);
    return SPDR;
}

void spi_send_cmd(uint8_t c, uint8_t d)
{
    SPI_VC_SELECT_PORT &= ~_BV(SPI_VC_SELECT_PIN);
    spi_transfer(c);
    spi_transfer(d);
    SPI_VC_SELECT_PORT |= _BV(SPI_VC_SELECT_PIN);
}
