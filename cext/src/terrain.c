#include <string.h>
#include "globals.h"
#include "terrain.h"
#include "../lib/FastNoiseLite.h"
#include "xorshift.h"

byte get_terrain_at_pixel(int x, int y, byte edge) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return edge;
    return TERRAIN[y * WIDTH + x];
}

void set_terrain_at_pixel(int x, int y, byte mat) {
    if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT) return;
    TERRAIN[y * WIDTH + x] = mat;
}

static byte terrain_generator_dirt(int x, int y, fnl_state noise, double depth) {
    if (y == 0) return 0x00;
    double sx        = x;
    double sy        = y * 2;
    double sx_n      = sx / (WIDTH); // sx, normalized to [0..1]
    double sy_n      = sy / (HEIGHT * 4); // sy, normalized to [0..1]
    double threshold = pow(fabs(0.5 - sy_n) / sy_n, 2) * pow(sin(M_PI * sy_n), 3) * 2;
    threshold = 2 * threshold - 1;
    double noise_freq  = 1;
    double noise_shift = 0; // Positive => More common blobs. Negative => Less common blobs.
    if (fnlGetNoise3D(&noise, sx * noise_freq, sy * noise_freq, depth) + noise_shift > threshold) return 0x01;
    return 0x00;
}

static byte terrain_generator_sand(int x, int y, fnl_state noise, double depth) {
    byte initial_terrain = get_terrain_at_pixel(x, y, 0x00);
    if (!initial_terrain) return 0x00;
    if (!get_terrain_at_pixel(x - 1, y + 1, 0x00) ||
        !get_terrain_at_pixel(x + 1, y + 1, 0x00) ||
        !get_terrain_at_pixel(x + 0, y + 1, 0x00))
        return initial_terrain;

    double sx          = x;
    double sy          = y * 2; // Squash the terrain vertically to reduce intensity of slopes
    double sx_n        = sx / WIDTH; // sx, normalized to [0..1]
    double sy_n        = sy / (HEIGHT * 2); // sy, normalized to [0..1]
    double threshold   = sy_n;
    double noise_freq  = 1;
    double noise_scale = 0.8; // AKA Blob rarity
    if (fnlGetNoise3D(&noise, noise_freq * sx, noise_freq * sy, depth) * noise_scale > threshold) return 0x02;
    return initial_terrain;
}

int terrain_generate(int base_seed, int smooth_seed) {
    fnl_state dirt_noise = fnlCreateState();
    dirt_noise.seed         = base_seed;
    dirt_noise.noise_type   = FNL_NOISE_OPENSIMPLEX2;
    dirt_noise.gain         = 0.5f;
    dirt_noise.octaves      = 1;
    dirt_noise.frequency    = 0.01f;
    dirt_noise.fractal_type = FNL_FRACTAL_NONE;
    xor_srand(base_seed);
    int       sand_seed  = xor_rand_signed();
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
    return 0;
}

void update_sand_at(int x, int y) {
    if (get_terrain_at_pixel(x, y, 0x00) == 0x02) {
        //Sand only falls down, so always consider the space above occupied
        byte neighbors = 0b11100000u;
        if (get_terrain_at_pixel(x - 1, y - 1, 0x00)) neighbors |= 0b10000000u;
        if (get_terrain_at_pixel(x + 0, y - 1, 0x00)) neighbors |= 0b01000000u;
        if (get_terrain_at_pixel(x + 1, y - 1, 0x00)) neighbors |= 0b00100000u;
        if (get_terrain_at_pixel(x - 1, y + 0, 0x00)) neighbors |= 0b00010000u;
        if (get_terrain_at_pixel(x + 1, y + 0, 0x00)) neighbors |= 0b00001000u;
        if (get_terrain_at_pixel(x - 1, y + 1, 0x00)) neighbors |= 0b00000100u;
        if (get_terrain_at_pixel(x + 0, y + 1, 0x00)) neighbors |= 0b00000010u;
        if (get_terrain_at_pixel(x + 1, y + 1, 0x00)) neighbors |= 0b00000001u;
        byte open_spaces = ~neighbors;

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
            set_terrain_at_pixel(x + dx, y + dy, 0x02);
            set_terrain_at_pixel(x, y, 0x00);
        }
    }
}

int terrain_update_sand() {
#pragma omp parallel for
    //FIXME: Need to move from the bottom up to avoid updating the same particle multiple times.
    // Maybe we should keep track of which particles still need to be updated?
    for (int y = HEIGHT - 1; y >= 0; --y) {
        for (int x = 0; x < WIDTH; ++x) {
            // If you don't stagger the column updates, sand will drift to the right
            update_sand_at((x * 2) % WIDTH + (x * 2) / WIDTH, y);
        }
    }
    return 0;
}
