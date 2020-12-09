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
    player->px             = x;
    player->py             = y;
    player->vx             = 0;
    player->vy             = 0;
    player->color          = (DblColor) {r, g, b, 1.0};
    player->hp_max         = 1000;
    player->hp             = player->hp_max;
    player->hp_delta       = 0;
    player->w              = 0;
    player->h              = 0;
    player->sprite         = NULL;
    player->collision_mask = NULL;
    return player;
}

void player_destroy_player(Player **player) {
    free((*player)->sprite);
    free((*player)->collision_mask);
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

static LocalTerrain intersecting_terrain(Player *player, int offset_x, int offset_y) {
    LocalTerrain lt = {0};

    int x = (int) player->px + offset_x;
    int y = (int) player->py + offset_y;

    for (int dy = 0; dy < 16; ++dy) {
        for (int dx = 0; dx < 16; ++dx) {
            uint32_t mask      = player->collision_mask[dy * 16 + dx].m_color;
            byte collide_any   = player->collision_mask[dy * 16 + dx].ch.a;
            byte collide_north = player->collision_mask[dy * 16 + dx].ch.g & 0xF0;
            byte collide_south = player->collision_mask[dy * 16 + dx].ch.g & 0x0F;
            byte collide_east  = player->collision_mask[dy * 16 + dx].ch.r & 0xF0;
            byte collide_west  = player->collision_mask[dy * 16 + dx].ch.r & 0x0F;
            byte     mat       = terrain_get_pixel(x + dx, y + dy, TERRAIN_DIRT).type; //Treat the edges like walls of indestructible dirt
            if (terrain_get_pixel_solidness(x + dx, y + dy, 1, 0.2) >= 1) { //Only collide with solid pixels
                if (collide_any) {
                    lt.clipped_mats_all[dy][dx] = mat;
                    lt.any = true;
                    if (collide_north && !lt.clipped_mats_top[dx]) {
                        lt.clipped_mats_top[dx] = mat;
                        lt.clipped_bits_top |= 0b1 << dx;
                    }
                    if (collide_south) {
                        lt.clipped_mats_bottom[dx] = mat;
                        lt.clipped_bits_bottom |= 0b1 << dx;
                    }
                    if (collide_east) {
                        lt.clipped_mats_right[dy] = mat;
                        lt.clipped_bits_right |= 0b1 << dy;
                    }
                    if (collide_west && !lt.clipped_mats_left[dy]) {
                        lt.clipped_mats_left[dy] = mat;
                        lt.clipped_bits_left |= 0b1 << dy;
                    }
                }
            }
        }
    }
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

void player_do_terrain_edit(Player *player) {
    if (player->input.move.vertical < 0) {
        for (int dy = 13; dy < 16; dy++) {
            for (int dx = 6; dx < 10; dx += 1) {
                if (terrain_get_pixel((int) player->px + dx, (int) player->py + dy, TERRAIN_DIRT).type == TERRAIN_NONE_TYPE)
                    terrain_set_pixel((int) player->px + dx, (int) player->py + dy, TERRAIN_XHST);
            }
        }
    }
}

void player_calc_input(Player *player, double move_vertical, double move_horizontal) {
    player->input.move.vertical   = fclamp(-move_vertical, -1, 0);
    player->input.move.horizontal = fclamp(move_horizontal, -1, 1);
}

void player_do_input(Player *player) {
    player->vx += player->input.move.horizontal * 0.4;
    player->vy += player->input.move.vertical * 0.4;
}

void player_do_movement(Player *player) {
    LocalTerrain clipping_terrain = intersecting_terrain(player, 0, 0);
    if (!clipping_terrain.clipped_bits_bottom) player->vy += 0.2;
    /// You got stuck. Now dig.
    if ((clipping_terrain.clipped_bits_left && clipping_terrain.clipped_bits_right) ||
        (clipping_terrain.clipped_bits_top && clipping_terrain.clipped_bits_bottom)) {
        player->vx = 0;
        player->vy = 0;
        return;
    }
    int      steps    = (int) (fmax(fabs(player->vx), fabs(player->vy)) + 1);
    double   dx       = player->vx / steps;
    double   dy       = player->vy / steps;
    for (int cur_step = 0; cur_step < steps; cur_step++) {

        player->px += dx;
        player->py += dy;
        clipping_terrain = intersecting_terrain(player, 0, 0);
        if (clipping_terrain.clipped_bits_bottom) {
            player->py -= 1;
            player->vy -= 0.5;
            clipping_terrain = intersecting_terrain(player, 0, 0);
        }
        if (clipping_terrain.clipped_bits_top) {
            player->py += 1;
            player->vy += 0.5;
            clipping_terrain = intersecting_terrain(player, 0, 0);
        }
        if (clipping_terrain.clipped_bits_left) {
            player->px += 1;
            player->vx += 0.5;
            clipping_terrain = intersecting_terrain(player, 0, 0);
        }
        if (clipping_terrain.clipped_bits_right) {
            player->px -= 1;
            player->vx -= 0.5;
            clipping_terrain = intersecting_terrain(player, 0, 0);
        }
    }

    player->vy = fclamp(player->vy, -3.99, 3.99);
    player->vx *= 0.9;


    /** ORIGINAL PLAN:
     *
     *  1: Update the player's position according to their current velocity.
     *      px+=vx; py+=vy;
     *
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
