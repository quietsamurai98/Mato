#include "globals.h"
#include "render.h"

int render_terrain() {
#pragma omp parallel for
    for (int o = 0; o < PIXELS; ++o) {
        switch (TERRAIN[o].type) {
            case TERRAIN_DIRT_TYPE:SCREEN[o] = (Color) {0xFF004891};
                break;
            case TERRAIN_SAND_TYPE:SCREEN[o] = (Color) {0xFF139BAD};
                break;
            case TERRAIN_XHST_TYPE:SCREEN[o] = (Color) {0xFFA8A8A8};
                break;
            default: SCREEN[o] = (Color) {0x00000000};  //Air
        }
    }
    return 0;
}
int render_player(Player *player){
    int ox = (int) player->px;
    int oy = (int) player->py;
    int sy = 0;
    int sx;
    int isx = 0;
    int dsx = 1;
    if(player->input.move.horizontal < 0 || (player->input.move.horizontal == 0 && player->vx < 0)){
        isx = 15;
        dsx = -1;
    }
    for (int y = oy; y < 16+oy; ++y) {
        sx = isx;
        for (int x = ox; x < 16+ox; ++x) {
            if (x < WIDTH && x >= 0 && y >= 0 && y < HEIGHT){
                Color col = player->sprite[sy*16+sx];
                if(col.ch.a && (byte)(player->color.a*col.ch.a)){
                    col.ch.r *= player->color.r;
                    col.ch.g *= player->color.g;
                    col.ch.b *= player->color.b;
                    col.ch.a *= player->color.a;
                    SCREEN[y*WIDTH+x] = col;
                }
            }
            sx+=dsx;
        }
        sy++;
    }
    return 0;
}
