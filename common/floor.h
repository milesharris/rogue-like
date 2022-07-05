/*
 * floor module definition for my rogue like
 * stores all information about the current state of a in-game floor
 * stored within the game struct
 */

#ifndef __FLOOR_H
#define __FLOOR_H

#include <stdbool.h>
#include "grid.h"
#include "hashtable.h"
#include "player.h"

/**************** global types ****************/
typedef struct floor floor_t; // opaque to users of the module

/**************** functions **************/

/**************** getters **************/
grid_t *floor_getGrid(floor_t *floor);
int *floor_getPiles(floor_t *floor);
hashtable_t *floor_getPlayers(floor_t *floor);
int floor_getRemainingGold(floor_t *floor);
char *floor_getMapfile(floor_t *floor);
int floor_getNumPiles(floor_t *floor);

/**************** setters ***************/
/* return false on failure, true on success */
bool floor_setRemainingGold(floor_t *floor, int gold);

/* Note: the setGrid function calls grid_delete on the previous floor->grid
 * to avoid memory leaks
 */
bool floor_setGrid(floor_t *floor, grid_t *grid);

/* sets the number of players to the given value
 * returns the new number on success
 * returns -1 when game NULL or numPlayers exceeds MAXPLAYERS
 * */
int floor_setNumPlayers(floor_t *floor, int numPlayers);

/* sets the number of gold piles in a game
 * returns the new number
 */
int floor_setNumPiles(floor_t *floor, int numPiles);

/*************** functions *****************/

/**************** floor_new *****************/
/* The game_new function allocates space for a new 'struct floor'
 * it only malloc's space for itself. All other memory must be allocated before
 * for example, a `floor` takes a non-null `grid` as a parameter
 * so grid_new must be called on a grid before passing it to `floor`
 * All memory allocated by the floor, its grid, and its int array
 * are freed in floor_delete
 */
floor_t *floor_new(grid_t *grid, int *piles);

#endif