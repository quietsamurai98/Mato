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
