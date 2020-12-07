#include "globals.h"
#include "render.h"

int render_terrain() {
#pragma omp parallel for
    for (int o = 0; o < PIXELS; ++o) {
        switch (TERRAIN[o]) {
            case 0x01:SCREEN[o] = (Color) {0xFF004891}; //Dirt
                break;
            case 0x02:SCREEN[o] = (Color) {0xFF139BAD}; //Sand
                break;
            default: SCREEN[o] = (Color) {0x00000000};  //Air
        }
    }
    return 0;
}
int render_player(Player *player){
    int ox = (int) player->px;
    int oy = (int) player->py;
    int so = 0;
    for (int y = oy; y < 16+oy; ++y) {
        for (int x = ox; x < 16+ox; ++x) {
            if (x < WIDTH && x >= 0 && y >= 0 && y < HEIGHT){
                Color col = player->sprite[so];
                if(col.ch.a && (byte)(player->color.a*col.ch.a)){
                    col.ch.r *= player->color.r;
                    col.ch.g *= player->color.g;
                    col.ch.b *= player->color.b;
                    col.ch.a *= player->color.a;
                    SCREEN[y*WIDTH+x] = col;
                }
            }
            so++;
        }
    }
    return 0;
}
