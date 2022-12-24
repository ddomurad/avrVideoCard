#include "../inc/defs.h"
#include "../inc/gfx.h"
#include "../inc/ctrl.h"
#include "../inc/utils.h"
#include "../inc/game.h"
#include "../inc/renderer.h"
#include "../inc/timer.h"
#include "../inc/dbg.h"

#include <stdio.h>

int main(void)
{
    gfx_init();
    ctrl_init();
    timer_init();

#ifndef USE_SIMULATOR
    sei();
    gfx_clear_inv();
#endif

    urect8 map_area = {1, 1, 60, 56};

    gfx_clear_src(0x00);
    gfx_set_color(0x07);
    gfx_set_glyphs(GFX_DEFAULT_GLYPH_SET);
    gfx_set_addr(GFX_GLYPHS_ADDR);

    render_ui();
    gfx_set_glyph_addr(10, 10);
    gfx_put_string("Loading ...");

    game_create_new();

    gfx_fill(1,1,60,56, 0x00);

    gfx_set_glyphs(GFX_DEFAULT_GLYPH_SET);
    gfx_set_addr(GFX_GLYPHS_ADDR);

    
    // urect8 area = {CAMERA_CENTER_X-6, CAMERA_CENTER_Y-6, 12, 12};

    render_set_camera(game_get_state()->player.x, game_get_state()->player.y);
    render_hero(0, 0, game_get_state());
    render_game_state(&map_area, game_get_state());
    render_hero(0, 0, game_get_state());
    

    while (1)
    {
        uint8_t key = ctrl_pop_key();
        if (key == CTRL_UP)
        {
            game_move_player(0, -1, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if (key == CTRL_DOWN)
        {
            game_move_player(0, 1, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if (key == CTRL_LEFT)
        {
            game_move_player(-1, 0, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if (key == CTRL_RIGHT)
        {
            game_move_player(1, 0, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if (key == CTRL_UP_LEFT)
        {
            game_move_player(-1, -1, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if (key == CTRL_UP_RIGHT)
        {
            game_move_player(1, -1, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if (key == CTRL_DOWN_LEFT)
        {
            game_move_player(-1, 1, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if (key == CTRL_DOWN_RIGHT)
        {
            game_move_player(1, 1, 1);
            render_updated_game(&map_area, game_get_state());
        }
        else if(key == CTRL_TAKE_ITEM)
        {
            game_take_item(0, 1);
            render_hero_adj_area(game_get_state());
        }
        else if (key == CTRL_F1)
        {
            gfx_fill_inv(0x00);
        }
        else if (key == CTRL_F2)
        {
            gfx_fill_inv(0xaa);
        }
        else if (key == CTRL_F5)
        {
            gfx_clear_inv();
            gfx_clear_src(0x00);
            gfx_set_color(0x07);
            gfx_set_glyphs(GFX_DEFAULT_GLYPH_SET);
            gfx_set_addr(GFX_GLYPHS_ADDR);

            render_ui();

            game_create_new();

            render_set_camera(game_get_state()->player.x, game_get_state()->player.y);
            render_game_state(&map_area, game_get_state());
            render_hero(0, 0, game_get_state());
            gfx_set_glyph_addr(0, 0);
        }
        else if (key == CTRL_F6)
        {
            gfx_clear_inv();
            gfx_clear_src(0x00);
            gfx_set_color(0x07);
            gfx_set_glyphs(GFX_DEFAULT_GLYPH_SET);
            gfx_set_addr(GFX_GLYPHS_ADDR);

            render_ui();

            render_set_camera(game_get_state()->player.x, game_get_state()->player.y);
            render_game_state(&map_area, game_get_state());
            render_hero(0, 0, game_get_state());
            gfx_set_glyph_addr(0, 0);
        }

        // if(key != 0x00)
        // {
        //     dbg_print(1, 1, "%d", key);
        // }
    }
}