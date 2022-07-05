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

#endif