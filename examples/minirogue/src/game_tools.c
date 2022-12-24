#include "../inc/game_tools.h"
#include "../inc/utils.h"

#include "../inc/gfx.h"
#include "../inc/renderer.h"
#include "../inc/timer.h"

uint8_t _is_inside_room(int8_t wx, int8_t wy, room *r)
{
    rect8 s = r->shape;

    if (wx < s.x)
        return 0;
    if (wy < s.y)
        return 0;

    if (wx >= s.x + s.w)
        return 0;
    if (wy >= s.y + s.h)
        return 0;

    return 1;
}

uint8_t _is_inside_corridor(int8_t wx, int8_t wy, corridor *r)
{
    rect8 s = r->shape;
    int8_t x1, x2, y1, y2;
    int8_t ex = 0;
    int8_t ey = 0;

    if (s.w < 0)
    {
        x1 = s.x + s.w + 1;
        x2 = s.x;
        ex = x1;
    }
    else
    {
        x1 = s.x;
        x2 = x1 + s.w - 1;
        ex = x2;
    }

    if (s.h < 0)
    {
        y1 = s.y + s.h + 1;
        y2 = s.y;
        ey = y1;
    }
    else
    {
        y1 = s.y;
        y2 = s.y + s.h - 1;
        ey = y2;
    }

    if (r->type == CORRITOR_TYPE_HORIZONTAL_FIRST)
    {
        if (wy == s.y)
        {
            if (wx >= x1 && wx <= x2)
                return 1;
        }
        if (wx == ex)
        {
            if (wy >= y1 && wy <= y2)
                return 1;
        }
        return 0;
    }
    else
    {
        if (wx == s.x)
        {
            if (wy >= y1 && wy <= y2)
                return 1;
        }
        if (wy == ey)
        {
            if (wx >= x1 && wx <= x2)
                return 1;
        }
        return 0;
    }

    return 0;
}

uint8_t _is_adj(int8_t wx1, int8_t wy1, int8_t wx2, int8_t wy2)
{
    int8_t dx = wx1 - wx2;
    int8_t dy = wy1 - wy2;
    
    if(dx != 0 && dx != -1 && dx != 1)
        return 0;

    if(dy != 0 && dy != -1 && dy != 1)
        return 0;

    return 1;
}

uint8_t gt_is_floor(int8_t wx, int8_t wy, game_state *state)
{
    for (uint8_t i = 0; i < state->level.rooms_cnt; i++)
    {
        if (_is_inside_room(wx, wy, &state->level.rooms[i]))
            return 1;
    }

    for (uint8_t i = 0; i < state->level.corridors_cnt; i++)
    {
        if (_is_inside_corridor(wx, wy, &state->level.corridors[i]))
            return 1;
    }

    return 0;
}

uint8_t gt_is_wall(int8_t wx, int8_t wy, game_state *state)
{
    if (gt_is_floor(wx, wy, state))
        return 0;
    return 1;
}

uint8_t gt_get_features(int8_t wx, int8_t wy, game_state *state)
{
    for (uint8_t i = 0; i < state->level.rooms_cnt; i++)
    {
        if (_is_inside_room(wx, wy, &state->level.rooms[i]))
            return state->level.rooms[i].features;
    }

    return 0;
}

room *gt_get_room(int8_t wx, int8_t wy, game_state *state)
{
    for (uint8_t i = 0; i < state->level.rooms_cnt; i++)
    {
        if (_is_inside_room(wx, wy, &state->level.rooms[i]))
            return &state->level.rooms[i];
    }

    return 0;
}

uint8_t gt_is_passage(int8_t wx, int8_t wy, game_state *state)
{
    if (!gt_is_floor(wx, wy, state))
        return 0;

    if (gt_is_wall(wx + 1, wy, state) && gt_is_wall(wx - 1, wy, state))
    {
        if (gt_is_floor(wx, wy + 1, state) && gt_is_floor(wx, wy - 1, state))
        {
            return 1;
        }
        return 0;
    }

    if (gt_is_floor(wx + 1, wy, state) && gt_is_floor(wx - 1, wy, state))
    {
        if (gt_is_wall(wx, wy + 1, state) && gt_is_wall(wx, wy - 1, state))
        {
            return 1;
        }
    }

    return 0;
}

uint8_t gt_room_has_one_passage_only(room *r, game_state *state)
{
    rect8 s = r->shape;
    uint8_t passage_cnt = 0;
    for (int8_t ix = s.x; ix < s.x + s.w; ix++)
    {
        if (gt_is_passage(ix, s.y - 1, state))
        {
            passage_cnt++;
        }
        else if (!gt_is_wall(ix, s.y - 1, state))
        {
            return 0;
        }

        if (gt_is_passage(ix, s.y + s.h, state))
        {
            passage_cnt++;
        }
        else if (!gt_is_wall(ix, s.y + s.h, state))
        {
            return 0;
        }

        if (passage_cnt > 1)
            return 0;
    }

    for (int8_t iy = s.y; iy < s.y + s.h; iy++)
    {
        if (gt_is_passage(s.x - 1, iy, state))
        {
            passage_cnt++;
        }
        else if (!gt_is_wall(s.x - 1, iy, state))
        {
            return 0;
        }

        if (gt_is_passage(s.x + s.w, iy, state))
        {
            passage_cnt++;
        }
        else if (!gt_is_wall(s.x + s.w, iy, state))
        {
            return 0;
        }

        if (passage_cnt > 1)
            return 0;
    }

    return passage_cnt == 1 ? 1 : 0;
}

uint8_t gt_room_has_any_passage(room *r, game_state *state)
{
    rect8 s = r->shape;
    uint8_t passage_cnt = 0;
    for (int8_t ix = s.x; ix < s.x + s.w; ix++)
    {
        if (gt_is_floor(ix, s.y - 1, state))
        {
            passage_cnt++;
        }
        if (gt_is_floor(ix, s.y + s.h, state))
        {
            passage_cnt++;
        }
    }

    for (int8_t iy = s.y; iy < s.y + s.h; iy++)
    {
        if (gt_is_floor(s.x - 1, iy, state))
        {
            passage_cnt++;
        }
        if (gt_is_floor(s.x + s.w, iy, state))
        {
            passage_cnt++;
        }
    }

    return passage_cnt;
}

door *gt_get_door(int8_t wx, int8_t wy, game_state *state)
{
    for (uint8_t di = 0; di < state->level.doors_cnt; di++)
    {
        if (wx == state->level.doors[di].x && wy == state->level.doors[di].y)
            return &state->level.doors[di];
    }

    return 0;
}

room *_get_staircase_room(game_state *state)
{
    for (uint8_t ri = 0; ri < state->level.rooms_cnt; ri++)
    {
        if (state->level.rooms[ri].features & ROOM_FEATURE_STAIRCASE)
            return &state->level.rooms[ri];
    }

    return 0;
}

item *gt_get_item(int8_t wx, int8_t wy, game_state *state)
{
    for (uint8_t ii = 0; ii < state->level.items_cnt; ii++)
    {
        if (state->level.items[ii].x == wx && state->level.items[ii].y == wy)
            return &state->level.items[ii];
    }

    return 0;
}

uint8_t gt_get_next_adj_item_index(uint8_t start_index, game_state *state)
{
    for (uint8_t i = start_index; i < state->level.items_cnt; i++)
    {
        if(_is_adj(
            state->player.x, 
            state->player.y, 
            state->level.items[i].x,
            state->level.items[i].y))
            {
                return i;
            }
    }

    return -1;
}

void _get_random_direction(int8_t *x, int8_t *y)
{
    uint8_t rdir = utils_urand_tim() % 4;
    if (rdir == 0)
    {
        *x = 1;
        *y = 0;
        return;
    }
    if (rdir == 1)
    {
        *x = -1;
        *y = 0;
        return;
    }
    if (rdir == 2)
    {
        *x = 0;
        *y = 1;
        return;
    }
    if (rdir == 3)
    {
        *x = 0;
        *y = -1;
        return;
    }
}

void _get_random_p_direction(int8_t x, int8_t y, int8_t *px, int8_t *py)
{
    uint8_t rnd = utils_urand_tim() % 2;
    int8_t dir = rnd == 0 ? -1 : 1;
    if (x != 0)
    {
        *px = 0;
        *py = dir;
    }
    else
    {
        *px = dir;
        *py = 0;
    }
}

void _update_room_connection(uint8_t new, game_state *state)
{
    room *room = gt_get_room(state->player.x, state->player.y, state);

    if (room == 0 || room->connection == new)
        return;

    if (room->connection == 0xff)
    {
        room->connection = new;
        return;
    }

    uint8_t old = room->connection;
    if (new < old)
    {
        for (uint8_t i = 0; i < state->level.rooms_cnt; i++)
        {
            if (state->level.rooms[i].connection == old)
                state->level.rooms[i].connection = new;
        }
    }
    else
    {
        for (uint8_t i = 0; i < state->level.rooms_cnt; i++)
        {
            if (state->level.rooms[i].connection == new)
                state->level.rooms[i].connection = old;
        }
    }
}

void gt_ghost_walker_test_2(room *src_room, game_state *state)
{
    // const urect8 area = {1, 1, 60, 56};
    uint16_t moves_left = 1000;

    state->player.x = src_room->shape.x;
    state->player.y = src_room->shape.y;
    player_state *player = &state->player;

    int8_t gdx, gdy, gpx, gpy;
    _get_random_direction(&gdx, &gdy);
    _get_random_p_direction(gdx, gdy, &gpx, &gpy);

    // gfx_fill(1, 1, 60, 56, 0);
    // render_set_camera(state->player.x, state->player.y);
    // render_game_state(&area, state);
    // gfx_set_glyph_addr(CAMERA_CENTER_X, CAMERA_CENTER_Y);
    // gfx_put('x');

    while (moves_left > 0)
    {
        uint8_t wx = player->x + gdx;
        uint8_t wy = player->y + gdy;

        if (gt_is_floor(wx, wy, state))
        {
            state->player.x += gdx;
            state->player.y += gdy;

            _update_room_connection(src_room->connection, state);
        }
        else if (gt_is_floor(wx, wy, state))
        {
            state->player.x += gpx;
            state->player.y += gpy;

            _update_room_connection(src_room->connection, state);
        }
        else
        {
            _get_random_direction(&gdx, &gdy);
            _get_random_p_direction(gdx, gdy, &gpx, &gpy);
        }

        // render_updated_game(&area, state);
        // gfx_set_glyph_addr(CAMERA_CENTER_X, CAMERA_CENTER_Y);
        // gfx_put('x');

        moves_left--;
    }
}

uint8_t gt_ghost_walker_test(game_state *state)
{
    room *sr = _get_staircase_room(state);
    if (sr == 0)
        return 0;

    for (uint8_t i = 0; i < state->level.rooms_cnt; i++)
    {
        room *r1 = &state->level.rooms[i];
        if (r1->connection != 0xff)
            continue;

        r1->connection = i;
        gt_ghost_walker_test_2(r1, state);
    }

    return sr->connection == 0 ? 1 : 0;
}