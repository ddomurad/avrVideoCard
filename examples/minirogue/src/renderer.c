#include "../inc/renderer.h"
#include "../inc/gfx.h"
#include "../inc/game_tools.h"
#include "../inc/dbg.h"

#define UI_H_BORDER_GLYPH 0xb0
#define UI_V_BORDER_GLYPH 0xb1
#define UI_TL_CORNER_GLYPH 0xb2
#define UI_TR_CORNER_GLYPH 0xb3
#define UI_BL_CORNER_GLYPH 0xb4
#define UI_BR_CORNER_GLYPH 0xb5

#define MAP_EMPTY_FLOOR 0x00
#define MAP_WALL1 0x80
#define MAP_WALL2 0x81

#define MAP_GRASS1 0x8d
#define MAP_GRASS2 0x8e
#define MAP_GRASS3 0x8f

#define MAP_DOOR_CLOSED 0x8c
#define MAP_DOOR_OPEND 0x8b
#define MAP_STAIRS_DOWN 0x89
#define MAP_KEY 0xa0

#define MAP_HERO 0x90

int8_t _camera_x = 0;
int8_t _camera_y = 0;

int8_t _current_tile_x = 0;
int8_t _current_tile_y = 0;


void _clear_tile_pos()
{
    _current_tile_x = -1;
    _current_tile_y = -1;
}

void _update_position(int8_t x, int8_t y)
{
    // if (x >= GFX_SCREEN_WIDTH)
    //     return;
    // if (y >= GFX_SCREEN_HEIGHT)
    //     return;

    // if(_current_tile_x == -1)
    // {
    //     gfx_set_glyph_addr(x, y);
    //     _current_tile_x = x;
    //     _current_tile_y = y;
    //     return;
    // }

    // int8_t dx = x - _current_tile_x;
    // int8_t dy = y - _current_tile_y;
    
    // if(dy == 0 && dx > 0)
    // {
    //     gfx_move_ptr(dx);
    // }
    // else if (dy > 0 && dy < 2)
    // {
    //     uint8_t ptr_move = dx + dy*GFX_SCREEN_WIDTH;
    //     gfx_move_ptr(ptr_move);
    // }
    
    // else
    // {
    //     gfx_set_glyph_addr(x, y);
    // }
    // _current_tile_x = x;
    // _current_tile_y = y;
    
    gfx_set_glyph_addr(x, y);
    _current_tile_x = x;
    _current_tile_y = y;
}

void _draw_tile(int8_t x, int8_t y, uint8_t t)
{
    if (_current_tile_x != x || _current_tile_y != y)
        _update_position(x, y);

    gfx_put(t);

    _current_tile_x++;
    if (_current_tile_x >= 62)
    {
        _current_tile_x = 0;
        _current_tile_y++;
    }
}

uint8_t _is_void(int8_t wx, int8_t wy, game_state *state)
{
    uint8_t has_floor_on_second_line_down = gt_is_floor(wx, wy + 2, state);
    uint8_t has_floor_on_second_line_up = gt_is_floor(wx, wy - 2, state);

    uint8_t has_any_adjacent_floors = gt_is_floor(wx - 1, wy, state);
    has_any_adjacent_floors |= gt_is_floor(wx + 1, wy, state);
    has_any_adjacent_floors |= gt_is_floor(wx, wy - 1, state);
    has_any_adjacent_floors |= gt_is_floor(wx, wy + 1, state);
    has_any_adjacent_floors |= gt_is_floor(wx - 1, wy - 1, state);
    has_any_adjacent_floors |= gt_is_floor(wx - 1, wy + 1, state);
    has_any_adjacent_floors |= gt_is_floor(wx + 1, wy - 1, state);
    has_any_adjacent_floors |= gt_is_floor(wx + 1, wy + 1, state);

    if (!has_any_adjacent_floors &
        !has_floor_on_second_line_down &
        !has_floor_on_second_line_up &
        !gt_is_floor(wx - 1, wy - 2, state) &
        !gt_is_floor(wx + 1, wy - 2, state) &
        !gt_is_floor(wx + 1, wy + 2, state) &
        !gt_is_floor(wx - 1, wy + 2, state))
        return 1;

    return 0;
}

void _render_wall1(uint8_t x, uint8_t y, int8_t wx, int8_t wy, game_state *state)
{
    if (_is_void(wx, wy + 1, state))
    {
        _draw_tile(x, y, MAP_WALL2);
    }
    else if (gt_is_wall(wx, wy, state) && gt_is_floor(wx, wy + 1, state))
    {
        _draw_tile(x, y, MAP_WALL2);
    }
}

void _render_wall2(uint8_t x, uint8_t y, int8_t wx, int8_t wy, game_state *state)
{
    if (_is_void(wx, wy + 1, state))
        return;

    if (gt_is_wall(wx, wy, state) && gt_is_wall(wx, wy + 1, state))
    {
        _draw_tile(x, y, MAP_WALL1);
    }
}

void _render_tile(uint8_t x, uint8_t y, int8_t wx, int8_t wy, game_state *state)
{
    if (_is_void(wx, wy, state))
        return;

    _render_wall1(x, y, wx, wy, state);
    _render_wall2(x, y, wx, wy, state);

    if (gt_is_floor(wx, wy, state))
    {
        uint8_t features = gt_get_features(wx, wy, state);
        item *item = gt_get_item(wx, wy, state);

        if (item != 0)
        {
            _draw_tile(x, y, MAP_KEY);
        }
        else if (features &= ROOM_FEATURE_GRASS)
        {
            // uint32_t seed = TCNT1; seed += cx; seed += cy; //TODO fix
            uint32_t seed = wx + wy;
            uint8_t tile = MAP_GRASS1 + utils_urand_ext(0, 2, seed);
            _draw_tile(x, y, tile);
        }
        else
        {
            _draw_tile(x, y, MAP_EMPTY_FLOOR);
        }
        // room * r = gt_get_room(cx, cy, state);
        // if(r == 0)
        // {
        //     _draw_tile(x, y, MAP_EMPTY_FLOOR);
        // }
        // else
        // {
        //     _draw_tile(x, y, 'A' + r->connection);
        // }
    }
}

void _render_doors(uint8_t x, uint8_t y, int8_t wx, int8_t wy, game_state *state)
{
    door *d = gt_get_door(wx, wy, state);

    if (d == 0)
        return;

    if (d->features & DOOR_FEATURE_STAIRS_DOWN)
    {
        _draw_tile(x, y, MAP_STAIRS_DOWN);
    }
    else if (d->features & DOOR_FEATURE_LOCKED)
    {
        _draw_tile(x, y, MAP_DOOR_OPEND);
    }
    else
    {
        _draw_tile(x, y, MAP_DOOR_CLOSED);
    }
}

void render_set_camera(int8_t x, int8_t y) // todo rename to reset_camera
{
    _camera_x = x;
    _camera_y = y;
}

void _render_game_state_at(uint8_t x, uint8_t y, game_state *state)
{
    int8_t wx = x - CAMERA_CENTER_X + state->player.x;
    int8_t wy = y - CAMERA_CENTER_Y + state->player.y;

    _render_tile(x, y, wx, wy, state);
    _render_doors(x, y, wx, wy, state);
}

void render_game_state(urect8 *area, game_state *state)
{
    _clear_tile_pos();

    for (uint8_t iy = area->y; iy < area->y + area->h; iy++)
    {
        for (uint8_t ix = area->x; ix < area->x + area->w; ix++)
        {
            _render_game_state_at(ix, iy, state);
        }
    }
}

void render_hero(int8_t dx, int8_t dy, game_state *state)
{
    _clear_tile_pos();
    _render_game_state_at(CAMERA_CENTER_X - dx, CAMERA_CENTER_Y - dy, state);
    _draw_tile(CAMERA_CENTER_X, CAMERA_CENTER_Y, MAP_HERO);
}

void render_updated_game(urect8 *area, game_state *state)
{
    _clear_tile_pos();

    int8_t dx = (int8_t)state->player.x - (int8_t)_camera_x;
    int8_t dy = (int8_t)state->player.y - (int8_t)_camera_y;

    if (_camera_x != state->player.x || _camera_y != state->player.y)
    {
        gfx_setup_ext_cmd(area->x, area->y, area->w, area->h, 0x00);
    }

    while (_camera_x != state->player.x || _camera_y != state->player.y)
    {
        if (_camera_x > state->player.x)
        {
            gfx_push_scr(GFX_PUSH_DIR_RIGHT);
            _camera_x--;
        }
        if (_camera_x < state->player.x)
        {
            gfx_push_scr(GFX_PUSH_DIR_LEFT);
            _camera_x++;
        }

        if (_camera_y > state->player.y)
        {
            gfx_push_scr(GFX_PUSH_DIR_DOWN);
            _camera_y--;
        }
        if (_camera_y < state->player.y)
        {
            gfx_push_scr(GFX_PUSH_DIR_UP);
            _camera_y++;
        }
    }

    urect8 parial_rect;
    if (dy > 0)
    {
        parial_rect.x = area->x;
        parial_rect.w = area->w;
        parial_rect.y = area->y + area->h - dy;
        parial_rect.h = dy;
        render_game_state(&parial_rect, state);
    }
    else if (dy < 0)
    {
        parial_rect.x = area->x;
        parial_rect.w = area->w;
        parial_rect.y = area->y;
        parial_rect.h = -dy;
        render_game_state(&parial_rect, state);
    }

    if (dx > 0)
    {
        parial_rect.x = area->x + area->w - dx;
        parial_rect.w = dx;
        parial_rect.y = area->y;
        parial_rect.h = area->h;
        render_game_state(&parial_rect, state);
    }
    else if (dx < 0)
    {
        parial_rect.x = area->x;
        parial_rect.w = -dx;
        parial_rect.y = area->y;
        parial_rect.h = area->h;
        render_game_state(&parial_rect, state);
    }

    if (dx != 0 || dy != 0)
        render_hero(dx, dy, state);
}

void render_hero_adj_area(game_state *state)
{
    _render_game_state_at(CAMERA_CENTER_X - 1, CAMERA_CENTER_Y - 1, state);
    _render_game_state_at(CAMERA_CENTER_X, CAMERA_CENTER_Y - 1, state);
    _render_game_state_at(CAMERA_CENTER_X + 1, CAMERA_CENTER_Y - 1, state);
    _render_game_state_at(CAMERA_CENTER_X - 1, CAMERA_CENTER_Y, state);
    _render_game_state_at(CAMERA_CENTER_X + 1, CAMERA_CENTER_Y, state);
    _render_game_state_at(CAMERA_CENTER_X - 1, CAMERA_CENTER_Y + 1, state);
    _render_game_state_at(CAMERA_CENTER_X, CAMERA_CENTER_Y + 1, state);
    _render_game_state_at(CAMERA_CENTER_X + 1, CAMERA_CENTER_Y + 1, state);
}

void render_ui()
{
    _clear_tile_pos();

    // left top corrner
    _draw_tile(0, 0, UI_TL_CORNER_GLYPH);
    // top border
    gfx_fill(1, 0, GFX_SCREEN_WIDTH - 2, 1, UI_H_BORDER_GLYPH);
    // right top corrner
    _draw_tile(GFX_SCREEN_WIDTH - 1, 0, UI_TR_CORNER_GLYPH);
    // left border
    gfx_fill(0, 1, 1, GFX_SCREEN_HEIGHT - 2, UI_V_BORDER_GLYPH);
    // right border
    gfx_fill(GFX_SCREEN_WIDTH - 1, 1, 1, GFX_SCREEN_HEIGHT - 2, UI_V_BORDER_GLYPH);
    // bottom left corner
    _draw_tile(0, GFX_SCREEN_HEIGHT - 1, UI_BL_CORNER_GLYPH);
    // right border
    gfx_fill(1, GFX_SCREEN_HEIGHT - 1, GFX_SCREEN_WIDTH - 2, 1, UI_H_BORDER_GLYPH);
    // bottom right corner
    _draw_tile(GFX_SCREEN_WIDTH - 1, GFX_SCREEN_HEIGHT - 1, UI_BR_CORNER_GLYPH);
}