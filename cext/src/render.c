#include "globals.h"
#include "render.h"
#include "terrain.h"

int render_terrain(int x_0, int y_0) {
#pragma omp parallel for collapse(2)
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            switch (terrain_get_pixel(x + x_0, y + y_0, TERRAIN_VOID).type) {
                case TERRAIN_DIRT_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0xFF1C4E71};
                    break;
                case TERRAIN_SAND_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0xFF1097AE};
                    break;
                case TERRAIN_XHST_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0xFFA8A8A8};
                    break;
                case TERRAIN_SMKE_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0xFF8A8A8A};
                    break;
                case TERRAIN_VOID_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0xFFFF00FF};
                    break;
                default: SCREEN[y * SCREEN_WIDTH + x] = (Color) {0x00000000};  //Air
            }
        }
    }
    return 0;
}
int render_terrain_debug(int x_0, int y_0) {
#pragma omp parallel for collapse(2)
    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
        for (int x = 0; x < SCREEN_WIDTH; ++x) {
            if(terrain_get_pixel(x + x_0, y + y_0, TERRAIN_VOID).type != TERRAIN_VOID_TYPE) {
                TerrainTreeNode *node = terrain_get_node_at(x + x_0, y + y_0);
                uint32_t alpha = 0x7F000000;
                if(node->metadata.need_updating) alpha = 0xFF000000;
                switch (TERRAIN[node->tp_offset].type) {
                    case TERRAIN_DIRT_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0x001C4E71 | alpha};
                        break;
                    case TERRAIN_SAND_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0x001097AE | alpha};
                        break;
                    case TERRAIN_XHST_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0x00A8A8A8 | alpha};
                        break;
                    case TERRAIN_SMKE_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0x008A8A8A | alpha};
                        break;
                    case TERRAIN_VOID_TYPE:SCREEN[y * SCREEN_WIDTH + x] = (Color) {0x00FF00FF | alpha};
                        break;
                        default: SCREEN[y * SCREEN_WIDTH + x] = (Color) {alpha == 0xFF000000 ? 0x7FFFFFFF : 0x00000000};  //Air
                }
            }
        }
    }
    return 0;
}

int render_player(Player *player, int x_0, int y_0) {
    int ox  = (int) player->px - x_0;
    int oy  = (int) player->py - y_0;
    int sy  = 0;
    int sx;
    int isx = 0;
    if (player->facing == -1) {
        isx = 15;
    }
    for (int y = oy; y < 16 + oy; ++y) {
        sx = isx;
        for (int x = ox; x < 16 + ox; ++x) {
            if (x < SCREEN_WIDTH && x >= 0 && y >= 0 && y < SCREEN_HEIGHT) {
                Color col = player->sprite[sy * 16 + sx];
                if (col.ch.a && (byte) (player->color.a * col.ch.a)) {
                    col.ch.r *= player->color.r;
                    col.ch.g *= player->color.g;
                    col.ch.b *= player->color.b;
                    col.ch.a *= player->color.a;
                    SCREEN[y * SCREEN_WIDTH + x] = col;
                }
            }
            sx += player->facing;
        }
        sy++;
    }
    return 0;
}
