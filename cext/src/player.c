#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "player.h"
#include "terrain.h"
#include "globals.h"
#include "utils.h"
#include "xorshift.h"

#define GRAVITY (0.2)

Player *player_initialize_player(double x, double y, double r, double g, double b) {
    Player *player = calloc(1, sizeof(Player));
    if (!player) return player; //If we can't calloc the player struct for some reason, return early to avoid a segfault.
    *player = (Player) {0};
    player->px             = x;
    player->py             = y;
    player->color          = (DblColor) {r, g, b, 1.0};
    player->hp_max         = 1000;
    player->hp             = player->hp_max;
    player->facing         = 1;
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
            Color mask          = player->collision_mask[dy * 16 + dx];
            byte  collide_any   = mask.ch.a;
            byte  collide_north = mask.ch.g & 0xF0;
            byte  collide_south = mask.ch.g & 0x0F;
            byte  collide_east  = mask.ch.r & 0xF0;
            byte  collide_west  = mask.ch.r & 0x0F;
            byte  mat           = terrain_get_pixel(x + dx, y + dy, TERRAIN_DIRT).type; //Treat the edges like walls of indestructible dirt
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
        while (!lt.clipped_bits_bottom && player->py < TERRAIN_SIZE) {
            player->py += 1;
            lt = intersecting_terrain(player, 0, 0);
        }
        player->py -= 1;
    }
}

void player_do_terrain_edit(Player *player) {
    //TODO: Allow player to dig out terrain natively
}

void player_calc_input(Player *player, double move_vertical, double move_horizontal) {
    player->input.move.vertical   = -fclamp(move_vertical, 0, 1);
    player->input.move.horizontal = fclamp(move_horizontal, -1, 1);
}

void player_do_input(Player *player) {
    LocalTerrain clipping_terrain = intersecting_terrain(player, 0, 2);
    if (clipping_terrain.clipped_bits_bottom) {
        player->physics_data.acc_x += player->input.move.horizontal * 0.06;
    } else {
        player->physics_data.acc_x += player->input.move.horizontal * 0.04;
    }
    player->physics_data.acc_y += player->input.move.vertical * 0.04;

    player->physics_data.acc_x = fclamp(player->physics_data.acc_x, -1.0, 1.0);
    player->physics_data.acc_y = fclamp(player->physics_data.acc_y, fmin(-player->physics_data.vel_y, -GRAVITY * 2), 0);

    if (player->input.move.horizontal == 0) {
        player->physics_data.acc_x = 0;
    } else {
        player->facing = (player->input.move.horizontal > 0) - (player->input.move.horizontal < 0);
    }
    if (player->input.move.vertical == 0) {
        player->physics_data.acc_y = 0;
    }

}

void player_do_movement(Player *player) {
    LocalTerrain clipping_terrain = intersecting_terrain(player, 0, 0);
    if (!clipping_terrain.clipped_bits_bottom) player->physics_data.vel_y += GRAVITY;
    /// You got stuck. Now dig.
    if (clipping_terrain.clipped_bits_left && clipping_terrain.clipped_bits_right && clipping_terrain.clipped_bits_top && clipping_terrain.clipped_bits_bottom) {
        player->physics_data.acc_x = 0;
        player->physics_data.acc_y = 0;
        player->physics_data.vel_x = 0;
        player->physics_data.vel_y = 0;
        return;
    }


    player->physics_data.vel_x += player->physics_data.acc_x;
    player->physics_data.vel_y += player->physics_data.acc_y;
    double vx       = player->physics_data.vel_x;
    double vy       = player->physics_data.vel_y;
    double ix       = player->px;
    double iy       = player->py;
    int    steps    = (int) (fmax(fabs(vx), fabs(vy)) + 1);
    double dx       = vx / steps;
    double dy       = vy / steps;
    int    on_floor = 0;

    for (int cur_step = 0; cur_step < steps; cur_step++) {

        player->px += dx;
        player->py += dy;
        clipping_terrain = intersecting_terrain(player, 0, 0);
        LocalTerrain xhst_clipping_terrain = intersecting_terrain(player, 0, 1);

        if (!xhst_clipping_terrain.clipped_bits_bottom && !(player->input.move.vertical == 0 && player->input.move.horizontal == 0)) {
            int    xhst_n = 12;
            int    xhst_s = 16;
            int    xhst_w = 6;
            int    xhst_e = 10;
            double prob   = 1.0 / ((xhst_s - xhst_n) * (xhst_e - xhst_w));

            for (int xhst_dy = xhst_n; xhst_dy < xhst_s; xhst_dy++) {
                for (int xhst_dx = xhst_w; xhst_dx < xhst_e; xhst_dx += 1) {
                    if (terrain_get_pixel((int) player->px + xhst_dx, (int) player->py + xhst_dy, TERRAIN_DIRT).type == TERRAIN_NONE_TYPE) {
                        if (xor_rand_double() < prob) { terrain_set_pixel((int) player->px + xhst_dx, (int) player->py + xhst_dy, TERRAIN_XHST, true); }
                    }
                }
            }
        }

        if (clipping_terrain.clipped_bits_left && clipping_terrain.clipped_bits_right && clipping_terrain.clipped_bits_top && clipping_terrain.clipped_bits_bottom) {
            player->px -= dx;
            player->py -= dy;
            continue;
        }
        double bump_px = 0;
        double bump_vx = 0;
        double bump_py = 0;
        double bump_vy = 0;
        if (clipping_terrain.clipped_bits_bottom) {
            bump_py -= 1;
            bump_vy -= GRAVITY * 1.5;
            on_floor = true;
        }
        if (clipping_terrain.clipped_bits_top) {
            bump_py += 1;
            bump_vy += GRAVITY * 1.5;
        }
        if (clipping_terrain.clipped_bits_left) {
            bump_px += 1;
            bump_vx += GRAVITY * 1.5;
        }
        if (clipping_terrain.clipped_bits_right) {
            bump_px -= 1;
            bump_vx -= GRAVITY * 1.5;
        }
        player->px += bump_px;
        player->py += bump_py;
        player->physics_data.vel_x += bump_vx;
        player->physics_data.vel_y += bump_vy;
    }
    ///Failsafe to prevent getting stuck
    clipping_terrain = intersecting_terrain(player, 0, 0);
    if (clipping_terrain.clipped_bits_left && clipping_terrain.clipped_bits_right && clipping_terrain.clipped_bits_top && clipping_terrain.clipped_bits_bottom) {
        player->px = ix;
        player->py = iy;
        return;
    }

    if (on_floor) {
        player->physics_data.vel_x *= 0.7;
    } else {
        player->physics_data.vel_x *= 0.999;
    }
    /// Prevent the player from accelerating to ludicrous speeds, but still allow them to be flung at high speeds
    if (fabs(player->physics_data.vel_x) > 4.0) {
        if (signbit(player->physics_data.vel_x) == signbit(player->physics_data.acc_x)) {
            player->physics_data.vel_x -= player->physics_data.acc_x * 0.9;
        }
    }


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
