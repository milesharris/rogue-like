/*
 * floor module implementation for my rogue-like
 * stores all information about the current state of a floor
 * stored within the game variable
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "floor.h"
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
  hashtable_t *players; // hashtable of player IDs on the floor
  int numPlayers;       // number of players on the floor
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

int floor_getNumPiles(floor_t *floor)
{
  return floor ? floor->numPiles : -1;
}

hashtable_t *floor_getPlayers(floor_t *floor)
{
  return floor ? floor->players : NULL;
}

int floor_getNumPlayers(floor_t *floor)
{
  return floor ? floor->numPlayers : -1;
}

int floor_getRemainingGold(floor_t *floor)
{
  return floor ? floor->remainingGold : -1;
}

/**************** setters ****************/

/************** floor_setRemainingGold **************/
/* see floor.h for details */
bool floor_setRemainingGold(floor_t *floor, int gold)
{
  if (floor == NULL)
  {
    return false;
  }

  floor->remainingGold = gold;
  return true;
}

/************** floor_setGrid **************/
/* see floor.h for details */
bool floor_setGrid(floor_t *floor, grid_t *grid)
{
  if (floor == NULL || grid == NULL)
  {
    return false;
  }
  else
  {
    grid_delete(floor->grid);
    floor->grid = grid;
    return true;
  }
}

/************** floor_setNumPlayers **************/
/* see floor.h for details */
int floor_setNumPlayers(floor_t *floor, int numPlayers)
{
  if (floor == NULL)
  {
    return -1;
  }
  floor->numPlayers = numPlayers;
  return floor->numPlayers;
}

/************** floor_setNumPiles **************/
/* see floor.h for details */
int floor_setNumPiles(floor_t *floor, int numPiles)
{
  if (floor == NULL)
  {
    return -1;
  }
  floor->numPiles = numPiles;
  return floor->numPiles;
}

/*************** functions *****************/
floor_t *floor_new(grid_t *grid, int *piles)
{
  hashtable_t *players; // tracks players on the floor

  // allocate space for floor and check
}