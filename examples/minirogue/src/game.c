#include "../inc/game.h"
#include "../inc/utils.h"
#include "../inc/renderer.h"
#include "../inc/dbg.h"
#include "../inc/game_tools.h"

#define SHOULD_DIVIDE_VERTICAL(x) (x & 0x01)
#define SHOULD_TAKE_SECOND_AREA(x) (x & 0x02)

game_state g_state;

void _divide_area(urect8 *area, uint8_t divider)
{
    if (SHOULD_DIVIDE_VERTICAL(divider))
    {
        area->h = area->h >> 1;
        if (SHOULD_TAKE_SECOND_AREA(divider))
        {
            area->y += area->h;
        }
    }
    else
    {
        area->w = area->w >> 1;
        if (SHOULD_TAKE_SECOND_AREA(divider))
        {
            area->x += area->w;
        }
    }
}

uint8_t _find_area_for_room(urect8 *area, room *r, uint8_t divider)
{
    uint8_t iter = 0;
    urect8 last = *area;
    uint8_t final_divider = 0x00;
    while (iter < 4)
    {
        last = *area;
        _divide_area(area, divider);

        if (area->w < r->shape.w || area->h < r->shape.h)
        {
            *area = last;
            return final_divider;
        }

        final_divider |= (divider & 0x03) << (iter * 2);
        divider = divider >> 2;
        iter++;
    }

    return final_divider;
}

uint8_t _fit_room(urect8 area, room *r, uint8_t divider, uint32_t *seed)
{
    divider = _find_area_for_room(&area, r, divider);
    r->shape.x = utils_urand_ext_ptr(area.x, area.x + area.w - r->shape.w, seed);
    r->shape.y = utils_urand_ext_ptr(area.y, area.y + area.h - r->shape.h, seed);
    return divider;
}

void _generate_rooms(uint32_t *rnd_seed)
{
    g_state.level.rooms_cnt = utils_urand_ext_ptr(
        MIN_ROOMS_PER_LEVEL,
        MAX_ROOMS_PER_LEVEL,
        rnd_seed);

    urect8 whole_map = {0, 0, LEVEL_WIDTH, LEVEL_HEIGHT};

    for (uint8_t i = 0; i < g_state.level.rooms_cnt; i++)
    {
        room *r = &g_state.level.rooms[i];
        r->shape.w = utils_urand_ext_ptr(3, 15, rnd_seed);
        r->shape.h = utils_urand_ext_ptr(3, 15, rnd_seed);
        r->features = utils_urand(*rnd_seed);
        r->connection = 0xff;
        r->features = 0;
        if (utils_urand_ext_ptr(0, 100, rnd_seed) > 70)
        {
            r->features = ROOM_FEATURE_GRASS;
        }

        uint8_t random_divider = *rnd_seed;
        _fit_room(whole_map, r, random_divider, rnd_seed);
    }
}

void _generate_corridors(uint32_t *rnd_seed)
{
    g_state.level.corridors_cnt = utils_urand_ext_ptr(
        g_state.level.rooms_cnt / 2,
        g_state.level.rooms_cnt,
        rnd_seed);

    for (uint8_t ci = 0; ci < g_state.level.corridors_cnt; ci++)
    {
        uint8_t r1 = utils_urand_ext_ptr(0, g_state.level.rooms_cnt, rnd_seed);
        uint8_t r2 = utils_urand_ext_ptr(0, g_state.level.rooms_cnt - 1, rnd_seed);
        uint8_t type = utils_urand_ext_ptr(0, 100, rnd_seed);

        if (type > 50)
            type = CORRITOR_TYPE_HORIZONTAL_FIRST;
        else
            type = CORRITOR_TYPE_VERTICAL_FIRST;

        if (r1 == r2)
            r2 += 1;

        room *room1 = &g_state.level.rooms[r1];
        room *room2 = &g_state.level.rooms[r2];

        g_state.level.corridors[ci].type = type;
        uint8_t cx = utils_urand_ext_ptr(
            room1->shape.x + 1,
            room1->shape.x + room1->shape.w - 1,
            rnd_seed);

        uint8_t cy = utils_urand_ext_ptr(
            room1->shape.y + 1,
            room1->shape.y + room1->shape.h - 1,
            rnd_seed);

        g_state.level.corridors[ci].shape.x = cx;
        g_state.level.corridors[ci].shape.y = cy;
        g_state.level.corridors[ci].shape.w = room2->shape.x - cx + 1;
        g_state.level.corridors[ci].shape.h = room2->shape.y - cy + 1;
    }
}

void _generate_doors_for_room(room *r, uint8_t ensure_features)
{
    rect8 s = r->shape;
    for (uint8_t ix = s.x; ix < s.x + s.w; ix++)
    {
        if (g_state.level.doors_cnt >= MAX_DOORS_PER_LEVEL)
            return;

        if (gt_is_passage(ix, s.y - 1, &g_state))
        {
            g_state.level.doors[g_state.level.doors_cnt].x = ix;
            g_state.level.doors[g_state.level.doors_cnt].y = s.y - 1;
            g_state.level.doors[g_state.level.doors_cnt].features = ensure_features;
            g_state.level.doors_cnt++;
        }

        if (g_state.level.doors_cnt >= MAX_DOORS_PER_LEVEL)
            return;

        if (gt_is_passage(ix, s.y + s.h, &g_state))
        {
            g_state.level.doors[g_state.level.doors_cnt].x = ix;
            g_state.level.doors[g_state.level.doors_cnt].y = s.y + s.h;
            g_state.level.doors[g_state.level.doors_cnt].features = ensure_features;
            g_state.level.doors_cnt++;
        }
    }

    for (uint8_t iy = s.y; iy < s.y + s.h; iy++)
    {
        if (g_state.level.doors_cnt >= MAX_DOORS_PER_LEVEL)
            return;

        if (gt_is_passage(s.x - 1, iy, &g_state))
        {
            g_state.level.doors[g_state.level.doors_cnt].x = s.x - 1;
            g_state.level.doors[g_state.level.doors_cnt].y = iy;
            g_state.level.doors[g_state.level.doors_cnt].features = ensure_features;
            g_state.level.doors_cnt++;
        }

        if (g_state.level.doors_cnt >= MAX_DOORS_PER_LEVEL)
            return;

        if (gt_is_passage(s.x + s.w, iy, &g_state))
        {
            g_state.level.doors[g_state.level.doors_cnt].x = s.x + s.w;
            g_state.level.doors[g_state.level.doors_cnt].y = iy;
            g_state.level.doors[g_state.level.doors_cnt].features = ensure_features;
            g_state.level.doors_cnt++;
        }
    }
}

void _generate_doors()
{
    for (uint8_t ri = 0; ri < g_state.level.rooms_cnt; ri++)
    {
        if (g_state.level.doors_cnt >= MAX_DOORS_PER_LEVEL)
            return;

        // locked rooms already have doors
        if (g_state.level.rooms[ri].features & ROOM_FEATURE_LOCKED)
            continue;
        _generate_doors_for_room(&g_state.level.rooms[ri], 0x00);
    }
}

void _generate_stairs(uint32_t *rnd_seed)
{
    // find first locked room, and place saircase
    for (uint8_t ri = g_state.level.rooms_cnt - 1; ri >= 1; ri--)
    {
        room *r = &g_state.level.rooms[ri];

        if (r->features & ROOM_FEATURE_LOCKED)
        {
            r->features |= ROOM_FEATURE_STAIRCASE;

            uint8_t x = utils_urand_ext_ptr(r->shape.x, r->shape.x + r->shape.w, rnd_seed);
            uint8_t y = utils_urand_ext_ptr(r->shape.y, r->shape.y + r->shape.h, rnd_seed);

            g_state.level.doors[g_state.level.doors_cnt].x = x;
            g_state.level.doors[g_state.level.doors_cnt].y = y;
            g_state.level.doors[g_state.level.doors_cnt].features = DOOR_FEATURE_STAIRS_DOWN;

            g_state.level.doors_cnt++;
            return;
        }
    }
}

uint8_t _find_locked_rooms()
{
    uint8_t cnt = 0;
    for (uint8_t ri = g_state.level.rooms_cnt - 1; ri > 0; ri--)
    {
        room *r = &g_state.level.rooms[ri];
        uint8_t result = gt_room_has_one_passage_only(r, &g_state);

        if (result)
        {
            r->features |= ROOM_FEATURE_LOCKED;
            _generate_doors_for_room(r, DOOR_FEATURE_LOCKED);
            cnt++;
        }

        if (cnt >= 2)
            return cnt;
    }

    return cnt;
}

void _find_hidden_rooms()
{
    for (uint8_t ri = 0; ri < g_state.level.rooms_cnt; ri++)
    {
        if (!gt_room_has_any_passage(&g_state.level.rooms[ri], &g_state))
        {
            g_state.level.rooms[ri].features |= ROOM_FEATURE_HIDDEN;
        }
    }
    // ROOM_FEATURE_HIDDEN
}

uint8_t _add_item(int8_t x, int8_t y, uint8_t type)
{
    if (g_state.level.items_cnt > MAX_ITEMS_PER_LEVEL)
        return 0;

    g_state.level.items[g_state.level.items_cnt].x = x;
    g_state.level.items[g_state.level.items_cnt].y = y;
    g_state.level.items[g_state.level.items_cnt].type = type;
    g_state.level.items_cnt++;
    return 1;
}

uint8_t _generate_keys(uint32_t *rnd_seed)
{
    int8_t key_count = 0;
    for (uint8_t ri = g_state.level.rooms_cnt - 1; ri > 0; ri--)
    {
        if (g_state.level.rooms[ri].features & ROOM_FEATURE_LOCKED)
            key_count++;
    }

    for (uint8_t ri = g_state.level.rooms_cnt - 1; ri > 0; ri--)
    {
        if (key_count <= 0)
            return 1;

        if (g_state.level.rooms[ri].features & ROOM_FEATURE_LOCKED)
            continue;

        if (g_state.level.rooms[ri].features & ROOM_FEATURE_HIDDEN)
            continue;

        rect8 *s = &g_state.level.rooms[ri].shape;
        int8_t kx = utils_urand_ext_ptr(s->x, s->x + s->w, rnd_seed);
        int8_t ky = utils_urand_ext_ptr(s->y, s->y + s->h, rnd_seed);

        if (_add_item(kx, ky, ITEM_KEY) == 0)
            return 0;
        key_count--;
    }
    return 0;
}

uint8_t _init_level(uint8_t level)
{
    uint32_t rnd_seed = g_state.prnd_seed + level + 0x1312d3b;

    g_state.player.x = 0;
    g_state.player.y = 0;

    g_state.level.rooms_cnt = 0;
    g_state.level.corridors_cnt = 0;
    g_state.level.doors_cnt = 0;
    g_state.level.items_cnt = 0;

    _generate_rooms(&rnd_seed);
    _generate_corridors(&rnd_seed);
    if (_find_locked_rooms() == 0)
    {
        return 0;
    }
    _generate_stairs(&rnd_seed);
    _generate_doors();
    _find_hidden_rooms();
    if (_generate_keys(&rnd_seed) == 0)
    {
        return 0;
    }

    return 1;
}

void _update_adj_items()
{
    uint8_t adj_i = 0;
    uint8_t item_index = 0;
    // item *adj_item;

    while (1)
    {
        item_index = gt_get_next_adj_item_index(item_index, &g_state);
        if (item_index == 0xff)
            break;

        g_state.adj_items[adj_i] = item_index;
        adj_i++;
        item_index++;
    }

    for (uint8_t i = adj_i; i < 9; i++)
    {
        g_state.adj_items[i] = -1;
    }

    // for (uint8_t i = 0; i < 9; i++)
    // {
    //     printf("adj %d: %d\n", i, g_state.adj_items[i]);
    // }
}

void _remove_item_from_map(uint8_t item_index)
{
    g_state.level.items_cnt--;
    for(uint8_t i=item_index; i < g_state.level.items_cnt;i++)
    {
        g_state.level.items[i].x = g_state.level.items[i+1].x;
        g_state.level.items[i].y = g_state.level.items[i+1].y;
        g_state.level.items[i].type = g_state.level.items[i+1].type;
    }
}

inventory_slot *_find_free_inventory_slot()
{
    for(uint8_t i=0;i<MAX_INVENTORY_SLOTS;i++)
    {
        if(g_state.player.items[i].state == IS_FREE)
            return  &g_state.player.items[i];
    }

    return 0;
}

void _take_adj_item(uint8_t adj_index)
{
    inventory_slot *free_slot = _find_free_inventory_slot();
    if(free_slot == 0)
    {
        // todo ui message !
        return;
    }

    uint8_t item_index = g_state.adj_items[adj_index];
    item *item = &g_state.level.items[item_index];
    free_slot->state = IS_USED;
    free_slot->type = item->type;

    _remove_item_from_map(item_index);
}

void game_create_new()
{
    while (1)
    {
        g_state.prnd_seed = utils_urand_tim();
#ifdef USE_SIMULATOR
        printf("Game see: %u\n", g_state.prnd_seed);
#endif

        if (_init_level(1) == 0)
        {
#ifdef USE_SIMULATOR
            printf("Level initialization fialed....\n");
#endif
            continue;
        }

        if (gt_ghost_walker_test(&g_state) == 0)
        {
#ifdef USE_SIMULATOR
            printf("Ghost walker test failed....\n");
#endif
            continue;
        }

        break;
    }

    g_state.player.x = g_state.level.rooms[0].shape.x;
    g_state.player.y = g_state.level.rooms[0].shape.y;
}

game_state *game_get_state()
{
    return &g_state;
}

void game_update_state()
{
    _update_adj_items();
}

void game_take_item(uint8_t adj_index, uint8_t update_game)
{
    if(g_state.adj_items[adj_index] != 0xff)
    {
        _take_adj_item(g_state.adj_items[adj_index]);
        // g_state.adj_items[adj_index] = 0xff;
    }

    if(update_game)
        game_update_state();
}

void game_move_player(int8_t dx, int8_t dy, uint8_t update_game)
{
    if (gt_is_wall(g_state.player.x + dx, g_state.player.y + dy, &g_state))
        return;

    g_state.player.x += dx;
    g_state.player.y += dy;

    if (g_state.player.x < 0)
        g_state.player.x = 0;

    if (g_state.player.y < 0)
        g_state.player.y = 0;

    if (g_state.player.x > LEVEL_WIDTH)
        g_state.player.x = LEVEL_WIDTH;

    if (g_state.player.y > LEVEL_HEIGHT)
        g_state.player.y = LEVEL_HEIGHT;

    if(update_game)
        game_update_state();
}
