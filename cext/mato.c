#ifdef NOT_DRAGONRUBY
#define DRB_FFI //This will be accessible from DRGTK
#define DRB_FFI_NAME(name) //This will be accessible from DRGTK as `name`
#else
#include <dragonruby.h>
#endif

#define FNL_IMPL
#include "lib/FastNoiseLite.h"

#include <string.h>
#include <math.h>
#include "src/globals.h"
#include "src/render.h"
#include "src/terrain.h"

#ifndef NOT_DRAGONRUBY
extern void *(*drb_symbol_lookup)(const char *sym);
typedef void (*drb_upload_pixel_array_fn)(const char *name, const int w, const int h, const Uint32 *pixels);
DRB_FFI
int screen_to_sprite(const char *name)
{
    static drb_upload_pixel_array_fn drb_upload_pixel_array = NULL;
    if (!drb_upload_pixel_array) {
        drb_upload_pixel_array = drb_symbol_lookup("drb_upload_pixel_array");
        if (!drb_upload_pixel_array) {
            return 1;  // oh well.
        }
    }
    drb_upload_pixel_array(name, WIDTH, HEIGHT, (const Uint32 *) &SCREEN);
    return 0;
}
#endif

DRB_FFI
int blank_screen(void) {
    memset(SCREEN, 0, PIXELS * sizeof(Color));
    return 0;
}
DRB_FFI
int draw_terrain(void) {
    return render_terrain();
}

DRB_FFI
int generate_terrain(int base_seed, int smooth_seed) {
    terrain_generate(base_seed, smooth_seed);
    return 0;
}
DRB_FFI
int update_sand(void) {
    terrain_update_sand();
    return 0;
}
DRB_FFI
int destroy_terrain(int screen_x, int screen_y, int radius) {
    int      mx = screen_x / ZOOM;
    int      my = HEIGHT - (screen_y / ZOOM);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (sqrt(dx * dx + dy * dy) <= radius) {
                int x = mx + dx;
                int y = my + dy;
                set_terrain_at_pixel(x,y,0x00);
            }
        }
    }
    return 0;
}
DRB_FFI
int create_terrain(int screen_x, int screen_y, int radius) {
    int      mx = screen_x / ZOOM;
    int      my = HEIGHT - (screen_y / ZOOM);
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (!get_terrain_at_pixel(mx+dx,my+dy,0x01) && sqrt(dx * dx + dy * dy) <= radius) {
                int x = mx + dx;
                int y = my + dy;
                set_terrain_at_pixel(x, y, 0x02);
            }
        }
    }
    return 0;
}
