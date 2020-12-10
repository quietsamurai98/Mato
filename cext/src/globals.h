//TODO: Storing all the state in global variables is... not great.
#include "types.h"

#ifndef ZOOM
#define ZOOM (2)
//#define HORZ_SCREENS 4
//#define TERRAIN_WIDTH ((int)(HORZ_SCREENS*1280/ZOOM))
//#define TERRAIN_HEIGHT ((int)(720/ZOOM))
#define TERRAIN_WIDTH (1600)
#define TERRAIN_HEIGHT (800)
#define TERRAIN_PIXELS (TERRAIN_WIDTH*TERRAIN_HEIGHT)
#define SCREEN_WIDTH (int)(1280/ZOOM)
#define SCREEN_HEIGHT (int)(720/ZOOM)
#define SCREEN_PIXELS (SCREEN_WIDTH*SCREEN_HEIGHT)

#define TERRAIN_NONE_TYPE (0x00)
#define TERRAIN_DIRT_TYPE (0x01)
#define TERRAIN_SAND_TYPE (0x02)
#define TERRAIN_XHST_TYPE (0x03)
#define TERRAIN_SMKE_TYPE (0x04)
#define TERRAIN_VOID_TYPE (0x05)

#define TERRAIN_NONE ((TerrainPixel){.type=TERRAIN_NONE_TYPE, .has_moved=0x00})
#define TERRAIN_DIRT ((TerrainPixel){.type=TERRAIN_DIRT_TYPE, .has_moved=0x00})
#define TERRAIN_SAND ((TerrainPixel){.type=TERRAIN_SAND_TYPE, .has_moved=0x00})
#define TERRAIN_XHST ((TerrainPixel){.type=TERRAIN_XHST_TYPE, .has_moved=0x00})
#define TERRAIN_SMKE ((TerrainPixel){.type=TERRAIN_SMKE_TYPE, .has_moved=0x00})
#define TERRAIN_VOID ((TerrainPixel){.type=TERRAIN_VOID_TYPE, .has_moved=0x00})

#endif //ZOOM
#ifndef CEXT_GLOBALS_H
#define CEXT_GLOBALS_H
extern Color        SCREEN[SCREEN_PIXELS];
Color               SCREEN[SCREEN_PIXELS];
extern TerrainPixel TERRAIN[TERRAIN_PIXELS];
TerrainPixel        TERRAIN[TERRAIN_PIXELS];
#else
extern Color        SCREEN[SCREEN_PIXELS];
extern TerrainPixel TERRAIN[TERRAIN_PIXELS];
#endif //CEXT_GLOBALS_H
