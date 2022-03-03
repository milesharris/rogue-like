/*
 * game module implementation for CS50 nuggets game
 * stores all information about the current state of a game
 * used as a global variable in both server and client
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "game.h"
#include "grid.h"

/******************** game struct ******************/
/* see game.h for details */

typedef struct game {
    int* piles;         // ptr to array of piles
    int* players;       // ptr to array of player IDs
    int remainingGold;  // gold left in the game
    grid_t* grid;       // current game grid
} game_t;

/**************** getters ****************/
grid_t* game_getGrid(game_t* game)
{
  return game ? game->grid : NULL;
}

int* game_getPiles(game_t* game)
{
  return game ? game->piles : NULL;
}

int* game_getPlayers(game_t* game) 
{
  return game ? game->players : NULL;
}

int game_getRemainingGold(game_t* game) 
{
  return game ? game->remainingGold : 0;
}

/**************** game_new ***************/
/* see game.h or details */

game_t* 
game_new(int* piles, int* players, grid_t* grid)
{
  const int MAXGOLD = 250;             // max amount of gold in game

  game_t* game = malloc(sizeof(game_t));

  if ( game == NULL ) { // malloc issue
    return NULL;
  }
  
  game->piles = piles;
  game->players = players;
  game->remainingGold = MAXGOLD;
  game->grid = grid;

  return game;
}

/***** game_setRemainingGold *********************************/
/* see game.h for details */

bool
game_setRemainingGold(game_t* game, int gold)
{
  if ( game == NULL || gold < 0 ) {
    return false;
  } else {
    game->remainingGold = gold;
    return true;
  }
}

/***************** game_subtractGold **************/
/* see game.h for details */
int
game_subtractGold(game_t* game, int gold) 
{
  if (game == NULL) {
    return -1;
  }
  game->remainingGold -= gold;
  return game->remainingGold;
}

/******************* game_setGrid *******************/
/* see game.h for details */

bool
game_setGrid(game_t* game, grid_t* grid)
{
  if ( game == NULL || grid == NULL ) {
    return false;
  } else {
    // free old grid before replacing with new
    grid_delete(game->grid);
    game->grid = grid;
    return true;
  }
}

/******************** game_delete ******************/
/* see game.h for full details */

void 
game_delete(game_t* game)
{
  if ( game != NULL ) {
    game->piles = NULL;
    game->players = NULL;
    grid_delete(game->grid); // make sure not to free this memory twice
    free(game);
  } 
}
