#ifndef __GAME_H
#define __GAME_H

#include "defs.h"
#include "utils.h"

// static camera position
// hero on screen position will be always equal to the camera position
#define CAMERA_CENTER_X 30
#define CAMERA_CENTER_Y 28

// level map size boundaries
#define LEVEL_WIDTH 80
#define LEVEL_HEIGHT 80

// maimum RAM usage for
#define MIN_ROOMS_PER_LEVEL 12
#define MAX_ROOMS_PER_LEVEL 24
#define MAX_DOORS_PER_LEVEL 24
#define MAX_ITEMS_PER_LEVEL 20
#define MAX_INVENTORY_SLOTS 10

#define MAX_CORRIDORS_PER_LEVEL MAX_ROOMS_PER_LEVEL

// room features flags
#define ROOM_FEATURE_NONE 0x00
#define ROOM_FEATURE_LOCKED 0x01
#define ROOM_FEATURE_STAIRCASE 0x02
#define ROOM_FEATURE_GRASS 0x04
#define ROOM_FEATURE_HIDDEN 0x08

// corridor type flags
#define CORRITOR_TYPE_HORIZONTAL_FIRST 0x00
#define CORRITOR_TYPE_VERTICAL_FIRST 0x01

// door feature flags
#define DOOR_FEATURE_LOCKED 0x01
// #define DOOR_FEATURE_TRAPPED    0x02
#define DOOR_FEATURE_STAIRS_DOWN 0x04

// item types
#define ITEM_KEY    0x01

// inventory slots state
#define IS_FREE     0x00
#define IS_USED     0x01

typedef struct door_t
{
    int8_t x, y;
    uint8_t features;
} door;

typedef struct item_t
{
    int8_t x, y;
    uint8_t type;
} item;

typedef struct inventory_slot_t
{
    uint8_t state;
    uint8_t type;
} inventory_slot;


typedef struct corridor_t
{
    rect8 shape;
    uint8_t type;
} corridor;

typedef struct room_t
{
    rect8 shape;
    uint8_t features;
    uint8_t connection; // used to group rooms that are connected
} room;

typedef struct player_state_t
{
    int8_t x, y;
    inventory_slot items[MAX_INVENTORY_SLOTS];
} player_state;

typedef struct level_state_t
{
    room rooms[MAX_ROOMS_PER_LEVEL];
    uint8_t rooms_cnt;

    corridor corridors[MAX_CORRIDORS_PER_LEVEL];
    uint8_t corridors_cnt;

    door doors[MAX_DOORS_PER_LEVEL];
    uint8_t doors_cnt;

    item items[MAX_ITEMS_PER_LEVEL];
    uint8_t items_cnt;
} level_state;

typedef struct game_state_t
{
    uint32_t prnd_seed;

    player_state player;
    level_state level;

    uint8_t adj_items[8];
} game_state;

void        game_create_new();
game_state* game_get_state();
void        game_update_state();
void        game_take_item(uint8_t adj_index, uint8_t update_game);
void        game_move_player(int8_t dx, int8_t dy, uint8_t update_game);

#endif