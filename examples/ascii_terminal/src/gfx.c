#include "../inc/gfx.h"
#include "../inc/spi.h"


void gfx_init()
{
    spi_init();
}

void gfx_set_addr(uint16_t addr)
{
    uint8_t addr_l = addr & 0x00ff;
    uint8_t addr_h = addr >> 8;

    spi_send_cmd(GFX_CMD_SET_ADDR_LOW, addr_l);
    spi_send_cmd(GFX_CMD_SET_ADDR_HIGH, addr_h);
}

void gfx_set_glyph_addr(uint8_t x, uint8_t y)
{
    uint16_t addr = GFX_GLYPHS_ADDR + x + y*GFX_SCREEN_WIDTH;
    gfx_set_addr(addr);
}

void gfx_ext_cmd(uint8_t cmd)
{
    spi_send_cmd(GFX_CMD_EXT, cmd);
    _delay_ms(SPI_EXTENDED_CMD_DELAY_MS);
}

void gfx_put_ext(uint8_t d, int8_t dir)
{
    if(dir < 0) 
    {
        spi_send_cmd(GFX_CMD_PUT_SUB, d);
        return;
    }
    
    if(dir == 0) 
    {
        spi_send_cmd(GFX_CMD_PUT, d);
        return;
    }

    if(dir > 0) 
    {
        spi_send_cmd(GFX_CMD_PUT_ADD, d);
        return;
    }
}

void gfx_put(uint8_t d)
{
    spi_send_cmd(GFX_CMD_PUT_ADD, d);
}

void gfx_put_ntimes(uint8_t d, uint16_t n)
{
    for(; n > 0; n--) 
    {
        spi_send_cmd(GFX_CMD_PUT_ADD, d); 
    }
}

void gfx_put_ntimes_ext(uint8_t d, uint16_t n, uint8_t offset)
{
    for(; n > 0; n--) 
    {
        spi_send_cmd(GFX_CMD_PUT, d);
        spi_send_cmd(GFX_CMD_MOV_PTR, offset);
    }
}

void gfx_put_string(const char *str)
{
    for(;*str!=0;str++) 
    {
        spi_send_cmd(GFX_CMD_PUT_ADD, *str);
    }
}

void gfx_put_string_ext(const char *str, uint16_t n)
{
    uint16_t i;
    for(i=0; i<=n && *str !=0; i++) 
    {
        spi_send_cmd(GFX_CMD_PUT_ADD, *str);
        str++;
    }
}

void gfx_move_ptr(uint8_t m)
{
    spi_send_cmd(GFX_CMD_MOV_PTR, m);
}

void gfx_setup_ext_cmd(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t d)
{
    gfx_set_addr(GFX_EXTCMD_PARAMS_ADDR);
    gfx_put(x);
    gfx_put(y);
    gfx_put(w);
    gfx_put(h);
    gfx_put(d);
}

void gfx_fill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t d)
{
    gfx_setup_ext_cmd(x, y, w, h, d);
    gfx_ext_cmd(GFX_EXTCMD_CLEAR);
}

void gfx_fill_inv(uint8_t d)
{
    gfx_set_addr(GFX_INVS_ADDR);
    gfx_put_ntimes(d, 58*8);
}

void gfx_clear_src(uint8_t d)
{
    gfx_fill(0, 0, 62, 58, d);
}

void gfx_clear_inv()
{
    gfx_fill_inv(0x00);
}

void gfx_set_color(uint8_t c)
{
    gfx_set_addr(GFX_TEXT_COLOR_ADDR);
    gfx_put(c);
}

void gfx_push_scr_ext(uint8_t dir, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t d)
{
    gfx_setup_ext_cmd(x, y, w, h, d);
    gfx_push_scr(dir);
}

void gfx_push_scr(uint8_t dir)
{
    if(dir & GFX_PUSH_DIR_UP) {
        gfx_ext_cmd(GFX_EXTCMD_PUSH_UP);
    }
    
    if(dir & GFX_PUSH_DIR_DOWN) {
        gfx_ext_cmd(GFX_EXTCMD_PUSH_DOWN);
    }

    if(dir & GFX_PUSH_DIR_LEFT) {
        gfx_ext_cmd(GFX_EXTCMD_PUSH_LEFT);
    }

    if(dir & GFX_PUSH_DIR_RIGHT) {
        gfx_ext_cmd(GFX_EXTCMD_PUSH_RIGHT);
    }
}

void gfx_set_glyphs(uint8_t g)
{
    gfx_set_addr(GFX_GLYPH_SET_ADDR);
    gfx_put(g);
}

void gfx_set_inv(uint8_t x8, uint8_t y, uint8_t d)
{
    uint8_t ptr = y*8 + x8;
    gfx_set_addr(GFX_INVS_ADDR + ptr);
    gfx_put_ext(d, 0);
}