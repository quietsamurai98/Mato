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
#include "src/drb_symbols.h"
#include "src/globals.h"
#include "src/render.h"
#include "src/terrain.h"
#include "src/player.h"

//TODO: This is just for debugging the very basics of player physics
static Player PLAYER1 = {0};

DRB_FFI
int blank_screen(void) {
    memset(SCREEN, 0, PIXELS * sizeof(Color));
    return 0;
}

DRB_FFI
int draw_terrain(void) {
    render_terrain();
    return 0;
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
                terrain_set_pixel(x, y, 0x00);
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
            if (!terrain_get_pixel(mx + dx, my + dy, 0x01) && sqrt(dx * dx + dy * dy) <= radius) {
                int x = mx + dx;
                int y = my + dy;
                terrain_set_pixel(x, y, 0x02);
            }
        }
    }
    return 0;
}

DRB_FFI
int set_player1_pos(int screen_x, int screen_y) {
    int mx = screen_x / ZOOM;
    int my = HEIGHT - (screen_y / ZOOM);
    if(PLAYER1.w == 0)  {
        PLAYER1 = *player_initialize_player(mx, my, 1.0, 0.3, 0.3);
        PLAYER1.sprite = load_image("/sprites/player.png", &PLAYER1.w, &PLAYER1.h);
    }
    PLAYER1.px = mx;
    PLAYER1.py = my;
    return 0;
}

DRB_FFI
int move_player1_to_surface(){
    player_do_surface_warp(&PLAYER1);
    return (int)((HEIGHT-PLAYER1.py) * ZOOM);
}

DRB_FFI
int draw_player1(void){
    if(PLAYER1.w != 0) render_player(&PLAYER1);
    return 0;
}
