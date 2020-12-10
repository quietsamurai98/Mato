#ifndef CEXT_TERRAIN_H
#define CEXT_TERRAIN_H

void terrain_generate(int base_seed, int smooth_seed);

void terrain_update();

void terrain_set_pixel(int x, int y, TerrainPixel terrain_pixel, bool should_update);

TerrainTreeNode *terrain_get_node_at(int x, int y);
/**
 * Gets the material of the pixel at the given coordinates
 * @param x Pixel x position
 * @param y Pixel y position
 * @param edge_mat Material to use for out of bounds pixels.
 * @return The material of the pixel
 */
TerrainPixel terrain_get_pixel(int x, int y, TerrainPixel edge);
/**
 * Tests how solid a pixel should be considered for purposes of collision detection with players and projectiles
 * @param x Pixel x position
 * @param y Pixel y position
 * @param edge_val Value to use for out of bounds pixels.
 * @param dynamic_mod Multiplier applied to solidness of moving pixels.
 * @return 1 if solid, 0 if completely empty, value on interval (0.0, 1.0) for everything in-between.
 */
double terrain_get_pixel_solidness(int x, int y, double edge_val, double dynamic_mod);
#endif //CEXT_TERRAIN_H
