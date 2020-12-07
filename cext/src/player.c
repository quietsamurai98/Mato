#include <stdlib.h>
#include <stdbool.h>
#include "player.h"
#include "terrain.h"
#include "globals.h"

Player *player_initialize_player(double x, double y, double r, double g, double b) {
    Player *player = calloc(1, sizeof(Player));
    if (!player) return player; //If we can't calloc the player struct for some reason, return early to avoid a segfault.
    *player = (Player) {0};
    player->px       = x;
    player->py       = y;
    player->vx       = 0;
    player->vy       = 0;
    player->color    = (DblColor) {r, g, b, 1.0};
    player->hp_max   = 1000;
    player->hp       = player->hp_max;
    player->hp_delta = 0;
    player->w = 0;
    player->h = 0;
    player->sprite   = NULL;
    return player;
}

void player_destroy_player(Player **player) {
    free((*player)->sprite);
    free(*player);
    *player = NULL;
}

typedef struct {
    bool     any;
    byte     clipped_mats_all[16][16];
    byte     clipped_mats_top[16];
    byte     clipped_mats_bottom[16];
    byte     clipped_mats_left[16];
    byte     clipped_mats_right[16];
    uint16_t clipped_bits_top;
    uint16_t clipped_bits_bottom;
    uint16_t clipped_bits_left;
    uint16_t clipped_bits_right;
} LocalTerrain;

static void lt_side_calc(LocalTerrain *lt){
    for (int dy = 0; dy < 4; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            if (!lt->clipped_mats_top[dx]) {
                lt->clipped_mats_top[dx] = lt->clipped_mats_all[dy][dx];
                if (lt->clipped_mats_top[dx]) lt->clipped_bits_top |= 0b0000000000000001 << dx;
            }
        }
    }
    for (int dy = 16 - 1; dy >= 12; --dy) {
        for (int dx = 0; dx < 16; ++dx) {
            if (!lt->clipped_mats_bottom[dx]) {
                lt->clipped_mats_bottom[dx] = lt->clipped_mats_all[dy][dx];
                if (lt->clipped_mats_bottom[dx]) lt->clipped_bits_bottom |= 0b0000000000000001 << dx;
            }
        }
    }
    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 4; ++dx) {
            if (!lt->clipped_mats_left[dy]) {
                lt->clipped_mats_left[dy] = lt->clipped_mats_all[dy][dx];
                if(lt->clipped_mats_left[dy]) lt->clipped_bits_left |= 0b0000000000000001 << dy;
            }
        }
    }
    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 16 - 1; dx >= 12; --dx) {
            if (!lt->clipped_mats_right[dy]) {
                lt->clipped_mats_right[dy] = lt->clipped_mats_all[dy][dx];
                if(lt->clipped_mats_right[dy]) lt->clipped_bits_right |= 0b0000000000000001 << dy;
            }
        }
    }
}

static LocalTerrain any_terrain(Player *player, int offset_x, int offset_y) {
    LocalTerrain lt = {0};

    int x = (int) player->px + offset_x;
    int y = (int) player->py + offset_y;

    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            if (terrain_get_pixel_solidness(x + dx, y + dy, 1, 0.2) >= 1) { //Only collide with solid pixels
                lt.clipped_mats_all[dy][dx] = terrain_get_pixel(x + dx, y + dy, 0x01); //Treat the edges like walls of indestructible dirt
                if(lt.clipped_mats_all[dy][dx]) lt.any = true;
            }
        }
    }
    lt_side_calc(&lt);
    return lt;
}

static LocalTerrain intersecting_terrain(Player *player) {
    LocalTerrain lt = {0};

    int x = (int) player->px;
    int y = (int) player->py;

    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            //Pixel perfect collision detection done dirt cheap: if the pixel is fully transparent in the sprite, the pixel shouldn't collide with terrain.
            if (player->sprite[dy * 16 + dx].ch.a)
                if (terrain_get_pixel_solidness(x + dx, y + dy, 1, 0.2) >= 1) { //Only collide with solid pixels
                    lt.clipped_mats_all[dy][dx] = terrain_get_pixel(x + dx, y + dy, 0x01); //Treat the edges like walls of indestructible dirt
                    if(lt.clipped_mats_all[dy][dx]) lt.any = true;
                }
        }
    }
    lt_side_calc(&lt);
    return lt;
}

void player_do_surface_warp(Player *player) {
    LocalTerrain lt = intersecting_terrain(player);
    //TODO: ew.
    if (lt.any) {
        //Up-warp
        while (lt.any && player->py > 0) {
            player->py -= 1;
            lt = intersecting_terrain(player);
        }
    } else if (!lt.clipped_bits_bottom) {
        //Down-warp
        while (!lt.clipped_bits_bottom && player->py < HEIGHT) {
            player->py += 1;
            lt = intersecting_terrain(player);
        }
        player->py -= 1;
    }
}
