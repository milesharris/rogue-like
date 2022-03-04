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
#include "hashtable.h"
#include "player.h"

//TODO: Document, change player storage to hashtable
// file-local constants (consistent with those in server)
static const int MAXPLAYERS = 26;      // max # players in game
static const int MAXGOLD = 250         // max # gold in game

/******************** game struct ******************/
/* see game.h for details */

typedef struct game {
    int* piles;           // ptr to array of piles
    hashtable_t* players; // hashtable of player IDs
    int remainingGold;    // gold left in the game
    grid_t* grid;         // current game grid
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

hashtable_t* game_getPlayers(game_t* game) 
{
  return game ? game->players : NULL;
}

int game_getRemainingGold(game_t* game) 
{
  return game ? game->remainingGold : 0;
}

player_t* game_getPlayer(game_t* game, char* playerName)
{
  // check params
  if (game == NULL || playerName == NULL) {
    return NULL;
  }

  return hashtable_find(game->players, playerName);
}

/**************** game_new ***************/
/* see game.h or details */

game_t* 
game_new(int* piles, grid_t* grid)
{
  hashtable_t* players;                // stores players

  game_t* game = malloc(sizeof(game_t));

  if ( game == NULL ) { // malloc issue
    return NULL;
  }
  
  // make hashtable and handle malloc fail
  if ((players = hashtable_new(MAXPLAYERS)) == NULL) {
    // free game as, currently, it's only a pointer to a struct
    free(game);
    return NULL;
  }

  game->piles = piles;
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

/************** game_addPlayer **************/
bool game_addPlayer(game_t* game, player_t* player)
{
  char* playerName;                    // name of player being added (keys HT)
  // check params
  if (game == NULL || player == NULL) {
    return false;
  }

  // get name for key and add to hashtable
  playerName = player_getName(player);
  if (hashtable_insert(game->players, playerName, player)) {
    return true;
  } else {
    return false;
  }
}

/******************** game_delete ******************/
/* see game.h for full details */

void 
game_delete(game_t* game)
{
  if ( game != NULL ) {
    game->piles = NULL;
    // delete all players in game
    if (game->players != NULL) {
      hashtable_delete(game->players, player_delete);
    }
    grid_delete(game->grid); // make sure not to free this memory twice
    free(game);
  } 
}
