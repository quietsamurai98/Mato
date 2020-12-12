#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "globals.h"
#include "terrain.h"
#include "../lib/FastNoiseLite.h"
#include "xorshift.h"
#include "utils.h"


static TerrainTreeNode terrain_tree_root = {0};

void initialize_terrain_tree(TerrainTreeNode *node, int depth) {
    TerrainTreeNode *p = node;
    unsigned int    x  = 0;
    unsigned int    y  = 0;
    for (int        i  = 0; i < depth; ++i) {
        x |= p->metadata.parent_on_left << i;
        y |= p->metadata.parent_on_top << i;
        p = p->parent;
    }
    node->tp_offset = (int) (y * TERRAIN_SIZE + x);
    node->tp_x      = (int) x;
    node->tp_y      = (int) y;
    if (depth == TERRAIN_SPAN) {
        node->metadata.terminal_node = true;
    } else {
        node->childNW = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
        *node->childNW = (TerrainTreeNode) {
                .metadata = {
                        .need_updating = true,
                        .terminal_node = false,
                        .parent_on_left = 0,
                        .parent_on_top = 0,
                },
                .tp_offset = -1,
                .tp_x = -1,
                .tp_y = -1,
                .parent = node,
                .childNW = NULL,
                .childNE = NULL,
                .childSW = NULL,
                .childSE = NULL,
        };
        node->childNE = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
        *node->childNE = (TerrainTreeNode) {
                .metadata = {
                        .need_updating = true,
                        .terminal_node = false,
                        .parent_on_left = 1,
                        .parent_on_top = 0,
                },
                .tp_offset = -1,
                .tp_x = -1,
                .tp_y = -1,
                .parent = node,
                .childNW = NULL,
                .childNE = NULL,
                .childSW = NULL,
                .childSE = NULL,
        };
        node->childSW = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
        *node->childSW = (TerrainTreeNode) {
                .metadata = {
                        .need_updating = true,
                        .terminal_node = false,
                        .parent_on_left = 0,
                        .parent_on_top = 1,
                },
                .tp_offset = -1,
                .tp_x = -1,
                .tp_y = -1,
                .parent = node,
                .childNW = NULL,
                .childNE = NULL,
                .childSW = NULL,
                .childSE = NULL,
        };
        node->childSE = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
        *node->childSE = (TerrainTreeNode) {
                .metadata = {
                        .need_updating = true,
                        .terminal_node = false,
                        .parent_on_left = 1,
                        .parent_on_top = 1,
                },
                .tp_offset = -1,
                .tp_x = -1,
                .tp_y = -1,
                .parent = node,
                .childNW = NULL,
                .childNE = NULL,
                .childSW = NULL,
                .childSE = NULL,
        };
        initialize_terrain_tree(node->childNW, depth + 1);
        initialize_terrain_tree(node->childNE, depth + 1);
        initialize_terrain_tree(node->childSW, depth + 1);
        initialize_terrain_tree(node->childSE, depth + 1);
    }
}

void destroy_terrain_tree(TerrainTreeNode *node) {
    if (!node) return;
    if (!node->metadata.terminal_node) {
        destroy_terrain_tree(node->childNW);
        destroy_terrain_tree(node->childNE);
        destroy_terrain_tree(node->childSW);
        destroy_terrain_tree(node->childSE);
    }
    if (node->parent) free(node);
}

TerrainTreeNode *terrain_get_node_at(int x, int y) {
    if (x < 0 || y < 0 || x >= TERRAIN_SIZE || y >= TERRAIN_SIZE) {
        return NULL;
    }
    unsigned int    tx    = x;
    unsigned int    ty    = y;
    TerrainTreeNode *node = &terrain_tree_root;
    TerrainTreeNode *last = NULL;
    for (int        i     = TERRAIN_SPAN - 1; i >= 0; --i) {
        bool left = (tx & 0x1u << i) != 0;
        bool top  = (ty & (0x1u << i)) != 0;
        if (!node) {
            printf("beans");
            fflush(stdout);
        }
        last = node;
        if (!top) {
            if (!left) {
//                printf("SE ");fflush(stdout);
                if (!node->childSE) {
                    node->childSE = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
                    *node->childSE = (TerrainTreeNode) {
                            .metadata = {
                                    .need_updating = false,
                                    .terminal_node = false,
                                    .parent_on_left = 1,
                                    .parent_on_top = 1,
                            },
                            .tp_offset = -1,
                            .tp_x = -1,
                            .tp_y = -1,
                            .parent = node,
                            .childNW = NULL,
                            .childNE = NULL,
                            .childSW = NULL,
                            .childSE = NULL,
                    };
                }
                node = node->childSE;
            } else {
//                printf("SW ");fflush(stdout);
                if (!node->childSW) {
                    node->childSW = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
                    *node->childSW = (TerrainTreeNode) {
                            .metadata = {
                                    .need_updating = false,
                                    .terminal_node = false,
                                    .parent_on_left = 0,
                                    .parent_on_top = 1,
                            },
                            .tp_offset = -1,
                            .tp_x = -1,
                            .tp_y = -1,
                            .parent = node,
                            .childNW = NULL,
                            .childNE = NULL,
                            .childSW = NULL,
                            .childSE = NULL,
                    };
                }
                node = node->childSW;
            }
        } else {
            if (!left) {
//                printf("NE ");fflush(stdout);
                if (!node->childNE) {
                    node->childNE = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
                    *node->childNE = (TerrainTreeNode) {
                            .metadata = {
                                    .need_updating = false,
                                    .terminal_node = false,
                                    .parent_on_left = 1,
                                    .parent_on_top = 0,
                            },
                            .tp_offset = -1,
                            .tp_x = -1,
                            .tp_y = -1,
                            .parent = node,
                            .childNW = NULL,
                            .childNE = NULL,
                            .childSW = NULL,
                            .childSE = NULL,
                    };
                }
                node = node->childNE;
            } else {
//                printf("NW ");fflush(stdout);
                if (!node->childNW) {
                    node->childNW = (TerrainTreeNode *) malloc(sizeof(TerrainTreeNode));
                    *node->childNW = (TerrainTreeNode) {
                            .metadata = {
                                    .need_updating = false,
                                    .terminal_node = false,
                                    .parent_on_left = 0,
                                    .parent_on_top = 0,
                            },
                            .tp_offset = -1,
                            .tp_x = -1,
                            .tp_y = -1,
                            .parent = node,
                            .childNW = NULL,
                            .childNE = NULL,
                            .childSW = NULL,
                            .childSE = NULL,
                    };
                }
                node = node->childNW;
            }
        }
    }
    if (node->tp_offset == -1) {
        node->tp_offset              = y * TERRAIN_SIZE + x;
        node->tp_x                   = x;
        node->tp_y                   = y;
        node->metadata.terminal_node = true;
    }
    return node;
}

TerrainPixel terrain_get_pixel(int x, int y, TerrainPixel edge) {
    if (x < 0 || y < 0 || x >= TERRAIN_SIZE || y >= TERRAIN_SIZE) return edge;
    return TERRAIN[y * TERRAIN_SIZE + x];
}

void terrain_set_pixel(int x, int y, TerrainPixel terrain_pixel, bool should_update) {
    if (x < 0 || y < 0 || x >= TERRAIN_SIZE || y >= TERRAIN_SIZE) return;
    TERRAIN[y * TERRAIN_SIZE + x] = terrain_pixel;
    if (should_update)

        for (int tx = x - 1; tx <= x + 1; ++tx) {
            for (int ty = y - 1; ty <= y + 1; ++ty) {
                if (tx < 0 || ty < 0 || tx >= TERRAIN_SIZE || ty >= TERRAIN_SIZE) continue;
                TerrainPixel *tp = &TERRAIN[ty * TERRAIN_SIZE + tx];
                if (tp->type != TERRAIN_NONE_TYPE && tp->type != TERRAIN_DIRT_TYPE) {
                    TerrainTreeNode *node = terrain_get_node_at(tx, ty);
                    if (node != NULL) {
                        tp->needs_update |= should_update;
                        node->metadata.need_updating = tp->needs_update;
                        node = node->parent;
                        while (node != NULL && !node->metadata.need_updating) {
                            node->metadata.need_updating = 1;
                            node = node->parent;
                        }
                    }
                }
            }
        }
}

void terrain_refresh_quadtree(TerrainTreeNode *node) {
    if (!node) return;
    if (node->metadata.terminal_node) {
        TerrainPixel *tp = &TERRAIN[node->tp_offset];
        node->metadata.need_updating = tp->needs_update != 0;
        tp->has_moved                = false;
        tp->needs_update             = 0;
    } else {
        if (node->metadata.need_updating) {
            terrain_refresh_quadtree(node->childNW);
            terrain_refresh_quadtree(node->childNE);
            terrain_refresh_quadtree(node->childSW);
            terrain_refresh_quadtree(node->childSE);
            node->metadata.need_updating = (node->childNW && node->childNW->metadata.need_updating) ||
                                           (node->childNE && node->childNE->metadata.need_updating) ||
                                           (node->childSW && node->childSW->metadata.need_updating) ||
                                           (node->childSE && node->childSE->metadata.need_updating);
        } else if (node->parent) {
            if (node->metadata.parent_on_top) {
                if (node->metadata.parent_on_left) {
                    node->parent->childSE = NULL;
                } else {
                    node->parent->childSW = NULL;
                }
            } else {
                if (node->metadata.parent_on_left) {
                    node->parent->childNE = NULL;
                } else {
                    node->parent->childNW = NULL;
                }
            }
            destroy_terrain_tree(node);
            node = NULL;
        }
    }
}

static TerrainPixel terrain_generator_dirt(int x, int y, fnl_state noise) {
    double sx          = x;
    double sy          = y * 2;
    double sx_n        = sx / (TERRAIN_SIZE); // sx, normalized to [0..1]
    double sy_n        = sy / (TERRAIN_SIZE * 2); // sy, normalized to [0..1]
//    double sy_n2       = sy_n * sy_n;
//    double sy_n3       = sy_n2 * sy_n;
//    double sy_n4       = sy_n2 * sy_n2;
//    double sy_n5       = sy_n3 * sy_n2;
//    double sy_n6       = sy_n3 * sy_n3;
//    double sy_n7       = sy_n4 * sy_n3;
//    double sy_n8       = sy_n4 * sy_n4;
//    double sy_n9       = sy_n5 * sy_n4;
//    double sy_n10      = sy_n5 * sy_n5;
////  double threshold = pow(fabs(0.5 - sy_n) / sy_n, 2) * pow(sin(M_PI * sy_n), 3) * 2;
////  double threshold   = -2 * cos(2 * M_PI * (sy_n - 0.5));
//    double threshold   = 1.95 + -36.3 * sy_n + 54.2 * sy_n2 + 1623 * sy_n3 + -11269 * sy_n4 + 31605 * sy_n5 + -44153 * sy_n6 + 30401 * sy_n7 + -8224 * sy_n8;
    double threshold   = fabs(sy_n - 0.5) > 0.45 ? 2 : -0.6;
    double noise_shift = 0; // Positive => More common blobs. Negative => Less common blobs.
    threshold += noise_shift;
    if (threshold <= -1 || fnlGetNoise2D(&noise, sx, sy) > threshold) return TERRAIN_NONE;
    return TERRAIN_DIRT;
    return TERRAIN_SAND;
}

static TerrainPixel terrain_generator_sand(int x, int y, fnl_state noise) {
    TerrainPixel initial_terrain = terrain_get_pixel(x, y, TERRAIN_NONE);
    if (initial_terrain.type == TERRAIN_NONE_TYPE || ((
            terrain_get_pixel(x - 1, y + 1, TERRAIN_NONE).type == TERRAIN_NONE_TYPE ||
            terrain_get_pixel(x + 1, y + 1, TERRAIN_NONE).type == TERRAIN_NONE_TYPE ||
            terrain_get_pixel(x + 0, y + 1, TERRAIN_NONE).type == TERRAIN_NONE_TYPE
    )))
        return initial_terrain;
    double sx          = x;
    double sy          = y * 2;
    double sx_n        = sx / (TERRAIN_SIZE); // sx, normalized to [0..1]
    double sy_n        = sy / (TERRAIN_SIZE * 2); // sy, normalized to [0..1]
//    double sy_n2     = sy_n * sy_n;
//    double sy_n3     = sy_n2 * sy_n;
//    double sy_n4     = sy_n2 * sy_n2;
//    double sy_n5     = sy_n3 * sy_n2;
//    double sy_n6     = sy_n3 * sy_n3;
//    double sy_n7     = sy_n4 * sy_n3;
//    double sy_n8     = sy_n4 * sy_n4;
//    double sy_n9     = sy_n5 * sy_n4;
//    double sy_n10    = sy_n5 * sy_n5;
////  double threshold = pow(fabs(0.5 - sy_n) / sy_n, 2) * pow(sin(M_PI * sy_n), 3) * 2;
////  double threshold   = -2 * cos(2 * M_PI * (sy_n - 0.5));
//    double threshold = -1.02 + 1.68 * sy_n + 8.3 * sy_n2 + -101 * sy_n3 + 129 * sy_n4 + 633 * sy_n5 + -1891 * sy_n6 + 1825 * sy_n7 + -605 * sy_n8;
//    threshold = (threshold + 1) / 2;
//    threshold *= 0.5;
//    threshold = threshold * 2 - 1;
    double threshold   = -0.875;
    double noise_shift = 0; // Positive => More common blobs. Negative => Less common blobs.
    threshold -= noise_shift;
    if (threshold <= -1 || fnlGetNoise2D(&noise, sx, sy) > threshold) return initial_terrain;
    return TERRAIN_SAND;
}

void terrain_generate(int seed) {
    if (terrain_tree_root.childNW != NULL) {
        destroy_terrain_tree(&terrain_tree_root);
    }
    terrain_tree_root    = (TerrainTreeNode) {
            .metadata = {
                    .need_updating = true,
                    .terminal_node = false,
                    .parent_on_left = 0,
                    .parent_on_top = 0,
            },
            .tp_offset = -1,
            .tp_x = -1,
            .tp_y = -1,
            .parent = NULL,
            .childNW = NULL,
            .childNE = NULL,
            .childSW = NULL,
            .childSE = NULL,
    };
//    initialize_terrain_tree(&terrain_tree_root, 0);
    fnl_state dirt_noise = fnlCreateState();
    dirt_noise.seed         = seed;
    dirt_noise.noise_type   = FNL_NOISE_OPENSIMPLEX2;
    dirt_noise.gain         = 0.5f;
    dirt_noise.octaves      = 1;
    dirt_noise.frequency    = 0.0025f;
    dirt_noise.fractal_type = FNL_FRACTAL_NONE;
    xor_srand(seed);
    int       sand_seed  = xor_rand_int32();
    fnl_state sand_noise = fnlCreateState();
    sand_noise.seed         = sand_seed;
    sand_noise.noise_type   = FNL_NOISE_OPENSIMPLEX2;
    sand_noise.gain         = 0.5f;
    sand_noise.octaves      = 1;
    sand_noise.frequency    = 0.005f;
    sand_noise.fractal_type = FNL_FRACTAL_NONE;

    memset(TERRAIN, 0, TERRAIN_PIXELS * sizeof(byte));
    for (int y = 0; y < TERRAIN_SIZE; ++y) {
        for (int x = 0; x < TERRAIN_SIZE; ++x) {
            TERRAIN[y * TERRAIN_SIZE + x] = terrain_generator_dirt(x, y, dirt_noise);
        }
    }
    for (int y = TERRAIN_SIZE - 1; y >= 0; --y) {
        for (int x = 0; x < TERRAIN_SIZE; ++x) {
            TerrainPixel tp = terrain_generator_sand(x, y, sand_noise);
            if (tp.type == TERRAIN_SAND_TYPE) {
                terrain_set_pixel(x, y, tp, true);
            } else {
                TERRAIN[y * TERRAIN_SIZE + x] = tp;
            }
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
            tp.has_moved = true;
            TerrainPixel tmp = terrain_get_pixel(x + dx, y + dy, TERRAIN_NONE);
            terrain_set_pixel(x + dx, y + dy, tp, true);
            terrain_set_pixel(x, y, tmp, true);
        } else {
            tp.has_moved    = false;
            tp.needs_update = false;
            terrain_set_pixel(x, y, tp, false);
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
            tp.has_moved = true;
            TerrainPixel tmp = terrain_get_pixel(x + dx, y + dy, TERRAIN_NONE);
            if (xor_rand_double() < 0.95) {
                terrain_set_pixel(x + dx, y + dy, tp, true);
            }
            terrain_set_pixel(x, y, tmp, true);
        } else {
            if (!sand_can_occupy(x - 1, y - 1) ||
                !sand_can_occupy(x + 0, y - 1) ||
                !sand_can_occupy(x + 1, y - 1) ||
                !sand_can_occupy(x - 1, y + 0) ||
                !sand_can_occupy(x + 1, y + 0) ||
                !sand_can_occupy(x - 1, y + 1) ||
                !sand_can_occupy(x + 0, y + 1) ||
                !sand_can_occupy(x + 1, y + 1))
                terrain_set_pixel(x, y, TERRAIN_NONE, false);
        }
    }
}

int smke_can_occupy(int x, int y) {
    byte mat = terrain_get_pixel(x, y, TERRAIN_NONE).type;
    return mat == TERRAIN_NONE_TYPE || mat == TERRAIN_XHST_TYPE || mat == TERRAIN_SMKE_TYPE;
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
            tp.has_moved     = true;
            TerrainPixel tmp = terrain_get_pixel(x + dx, y + dy, TERRAIN_NONE);
            if (xor_rand_double() < 0.99) {
                terrain_set_pixel(x + dx, y + dy, tp, true);
            }
            terrain_set_pixel(x, y, tmp, true);
        } else {
            if (!smke_can_occupy(x - 1, y - 1) ||
                !smke_can_occupy(x + 0, y - 1) ||
                !smke_can_occupy(x + 1, y - 1) ||
                !smke_can_occupy(x - 1, y + 0) ||
                !smke_can_occupy(x + 1, y + 0) ||
                !smke_can_occupy(x - 1, y + 1) ||
                !smke_can_occupy(x + 0, y + 1) ||
                !smke_can_occupy(x + 1, y + 1) ||
                xor_rand_double() > 0.99)
                terrain_set_pixel(x, y, TERRAIN_NONE, false);
            else
                terrain_set_pixel(x, y, tp, true);
        }
    }
}

void terrain_update_bottom_up() {
    //FIXME: Need to move from the bottom up to make convincing falling sand effect,
    // and stagger the columns to avoid updating the same particle multiple times.
    // Maybe we should keep track of which particles still need to be updated?
    // We should *definitely* switch to using a quadtree to reduce the number of pixel tests.
    for (int y = TERRAIN_SIZE - 1; y >= 0; --y) {
        for (int x = 0; x < TERRAIN_SIZE; ++x) {
            update_sand_at((x * 2) % TERRAIN_SIZE + (x * 2) / TERRAIN_SIZE, y);
            update_xhst_at((x * 2) % TERRAIN_SIZE + (x * 2) / TERRAIN_SIZE, y);
        }
    }
}

void terrain_update_quadtree_bottom_up(TerrainTreeNode *node) {
    if (!node) return;
    if (node->metadata.need_updating) {
        if (node->metadata.terminal_node) {
            update_sand_at(node->tp_x, node->tp_y);
            update_xhst_at(node->tp_x, node->tp_y);
        } else {
            if (xor_rand_double() < 0.5) {
                terrain_update_quadtree_bottom_up(node->childNW);
                terrain_update_quadtree_bottom_up(node->childNE);
                terrain_update_quadtree_bottom_up(node->childSW);
                terrain_update_quadtree_bottom_up(node->childSE);
            } else {
                terrain_update_quadtree_bottom_up(node->childNE);
                terrain_update_quadtree_bottom_up(node->childNW);
                terrain_update_quadtree_bottom_up(node->childSE);
                terrain_update_quadtree_bottom_up(node->childSW);
            }
        }
    }
}

void terrain_update_quadtree_top_down(TerrainTreeNode *node) {
    if (!node) return;
    if (node->metadata.need_updating) {
        if (node->metadata.terminal_node) {
            update_smke_at(node->tp_x, node->tp_y);
        } else {
            if (xor_rand_double() < 0.5) {
                terrain_update_quadtree_top_down(node->childSW);
                terrain_update_quadtree_top_down(node->childSE);
                terrain_update_quadtree_top_down(node->childNW);
                terrain_update_quadtree_top_down(node->childNE);
            } else {
                terrain_update_quadtree_top_down(node->childSE);
                terrain_update_quadtree_top_down(node->childSW);
                terrain_update_quadtree_top_down(node->childNE);
                terrain_update_quadtree_top_down(node->childNW);
            }
        }
    }
}

void terrain_update_top_down() {
    //FIXME: Need to stagger x and y to avoid updating the same particle multiple times.
    // Maybe we should keep track of which particles still need to be updated?
    // We should *definitely* switch to using a quadtree to reduce the number of pixel tests.
#pragma omp parallel for
    for (int y = 0; y < TERRAIN_SIZE; ++y) {
        for (int x = 0; x < TERRAIN_SIZE; ++x) {
            update_smke_at((x * 2) % TERRAIN_SIZE + (x * 2) / TERRAIN_SIZE, y);
        }
    }
}

double terrain_get_pixel_solidness(int x, int y, double edge_val, double dynamic_mod) {
    if (x < 0 || y < 0 || x >= TERRAIN_SIZE || y >= TERRAIN_SIZE) return edge_val;
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
//#pragma omp parallel for
//    for (int o = 0; o < TERRAIN_PIXELS; ++o) {
//        TERRAIN[o].has_moved = false;
//    }
//    terrain_update_bottom_up();
//    terrain_update_top_down();
    terrain_refresh_quadtree(&terrain_tree_root);
    terrain_update_quadtree_top_down(&terrain_tree_root);
    terrain_update_quadtree_bottom_up(&terrain_tree_root);
}
