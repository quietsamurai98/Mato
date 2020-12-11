#ifndef CEXT_TYPES_H
#define CEXT_TYPES_H

#include "stdint.h"
#include <stdbool.h>

typedef uint8_t byte;
typedef union {
    uint32_t m_color;
    struct {
        byte r: 8;
        byte g: 8;
        byte b: 8;
        byte a: 8;

    } ch;

} Color;
typedef struct {
    double r, g, b, a;

} DblColor;
typedef struct {
    struct {
        double vertical;
        double horizontal;
    } move;

} UserInput;
typedef struct {
    double    px, py; //X and Y position (subpixel resolution for smooth velocity support)
    int       w, h; //Sprite width and height (for collision detection and rendering)
    DblColor  color; //Player's color
    int       hp, hp_max; //Player health data
    int       hp_delta; //A running sum of all the damage taken by the player this tick. Negative values => Healing
    Color     *sprite; //Player sprite
    Color     *collision_mask; //Collision mask loaded as an image
    UserInput input; //Player input for this tick
    struct {
        double acc_x;
        double acc_y;
        double vel_x;
        double vel_y;

    }   physics_data;
    int facing; //-1 if facing left, 1 if facing right

} Player;

typedef struct {
    byte type : 6;
    byte has_moved: 1;
    byte needs_update: 1;
} TerrainPixel;

typedef struct TerrainTreeNode {
    struct {
        bool need_updating: 1;
        bool terminal_node: 1;
        unsigned int parent_on_left: 1;
        unsigned int parent_on_top: 1;
    }   metadata;
    int tp_offset;
    int tp_x;
    int tp_y;
    struct TerrainTreeNode *parent;
    struct TerrainTreeNode *childNW;
    struct TerrainTreeNode *childNE;
    struct TerrainTreeNode *childSW;
    struct TerrainTreeNode *childSE;
} TerrainTreeNode;
#endif //CEXT_TYPES_H
