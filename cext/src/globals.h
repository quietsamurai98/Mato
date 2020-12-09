//TODO: Storing all the state in global variables is... not great.
#include "types.h"

#ifndef ZOOM
#define ZOOM 2
#define HORZ_SCREENS 4
#define WIDTH ((int)(HORZ_SCREENS*1280/ZOOM))
#define HEIGHT ((int)(720/ZOOM))
#define PIXELS (WIDTH*HEIGHT)

#define TERRAIN_NONE_TYPE (0x00)
#define TERRAIN_DIRT_TYPE (0x01)
#define TERRAIN_SAND_TYPE (0x02)
#define TERRAIN_XHST_TYPE (0x03)
#define TERRAIN_SMKE_TYPE (0x04)

#define TERRAIN_NONE ((TerrainPixel){.type=TERRAIN_NONE_TYPE, .has_moved=0x00})
#define TERRAIN_DIRT ((TerrainPixel){.type=TERRAIN_DIRT_TYPE, .has_moved=0x00})
#define TERRAIN_SAND ((TerrainPixel){.type=TERRAIN_SAND_TYPE, .has_moved=0x00})
#define TERRAIN_XHST ((TerrainPixel){.type=TERRAIN_XHST_TYPE, .has_moved=0x00})
#define TERRAIN_SMKE ((TerrainPixel){.type=TERRAIN_SMKE_TYPE, .has_moved=0x00})

#endif //ZOOM
#ifndef CEXT_GLOBALS_H
#define CEXT_GLOBALS_H
extern Color        SCREEN[PIXELS];
Color               SCREEN[PIXELS];
extern TerrainPixel TERRAIN[PIXELS];
TerrainPixel        TERRAIN[PIXELS];
#else
extern Color        SCREEN[PIXELS];
extern TerrainPixel TERRAIN[PIXELS];
#endif //CEXT_GLOBALS_H
