/*
 * floor module implementation for my rogue-like
 * stores all information about the current state of a floor
 * stored within the game variable
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "game.h"
#include "mem.h"
#include "grid.h"
#include "hashtable.h"
#include "player.h"
#include "log.h"

/******************** floor struct ******************/
/* see floor.h for details */
typedef struct floor
{
  int *piles;           // array of gold piles
  int remainingGold;    // gold left in the game
  int numPiles;         // number of gold piles in game
  hashtable_t *players; // hashtable of player IDs
  grid_t *grid;         // current floor grid
  char *mapfile;        // filepath of the in-game map
} floor_t;

/**************** getters ****************/
int *floor_getPiles(floor_t *floor)
{
  return floor ? floor->piles : NULL;
}

grid_t *floor_getGrid(floor_t *floor)
{
  return floor ? floor->grid : NULL;
}

char *floor_getMapfile(floor_t *floor)
{
  return floor ? floor->mapfile : NULL;
}

int floor_getNumPiles(game_t *floor)
{
  return floor ? floor->numPiles : -1;
}

hashtable_t *floor_getPlayers(floor_t *floor)
{
  return game ? floor->players : NULL;
}

int floor_getNumPlayers(floor_t *floor)
{
  return floor ? floor->numPlayers : -1;
}

int floor_getRemainingGold(floor_t *floor)
{
  return floor ? floor->remainingGold : -1;
}