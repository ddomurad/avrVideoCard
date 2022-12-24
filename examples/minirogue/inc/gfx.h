#ifndef __GFX_H
#define __GFX_H

#include "defs.h"

#define GFX_SCREEN_WIDTH            62
#define GFX_SCREEN_HEIGHT           58

// Video card commands
#define GFX_CMD_NOP                 0x80
#define GFX_CMD_SET_ADDR_LOW        0x01
#define GFX_CMD_SET_ADDR_HIGH       0x02
#define GFX_CMD_PUT_ADD             0x84
#define GFX_CMD_PUT                 0x88
#define GFX_CMD_PUT_SUB             0x90
#define GFX_CMD_EXT                 0x20
#define GFX_CMD_MOV_PTR             0x40

//Video card extended commands  
#define GFX_EXTCMD_PUSH_UP          0x01
#define GFX_EXTCMD_PUSH_DOWN        0x82
#define GFX_EXTCMD_PUSH_LEFT        0x04
#define GFX_EXTCMD_PUSH_RIGHT       0x88
#define GFX_EXTCMD_CLEAR            0x10

//Video card addresses  
#define GFX_GLYPHS_ADDR             0x0100
#define GFX_INVS_ADDR               0x0f0c
#define GFX_GLYPH_SET_ADDR          0x10dc
#define GFX_TEXT_COLOR_ADDR         0x10dd
#define GFX_EXTCMD_PARAMS_ADDR      0x10de

#define GFX_DEFAULT_GLYPH_SET       0x30

#define GFX_PUSH_DIR_UP             0x01
#define GFX_PUSH_DIR_DOWN           0x02
#define GFX_PUSH_DIR_LEFT           0x04
#define GFX_PUSH_DIR_RIGHT          0x08

void gfx_init();

void gfx_set_addr(uint16_t addr);
void gfx_set_glyph_addr(uint8_t x, uint8_t y);

void gfx_ext_cmd(uint8_t cmd);

void gfx_put_ext(uint8_t d, int8_t dir);
void gfx_put(uint8_t d);
void gfx_put_ntimes(uint8_t d, uint16_t n);
void gfx_put_ntimes_ext(uint8_t d, uint16_t n, uint8_t offset);

void gfx_put_string(const char *str);
void gfx_put_string_ext(const char *str, uint16_t n);

void gfx_move_ptr(uint8_t m);

void gfx_setup_ext_cmd(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t d);

void gfx_fill(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t d);
void gfx_fill_inv(uint8_t d);

void gfx_clear_src(uint8_t d);
void gfx_clear_inv();

void gfx_push_scr(uint8_t dir);
void gfx_push_scr_ext(uint8_t dir, uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t d);

void gfx_set_color(uint8_t c);

void gfx_set_glyphs(uint8_t g);
#endif