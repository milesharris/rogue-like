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

/************** floor_delete ****************/
/* free's all memory assosciated with a `floor`
 * free's the piles array
 * calls grid_delete on the grid
 * then free's the game itself
 */
void floor_delete(floor_t *floor);

/************** floor_subtractGold *************/
/* subtracts the given amount from the remaining gold
 * returns the new amount
 * returns -1 if the amount is negative
 */
int floor_subtractGold(floor_t *floor, int gold);

#endif