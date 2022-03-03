/*
 * game module definition for CS50 nuggets game
 * stores all information about the current state of a game
 * used as a global variable in both server and client
 */

#ifndef __GAME_H
#define __GAME_H

#include <stdbool.h>
#include "grid.h"

/**************** global types ****************/
typedef struct game game_t;  // opaque to users of the module

/**************** functions **************/

/**************** getters **************/
grid_t* game_getGrid(game_t* game);
int* game_getPiles(game_t* game);
int* game_getPlayers(game_t* game);
int game_getRemainingGold(game_t* game);

/**************** setters ***************/
/* return false on failure, true on success */
bool game_setRemainingGold(game_t* game, int gold);

/* Note: the setGrid function calls grid_delete on the previous game->grid
 * to avoid memory leaks
 */
bool game_setGrid(game_t* game, grid_t* grid);

/**************** game_new *****************/
/* The game_new function allocates space for a new 'struct game' 
 * it only malloc's space for itself. All other memory must be allocated before
 * for example, a `game` takes non-null `grids` as parameters
 * so grid_new must be called on a grid before passing it to `game`
 * All memory allocated by the game and its grid, are freed in game_delete 
 */
game_t* game_new(int* piles, int* players, grid_t* grid);

/*************** game_subtractGold ***********/
/* Simple function to reduce a game's remaining gold by the given amount
 * returns -1 if game does not exist
 * returns the new value of game->remainingGold on success
 */
int game_subtractGold(game_t* game, int gold);

/************** game_delete ****************/
/* free's all memory assosciated with a `game` 
 * sets the int arrays to NULL
 * calls grid_delete on the grid
 * then free's the game itself
 */
void game_delete(game_t* game);

#endif
