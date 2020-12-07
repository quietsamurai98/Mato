#ifndef CEXT_TERRAIN_H
#define CEXT_TERRAIN_H

int terrain_generate(int base_seed, int smooth_seed);

int terrain_update_sand();

void set_terrain_at_pixel(int x, int y, byte mat);
byte get_terrain_at_pixel(int x, int y, byte edge);
#endif //CEXT_TERRAIN_H
