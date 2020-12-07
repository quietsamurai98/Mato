//TODO: Storing all the state in global variables is... not great.
#include "types.h"
#ifndef ZOOM
#define ZOOM 2
#define WIDTH ((int)(1280/ZOOM))
#define HEIGHT ((int)(720/ZOOM))
#define PIXELS (WIDTH*HEIGHT)
#endif //ZOOM
#ifndef CEXT_GLOBALS_H
#define CEXT_GLOBALS_H
extern Color SCREEN[PIXELS];
extern byte  TERRAIN[PIXELS];
Color SCREEN[PIXELS];
byte  TERRAIN[PIXELS];
#else
extern Color SCREEN[PIXELS];
extern byte  TERRAIN[PIXELS];
#endif //CEXT_GLOBALS_H
