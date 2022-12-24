#ifndef __RENDERER_H
#define __RENDERER_H

#include "defs.h"
#include "utils.h"
#include "game.h"

void render_game_state(urect8 *area, game_state *state);
void render_updated_game(urect8 *area, game_state *state);
void render_updated_game(urect8 *area, game_state *state);
void render_set_camera(int8_t x, int8_t y);
void render_hero(int8_t dx, int8_t dy, game_state *state);
void render_hero_adj_area(game_state *state);
void render_ui();

#endif