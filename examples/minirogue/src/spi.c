#include "../inc/spi.h"

#ifndef USE_SIMULATOR

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

#else

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define PORT 10601

int server_sockfd;

void spi_init()
{

    struct sockaddr_in servaddr;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (connect(server_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        printf("connection failed...\n");
        exit(0);
    }
    else
    {
        printf("connected to the simulator server..\n");
    }
}

uint8_t spi_transfer(uint8_t b)
{
    write(server_sockfd, &b, 1);
}

void spi_send_cmd(uint8_t c, uint8_t d)
{
    spi_transfer(c);
    spi_transfer(d);
    // printf("sim send cmd: %u %u\n", c, d);
}

#endif