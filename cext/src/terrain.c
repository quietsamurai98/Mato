#include <string.h>
#include "globals.h"
#include "terrain.h"
#include "../lib/FastNoiseLite.h"
#include "xorshift.h"

TerrainPixel terrain_get_pixel(int x, int y, TerrainPixel edge) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return edge;
    return TERRAIN[y * WIDTH + x];
}

void terrain_set_pixel(int x, int y, TerrainPixel terrain_pixel) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return;
    TERRAIN[y * WIDTH + x] = terrain_pixel;
}

static TerrainPixel terrain_generator_dirt(int x, int y, fnl_state noise, double depth) {
    double sx          = x;
    double sy          = y * 2;
    double sx_n        = sx / (WIDTH); // sx, normalized to [0..1]
    double sy_n        = sy / (HEIGHT * 2); // sy, normalized to [0..1]
    double sy_n2       = sy_n * sy_n;
    double sy_n3       = sy_n2 * sy_n;
    double sy_n4       = sy_n2 * sy_n2;
    double sy_n5       = sy_n3 * sy_n2;
    double sy_n6       = sy_n3 * sy_n3;
    double sy_n7       = sy_n4 * sy_n3;
    double sy_n8       = sy_n4 * sy_n4;
    double sy_n9       = sy_n5 * sy_n4;
    double sy_n10      = sy_n5 * sy_n5;
//  double threshold = pow(fabs(0.5 - sy_n) / sy_n, 2) * pow(sin(M_PI * sy_n), 3) * 2;
//  double threshold   = -2 * cos(2 * M_PI * (sy_n - 0.5));
    double threshold   = 1.24 + -22.7 * sy_n + 40.4 * sy_n2 + 431 * sy_n3 + -3619 * sy_n4 + 11792 * sy_n5 + -18638 * sy_n6 + 14178 * sy_n7 + -4161 * sy_n8;
    double noise_freq  = 1;
    double noise_shift = 0; // Positive => More common blobs. Negative => Less common blobs.
    if (fnlGetNoise3D(&noise, sx * noise_freq, sy * noise_freq, depth) + noise_shift > threshold) return TERRAIN_NONE;
    return TERRAIN_DIRT;
}

static TerrainPixel terrain_generator_sand(int x, int y, fnl_state noise, double depth) {
    TerrainPixel initial_terrain = terrain_get_pixel(x, y, TERRAIN_NONE);
    if (initial_terrain.type == TERRAIN_NONE_TYPE || ((
            terrain_get_pixel(x - 1, y + 1, TERRAIN_NONE).type == TERRAIN_NONE_TYPE ||
            terrain_get_pixel(x + 1, y + 1, TERRAIN_NONE).type == TERRAIN_NONE_TYPE ||
            terrain_get_pixel(x + 0, y + 1, TERRAIN_NONE).type == TERRAIN_NONE_TYPE
    )))
        return initial_terrain;

    double sx          = x;
    double sy          = y * 2; // Squash the terrain vertically to reduce intensity of slopes
    double sx_n        = sx / WIDTH; // sx, normalized to [0..1]
    double sy_n        = sy / (HEIGHT * 2); // sy, normalized to [0..1]
    //double threshold   = -sy_n;
    double threshold   = -1.5 * (sy_n - 0.3);
    double noise_freq  = 1;
    double noise_scale = 1; // AKA Blob rarity
    if (fnlGetNoise3D(&noise, noise_freq * sx, noise_freq * sy, depth) * noise_scale > threshold) return initial_terrain;
    return TERRAIN_SAND;
}

void terrain_generate(int base_seed, int smooth_seed) {
    fnl_state dirt_noise = fnlCreateState();
    dirt_noise.seed         = base_seed;
    dirt_noise.noise_type   = FNL_NOISE_OPENSIMPLEX2;
    dirt_noise.gain         = 0.5f;
    dirt_noise.octaves      = 1;
    dirt_noise.frequency    = 0.01f;
    dirt_noise.fractal_type = FNL_FRACTAL_NONE;
    xor_srand(base_seed);
    int       sand_seed  = xor_rand_int32();
    fnl_state sand_noise = fnlCreateState();
    sand_noise.seed         = sand_seed;
    sand_noise.noise_type   = FNL_NOISE_OPENSIMPLEX2;
    sand_noise.gain         = 0.5f;
    sand_noise.octaves      = 1;
    sand_noise.frequency    = 0.1f;
    sand_noise.fractal_type = FNL_FRACTAL_NONE;

    memset(TERRAIN, 0, PIXELS * sizeof(byte));
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            TERRAIN[y * WIDTH + x] = terrain_generator_dirt(x, y, dirt_noise, smooth_seed);
        }
    }
    for (int y = HEIGHT - 1; y >= 0; --y) {
        for (int x = 0; x < WIDTH; ++x) {
            TERRAIN[y * WIDTH + x] = terrain_generator_sand(x, y, dirt_noise, smooth_seed);
        }
    }
}

int sand_can_occupy(int x, int y) {
    byte mat = terrain_get_pixel(x, y, TERRAIN_NONE).type;
    return mat == TERRAIN_NONE_TYPE || mat == TERRAIN_XHST_TYPE || mat == TERRAIN_SMKE_TYPE;
}

void update_sand_at(int x, int y) {
    TerrainPixel tp = terrain_get_pixel(x, y, TERRAIN_NONE);
    if (tp.type == TERRAIN_SAND_TYPE && !tp.has_moved) {
        byte open_spaces = 0x00;
        if (sand_can_occupy(x - 1, y - 1)) open_spaces |= 0b10000000u;
        if (sand_can_occupy(x + 0, y - 1)) open_spaces |= 0b01000000u;
        if (sand_can_occupy(x + 1, y - 1)) open_spaces |= 0b00100000u;
        if (sand_can_occupy(x - 1, y + 0)) open_spaces |= 0b00010000u;
        if (sand_can_occupy(x + 1, y + 0)) open_spaces |= 0b00001000u;
        if (sand_can_occupy(x - 1, y + 1)) open_spaces |= 0b00000100u;
        if (sand_can_occupy(x + 0, y + 1)) open_spaces |= 0b00000010u;
        if (sand_can_occupy(x + 1, y + 1)) open_spaces |= 0b00000001u;
        //Sand only falls down, so always consider the space above occupied
        open_spaces &= 0b00011111u;

        int dx = 0, dy = 0;

        if (open_spaces & 0b00000111u) {
            dy = 1;
            //Move down
            switch (open_spaces & 0b00000111u) {
                case 0b111: dx = rand_sample((const int[]) {-1, 0, 0, 0, 1}, 5);
                    break;
                case 0b110: dx = rand_sample((const int[]) {-1, 0, 0, 0, 0}, 5);
                    break;
                case 0b011: dx = rand_sample((const int[]) {0, 0, 0, 0, 1}, 5);
                    break;
                case 0b010: dx = 0;
                    break;
                case 0b101: dx = rand_sample((const int[]) {-1, 1}, 2);
                    break;
                case 0b100: dx = -1;
                    break;
                case 0b001: dx = 1;
                    break;
                default: dx = 0;
            }
        }
        if (dx || dy) {
            if (dx < -1) dx = -1; else if (dx > 1) dx = 1;
            tp.has_moved = 0xFF;
            TerrainPixel tmp = terrain_get_pixel(x + dx, y + dy, TERRAIN_NONE);
            terrain_set_pixel(x + dx, y + dy, tp);
            terrain_set_pixel(x, y, tmp);
        }
    }
}

void terrain_update_sand() {
//#pragma omp parallel for
    //FIXME: Need to move from the bottom up and stagger the columns to avoid updating the same particle multiple times.
    // Maybe we should keep track of which particles still need to be updated?
    // We should *definitely* switch to using a quadtree to reduce the number of pixel tests.
    for (int y = HEIGHT - 1; y >= 0; --y) {
        for (int x = 0; x < WIDTH; ++x) {
            update_sand_at((x * 2) % WIDTH + (x * 2) / WIDTH, y);
        }
    }
}

void update_xhst_at(int x, int y) {
    TerrainPixel tp = terrain_get_pixel(x, y, TERRAIN_NONE);
    if (tp.type == TERRAIN_XHST_TYPE && !tp.has_moved) {
        //Exhaust falls down, so always consider the space above occupied
        byte neighbors = 0b11100000u;
        if (terrain_get_pixel(x - 1, y - 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b10000000u;
        if (terrain_get_pixel(x + 0, y - 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b01000000u;
        if (terrain_get_pixel(x + 1, y - 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00100000u;
        if (terrain_get_pixel(x - 1, y + 0, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00010000u;
        if (terrain_get_pixel(x + 1, y + 0, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00001000u;
        if (terrain_get_pixel(x - 1, y + 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00000100u;
        if (terrain_get_pixel(x + 0, y + 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00000010u;
        if (terrain_get_pixel(x + 1, y + 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00000001u;
        byte open_spaces = ~neighbors;

        int dx = 0, dy = 0;

        if (open_spaces & 0b00000111u) {
            dy = 1;
            //Move down
            switch (open_spaces & 0b00000111u) {
                case 0b111: dx = rand_sample((const int[]) {-1, 0, 1}, 3);
                    break;
                case 0b110: dx = rand_sample((const int[]) {-1, 0}, 2);
                    break;
                case 0b011: dx = rand_sample((const int[]) {0, 1}, 2);
                    break;
                case 0b010: dx = 0;
                    break;
                case 0b101: dx = rand_sample((const int[]) {-1, 1}, 2);
                    break;
                case 0b100: dx = -1;
                    break;
                case 0b001: dx = 1;
                    break;
                default: dx = 0;
            }
        }
        if (dx || dy) {
            if (dx < -1) dx = -1; else if (dx > 1) dx = 1;
            tp.has_moved = 0xFF;
            TerrainPixel tmp = terrain_get_pixel(x + dx, y + dy, TERRAIN_NONE);
            if (xor_rand_double() < 0.9) {
                terrain_set_pixel(x + dx, y + dy, tp);
            } else if (xor_rand_double() < 0.5) {
                terrain_set_pixel(x + dx, y + dy, TERRAIN_SMKE);

            }
            terrain_set_pixel(x, y, tmp);
        } else {
            if (!sand_can_occupy(x - 1, y - 1) ||
                !sand_can_occupy(x + 0, y - 1) ||
                !sand_can_occupy(x + 1, y - 1) ||
                !sand_can_occupy(x - 1, y + 0) ||
                !sand_can_occupy(x + 1, y + 0) ||
                !sand_can_occupy(x - 1, y + 1) ||
                !sand_can_occupy(x + 0, y + 1) ||
                !sand_can_occupy(x + 1, y + 1))
                terrain_set_pixel(x, y, TERRAIN_NONE);
        }
    }
}
int smke_can_occupy(int x, int y) {
    byte mat = terrain_get_pixel(x, y, TERRAIN_NONE).type;
    return mat == TERRAIN_NONE_TYPE || mat == TERRAIN_XHST_TYPE;
}
void update_smke_at(int x, int y) {
    TerrainPixel tp = terrain_get_pixel(x, y, TERRAIN_NONE);
    if (tp.type == TERRAIN_SMKE_TYPE && !tp.has_moved) {
        //Smoke rises, so always consider the space below occupied
        byte neighbors = 0b00000111u;
        if (terrain_get_pixel(x - 1, y - 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b10000000u;
        if (terrain_get_pixel(x + 0, y - 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b01000000u;
        if (terrain_get_pixel(x + 1, y - 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00100000u;
        if (terrain_get_pixel(x - 1, y + 0, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00010000u;
        if (terrain_get_pixel(x + 1, y + 0, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00001000u;
        if (terrain_get_pixel(x - 1, y + 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00000100u;
        if (terrain_get_pixel(x + 0, y + 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00000010u;
        if (terrain_get_pixel(x + 1, y + 1, TERRAIN_NONE).type != TERRAIN_NONE_TYPE) neighbors |= 0b00000001u;
        byte open_spaces = ~neighbors;

        int i = 0;

        if (open_spaces & 0b11111000u) {
            // @formatter:off
            switch ((open_spaces & 0b11111000u) >> 3) {
                case 0b00000: i = rand_sample((const int[]) {0,0,0,0,0,0}, 6); break;
                case 0b00001: i = rand_sample((const int[]) {0,1,0,0,0,0}, 6); break;
                case 0b00010: i = rand_sample((const int[]) {0,0,2,0,0,0}, 6); break;
                case 0b00011: i = rand_sample((const int[]) {0,1,2,0,0,0}, 6); break;
                case 0b00100: i = rand_sample((const int[]) {0,0,0,3,0,0}, 6); break;
                case 0b00101: i = rand_sample((const int[]) {0,1,0,3,0,0}, 6); break;
                case 0b00110: i = rand_sample((const int[]) {0,0,2,3,0,0}, 6); break;
                case 0b00111: i = rand_sample((const int[]) {0,1,2,3,0,0}, 6); break;
                case 0b01000: i = rand_sample((const int[]) {0,0,0,0,4,0}, 6); break;
                case 0b01001: i = rand_sample((const int[]) {0,1,0,0,4,0}, 6); break;
                case 0b01010: i = rand_sample((const int[]) {0,0,2,0,4,0}, 6); break;
                case 0b01011: i = rand_sample((const int[]) {0,1,2,0,4,0}, 6); break;
                case 0b01100: i = rand_sample((const int[]) {0,0,0,3,4,0}, 6); break;
                case 0b01101: i = rand_sample((const int[]) {0,1,0,3,4,0}, 6); break;
                case 0b01110: i = rand_sample((const int[]) {0,0,2,3,4,0}, 6); break;
                case 0b01111: i = rand_sample((const int[]) {0,1,2,3,4,0}, 6); break;
                case 0b10000: i = rand_sample((const int[]) {0,0,0,0,0,5}, 6); break;
                case 0b10001: i = rand_sample((const int[]) {0,1,0,0,0,5}, 6); break;
                case 0b10010: i = rand_sample((const int[]) {0,0,2,0,0,5}, 6); break;
                case 0b10011: i = rand_sample((const int[]) {0,1,2,0,0,5}, 6); break;
                case 0b10100: i = rand_sample((const int[]) {0,0,0,3,0,5}, 6); break;
                case 0b10101: i = rand_sample((const int[]) {0,1,0,3,0,5}, 6); break;
                case 0b10110: i = rand_sample((const int[]) {0,0,2,3,0,5}, 6); break;
                case 0b10111: i = rand_sample((const int[]) {0,1,2,3,0,5}, 6); break;
                case 0b11000: i = rand_sample((const int[]) {0,0,0,0,4,5}, 6); break;
                case 0b11001: i = rand_sample((const int[]) {0,1,0,0,4,5}, 6); break;
                case 0b11010: i = rand_sample((const int[]) {0,0,2,0,4,5}, 6); break;
                case 0b11011: i = rand_sample((const int[]) {0,1,2,0,4,5}, 6); break;
                case 0b11100: i = rand_sample((const int[]) {0,0,0,3,4,5}, 6); break;
                case 0b11101: i = rand_sample((const int[]) {0,1,0,3,4,5}, 6); break;
                case 0b11110: i = rand_sample((const int[]) {0,0,2,3,4,5}, 6); break;
                case 0b11111: i = rand_sample((const int[]) {0,1,2,3,4,5}, 6); break;
                default: i = 0;
            }
            // @formatter:on
        }
        int dx = ((const int[]) {0, 1, -1, 1, 0, -1})[i];
        int dy = ((const int[]) {0, 0, 0, -1, -1, -1})[i];
        if (dx || dy) {
            if (dx < -1) dx = -1; else if (dx > 1) dx = 1;
            tp.has_moved     = 0xFF;
            TerrainPixel tmp = terrain_get_pixel(x + dx, y + dy, TERRAIN_NONE);
            if (xor_rand_double() < 0.99) {
                terrain_set_pixel(x + dx, y + dy, tp);
            }
            terrain_set_pixel(x, y, tmp);
        } else {
            if (!smke_can_occupy(x - 1, y - 1) ||
                !smke_can_occupy(x + 0, y - 1) ||
                !smke_can_occupy(x + 1, y - 1) ||
                !smke_can_occupy(x - 1, y + 0) ||
                !smke_can_occupy(x + 1, y + 0) ||
                !smke_can_occupy(x - 1, y + 1) ||
                !smke_can_occupy(x + 0, y + 1) ||
                !smke_can_occupy(x + 1, y + 1))
                terrain_set_pixel(x, y, TERRAIN_NONE);
        }
    }
}

void terrain_update_gas() {
    //FIXME: Need to stagger x and y to avoid updating the same particle multiple times.
    // Maybe we should keep track of which particles still need to be updated?
    // We should *definitely* switch to using a quadtree to reduce the number of pixel tests.
    for (int y = HEIGHT - 1; y >= 0; --y) {
        for (int x = 0; x < WIDTH; ++x) {
            update_smke_at((x * 2) % WIDTH + (x * 2) / WIDTH, y);
            update_xhst_at((x * 2) % WIDTH + (x * 2) / WIDTH, y);
        }
    }
}

double terrain_get_pixel_solidness(int x, int y, double edge_val, double dynamic_mod) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return edge_val;
    TerrainPixel tp = terrain_get_pixel(x, y, TERRAIN_NONE);
    // Dirt is always solid
    if (tp.type == TERRAIN_DIRT_TYPE) return 1.0;
    // Sand is solid if stable, but less solid if moving.
    if (tp.type == TERRAIN_SAND_TYPE) {
        if (tp.has_moved) return 0.0;
        return 1.0;
    } //TODO: Falling sand should be less solid than static sand.
    // Exhaust is never solid
    if (tp.type == TERRAIN_XHST_TYPE) return 0.0;
    // Smoke is never solid
    if (tp.type == TERRAIN_SMKE_TYPE) return 0.0;
    return 0.0;
}

void terrain_update() {
    for (int o = 0; o < PIXELS; ++o) {
        TERRAIN[o].has_moved = 0x00;
    }
    terrain_update_sand();
    terrain_update_gas();
}
