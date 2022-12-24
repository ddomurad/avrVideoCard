#ifndef __GAME_TOOLS_H
#define __GAME_TOOLS_H

#include "defs.h"
#include "game.h"

// All functions take world coordnates
// world_x = sreen_x - camera_x + player_x
// world_y = sreen_y - camera_y + player_y

uint8_t gt_is_floor(int8_t wx, int8_t wy, game_state * state);
uint8_t gt_is_wall(int8_t wx, int8_t wy, game_state * state);
uint8_t gt_get_features(int8_t wx, int8_t wy, game_state *state);
uint8_t gt_is_passage(int8_t wx, int8_t wy, game_state *state);
uint8_t gt_room_has_one_passage_only(room *r, game_state *state);
uint8_t gt_room_has_any_passage(room *r, game_state *state);

door *gt_get_door(int8_t wx, int8_t wy, game_state *state);
room *gt_get_room(int8_t wx, int8_t wy, game_state *state);
item *gt_get_item(int8_t wx, int8_t wy, game_state *state);
uint8_t gt_get_next_adj_item_index(uint8_t start_index, game_state *state);
// used for testing the generated level
// returns 0x01 if the level passed the test, 0x00 if not
uint8_t gt_ghost_walker_test(game_state *state);
#endif