#ifdef NOT_DRAGONRUBY
#define DRB_FFI //This will be accessible from DRGTK
#define DRB_FFI_NAME(name) //This will be accessible from DRGTK as `name`
#else
#include <dragonruby.h>
#endif

#define FNL_IMPL
#include "lib/FastNoiseLite.h"
#include <string.h>
#include <math.h>
#include "src/drb_symbols.h"
#include "src/globals.h"
#include "src/render.h"
#include "src/terrain.h"
#include "src/player.h"
#include "src/utils.h"

DRB_FFI_NAME(blank_screen)
int matocore_blank_screen(void) {
    memset(SCREEN, 0, PIXELS * sizeof(Color));
    return 0;
}

DRB_FFI_NAME(draw_terrain)
int matocore_draw_terrain(void) {
    render_terrain();
    return 0;
}

DRB_FFI_NAME(generate_terrain)
int matocore_generate_terrain(int base_seed, int smooth_seed) {
    terrain_generate(base_seed, smooth_seed);
    return 0;
}

DRB_FFI_NAME(update_sand)
int matocore_update_sand(void) {
    terrain_update_sand();
    return 0;
}

DRB_FFI_NAME(destroy_terrain)
int matocore_destroy_terrain(int screen_x, int screen_y, int radius) {
    int      mx = screen_x / ZOOM;
    int      my = HEIGHT - (screen_y / ZOOM);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (sqrt(dx * dx + dy * dy) <= radius) {
                int x = mx + dx;
                int y = my + dy;
                terrain_set_pixel(x, y, TERRAIN_NONE);
            }
        }
    }
    return 0;
}

DRB_FFI_NAME(create_terrain)
int matocore_create_terrain(int screen_x, int screen_y, int radius) {
    int      mx = screen_x / ZOOM;
    int      my = HEIGHT - (screen_y / ZOOM);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (terrain_get_pixel(mx + dx, my + dy, TERRAIN_DIRT).type == TERRAIN_NONE.type && sqrt(dx * dx + dy * dy) <= radius) {
                int x = mx + dx;
                int y = my + dy;
                terrain_set_pixel(x, y, TERRAIN_SAND);
            }
        }
    }
    return 0;
}

DRB_FFI_NAME(spawn_player)
void *matocore_spawn_player(double screen_x, double screen_y, double r, double g, double b) {
    double px      = screen_x / ZOOM;
    double py      = HEIGHT - (screen_y / ZOOM);
    Player *player = player_initialize_player(px, py, fclamp(r, 0, 1), fclamp(g, 0, 1), fclamp(b, 0, 1));
    player->sprite = load_image("/sprites/player2.png", &player->w, &player->h);
    player_do_surface_warp(player);
    return player;
}

DRB_FFI_NAME(despawn_player)
int matocore_despawn_player(void *player) {
    Player* cast_player = player;
    player_destroy_player(&cast_player);
    return 0;
}

DRB_FFI_NAME(draw_player)
int matocore_draw_player(void *player) {
    return render_player((Player*)player);
}

DRB_FFI_NAME(player_input)
int matocore_player_input(void *player, double up_down, double left_right) {
    player_calc_input((Player*)player, up_down, left_right);
    return 0;
}

DRB_FFI_NAME(player_tick)
int matocore_player_tick(void *player) {
    player_do_input((Player*)player);
    player_do_movement((Player*)player);
    return 0;
}

DRB_FFI_NAME(player_surface_warp)
int matocore_player_surface_warp(void *player) {
    player_do_surface_warp((Player*)player);
    return 0;
}

