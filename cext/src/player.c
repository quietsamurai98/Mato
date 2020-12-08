#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "player.h"
#include "terrain.h"
#include "globals.h"
#include "utils.h"

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
    player->w        = 0;
    player->h        = 0;
    player->sprite   = NULL;
    return player;
}

void player_destroy_player(Player **player) {
    free((*player)->sprite);
    free(*player);
    *player = NULL;
}

void player_calc_input(Player *player, double move_vertical, double move_horizontal) {
    player->input.move.vertical   = fclamp(-move_vertical, -1, 1);
    player->input.move.horizontal = fclamp(move_horizontal, -1, 1);
}

void player_do_input(Player *player) {
    player->vx += player->input.move.horizontal * 0.5;
    player->vy += player->input.move.vertical;
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

static void lt_side_calc(LocalTerrain *lt) {
    for (int dy = 0; dy < 8; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            if (!lt->clipped_mats_top[dx]) {
                lt->clipped_mats_top[dx] = lt->clipped_mats_all[dy][dx];
                if (lt->clipped_mats_top[dx]) lt->clipped_bits_top |= 0b0000000000000001 << dx;
            }
        }
    }
    for (int dy = 16 - 1; dy >= 8; --dy) {
        for (int dx = 0; dx < 16; ++dx) {
            if (!lt->clipped_mats_bottom[dx]) {
                lt->clipped_mats_bottom[dx] = lt->clipped_mats_all[dy][dx];
                if (lt->clipped_mats_bottom[dx]) lt->clipped_bits_bottom |= 0b0000000000000001 << dx;
            }
        }
    }
    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 8; ++dx) {
            if (!lt->clipped_mats_left[dy]) {
                lt->clipped_mats_left[dy] = lt->clipped_mats_all[dy][dx];
                if (lt->clipped_mats_left[dy]) lt->clipped_bits_left |= 0b0000000000000001 << dy;
            }
        }
    }
    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 16 - 1; dx >= 8; --dx) {
            if (!lt->clipped_mats_right[dy]) {
                lt->clipped_mats_right[dy] = lt->clipped_mats_all[dy][dx];
                if (lt->clipped_mats_right[dy]) lt->clipped_bits_right |= 0b0000000000000001 << dy;
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
                lt.clipped_mats_all[dy][dx] = terrain_get_pixel(x + dx, y + dy, TERRAIN_DIRT).type; //Treat the edges like walls of indestructible dirt
                if (lt.clipped_mats_all[dy][dx]) lt.any = true;
            }
        }
    }
    lt_side_calc(&lt);
    return lt;
}

static LocalTerrain intersecting_terrain(Player *player, int offset_x, int offset_y) {
    LocalTerrain lt = {0};

    int x = (int) player->px + offset_x;
    int y = (int) player->py + offset_y;

    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            //Pixel perfect collision detection done dirt cheap: if the pixel is fully transparent in the sprite, the pixel shouldn't collide with terrain.
            if (player->sprite[dy * 16 + dx].ch.a)
                if (terrain_get_pixel_solidness(x + dx, y + dy, 1, 0.2) >= 1) { //Only collide with solid pixels
                    lt.clipped_mats_all[dy][dx] = terrain_get_pixel(x + dx, y + dy, TERRAIN_DIRT).type; //Treat the edges like walls of indestructible dirt
                    if (lt.clipped_mats_all[dy][dx]) lt.any = true;
                }
        }
    }
    lt_side_calc(&lt);
    return lt;
}

void player_do_surface_warp(Player *player) {
    LocalTerrain lt = intersecting_terrain(player, 0, 0);
    //TODO: ew.
    if (lt.any) {
        //Up-warp
        while (lt.any && player->py > 0) {
            player->py -= 1;
            lt = intersecting_terrain(player, 0, 0);
        }
    } else if (!lt.clipped_bits_bottom) {
        //Down-warp
        while (!lt.clipped_bits_bottom && player->py < HEIGHT) {
            player->py += 1;
            lt = intersecting_terrain(player, 0, 0);
        }
        player->py -= 1;
    }
}

void warp_N(Player *player) {
    LocalTerrain lt = intersecting_terrain(player, 0, 0);
    while (lt.clipped_bits_bottom && player->py > 0) {
        player->py -= 1;
        lt = intersecting_terrain(player, 0, 0);
    }
}

void warp_S(Player *player) {
    LocalTerrain lt = intersecting_terrain(player, 0, 0);
    while (lt.clipped_bits_top && player->py < HEIGHT - player->h) {
        player->py += 1;
        lt = intersecting_terrain(player, 0, 0);
    }
}

void warp_W(Player *player) {
    LocalTerrain lt = intersecting_terrain(player, 0, 0);
    while (lt.clipped_bits_right && player->px > 1) {
        player->px -= 1;
        lt = intersecting_terrain(player, 0, 0);
    }
}

void warp_E(Player *player) {
    LocalTerrain lt = intersecting_terrain(player, 0, 0);
    while (lt.clipped_bits_left && player->px < WIDTH - player->w) {
        player->px += 1;
        lt = intersecting_terrain(player, 0, 0);
    }
}

void player_do_movement(Player *player) {
    LocalTerrain clipping_terrain = intersecting_terrain(player, 0, 0);
    LocalTerrain local_terrain    = any_terrain(player, 0, 0);
    /**
     *  1: Update the player's position according to their current velocity.
     *      px+=vx; py+=vy;
     */
    player->px += player->vx;
    player->py += player->vy;
    player->vx *= 0.9;
    player->vy                    = fmax(player->vy, -1);
    if (!(clipping_terrain.clipped_bits_left & 0b0000111111110000) != !(clipping_terrain.clipped_bits_right & 0b0000111111110000)) {
        if (clipping_terrain.clipped_bits_left & 0b0000111111110000) {
            player->vx = fmin(player->vx, 0);
            warp_E(player);
            clipping_terrain = intersecting_terrain(player, 0, 0);
        } else {
            player->vx = fmax(player->vx, 0);
            warp_W(player);
            clipping_terrain = intersecting_terrain(player, 0, 0);
        }
    }
    if(!clipping_terrain.clipped_bits_bottom) player->vy += 0.2;
    if(!clipping_terrain.clipped_bits_bottom != !clipping_terrain.clipped_bits_top){
        if (clipping_terrain.clipped_bits_bottom) {
            player->vy = fmin(player->vy, 0);
            warp_N(player);
            clipping_terrain = intersecting_terrain(player, 0, 0);
        } else {
            player->vy = fmax(player->vy, 0);
            warp_S(player);
            clipping_terrain = intersecting_terrain(player, 0, 0);
        }
    }

    /**
     *  2: Update the player's velocity to reflect the terrain at this new position (excluding collision resolution).
     *      Moving through dynamic terrain (i.e. falling sand) will push the player according to the terrain's velocity.
     *      If the player is in the air, they accelerate downwards due to gravity.
     *  3: Resolve collisions with the terrain, updating the player's position and velocity.
     *      If the player's bottom-side is clipping into terrain, and there's room above their top-side, move up a little and add a little upwards velocity.
     *      If the player's top-side is clipping into terrain, and there's room below their bottom-side, move down a little and add a little downwards velocity.
     *      If the top and bottom of the player's left-side is clipping into terrain, and there's room to their right-side, move right a little and zero out the horizontal velocity.
     *      If the top and bottom of the player's right-side is clipping into terrain, and there's room to their left-side, move left a little and zero out the horizontal velocity.
     *      If any two opposite corners of the player are clipping into terrain, zero out the velocity. Player will need to dig their way out.
     */
}
