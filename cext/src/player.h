#ifndef CEXT_PLAYER_H
#define CEXT_PLAYER_H

#include "types.h"

/**
 * A quick guide on naming conventions:
 *  All functions use the form `player_<VERB>_<NOUN>`
 *      VERBs and their meaning:
 *          construct - Allocate a pointer to a new player struct, and initialize the values.
 *          destroy - Deallocate a pointer to an existing player struct, and set the pointer to NULL.
 *          do - Do something that should only happen once per tick, like movement.
 *          calc - Calculate a portion of some common value stored in the player struct, like adding terrain damage to `hp_delta`.
 *      NOUNs can be just about anything, and are used to describe what the function is VERBing.
 */


/**
 * Allocates and initializes a new player struct.
 * @param x The initial X position of the player
 * @param y The initial Y position of the player
 * @param r The player's red color modifier
 * @param g The player's green color modifier
 * @param b The player's blue color modifier
 * @return A pointer to the newly created struct, or NULL if the memory couldn't be allocated. Must free with player_destroy.
 */
Player *player_initialize_player(double x, double y, double r, double g, double b);

/**
 * Frees the memory used by the given player pointer and sets the player pointer to NULL.
 * @param player A pointer to the player pointer. (Takes a pointer to the player pointer so we can set the latter to NULL)
 */
void player_destroy_player(Player **player);

/**
 * If the player is standing in mid-air, they are down-warped so they're standing on the surface immediately below them, and their y velocity is set to 0.
 * If the player is inside the terrain, they are up-warped so they're standing on the surface immediately above them, and their y velocity is set to 0.
 * If the player is standing on a valid surface, this function is essentially a noop (y position and y velocity are unchanged).
 * Modifies player->py and player->vy.
 * @param player The player to warp.
 */
void player_do_surface_warp(Player *player);

/**
 * General process:
 *  1: Update the player's position according to their current velocity.
 *      px+=vx; py+=vy; x=(int)px; y=(int)py;
 *  2: Update the player's velocity to reflect the terrain at this new position (excluding collision resolution).
 *      Moving through dynamic terrain (i.e. falling sand) will push the player according to the terrain's velocity.
 *      If the player is in the air, they accelerate downwards due to gravity.
 *  3: Resolve collisions with the terrain, updating the player's position and velocity.
 *      If the player's bottom-side is clipping into terrain, and there's room above their top-side, move up a little and add a little upwards velocity.
 *      If the player's top-side is clipping into terrain, and there's room below their bottom-side, move down a little and add a little downwards velocity.
 *      If the top and bottom of the player's left-side is clipping into terrain, and there's room to their right-side, move right a little and zero out the horizontal velocity.
 *      If the top and bottom of the player's right-side is clipping into terrain, and there's room to their left-side, move left a little and zero out the horizontal velocity.
 *      If any two opposite corners of the player are clipping into terrain, zero out the velocity. Player will need to dig their way out.
 * Modifies player->px, player->py, player->vx, and player->vy.
 * @param player The player to move.
 */
void player_do_movement(Player *player);

/**
 * Subtracts and zeros `hp_delta` from the player's hp.
 * Modifies player->hp and player->hp_delta.
 * @param player The player to damage.
 */
void player_do_damage(Player *player);

/**
 * Calculates the damage taken from the environment this tick.
 * Modifies player->hp_delta.
 * @param player
 */
void player_calc_damage_from_environment(Player *player);

#endif //CEXT_PLAYER_H
