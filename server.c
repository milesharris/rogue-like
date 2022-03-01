/* 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file.h"
#include "grid.h"

int MAXPILES = 30;
int MAXPLAYERS = 25;

/***** game struc ********************************************/
/* see server.h for details */

static typedef struct game {
    int* piles;         // ptr to array of piles
    int* players;       // ptr to array of players
    int remainingGold;  // gold left in the game
    grid_t* grid;       // current game grid
} game_t;

/***** game_new **********************************************/
/* see server.h or details */

game_t* 
game_new(int* piles, int* players, grid_t* grid)
{
  game_t* game = malloc(sizeof(game_t));

  if( game == NULL ){ // malloc issue
    return NULL;
  }
  
  game->piles = piles;
  game->players = players;
  game->remainingGold = 250;
  game->grid = grid;

  return game;
}

/***** game_setRemainingGold *********************************/
/* see server.h for details */

bool
game_setRemainingGold(game_t* game, int gold)
{
  if( game == NULL || gold < 0 ){
    return false;
  } else {
    game->remainingGold = gold;
    return true;
  }
}

/***** game_setGrid ******************************************/
/* see server.h for details */

bool
game_setGrid(game_t* game, grid_t* grid)
{
  if( game == NULL || grid == NULL ){
    return false;
  } else {
    game->grid = grid;
    return true;
  }
}

/***** game_delete *******************************************/
/* see server.h for full details */

void 
game_delete(game_t* game)
{
  if( game != NULL ){
    game->piles = NULL;
    game->players = NULL;
    grid_delete(game->grid); // make sure not to free this memory twice
    free(game);
  } 
  return;
}


/***** initializeGame ****************************************/
/* see server.h for details */

static int
initializeGame(char* filepathname, int seed)
{
  // check args
  if( filepathname == NULL ){
    fprintf(stderr, "initializeGame: NULL arg given\n");
    return 1;
  }
  
  // create a filepointer and check it 
  FILE* fp = fopen(filepathname);
  if( fp == NULL ){
    fprintf(stderr, "initializeGame: err creating filepointer\n");
    return 1;
  }

  // setup pseudo-random number sequence
  if( seed == NULL ){
    srand(getpid());
  } else {
    srand(seed);
  }
  
  // create the grid
  grid_t* grid = grid_new(filepathname);
    
  // Generating random piles

  int piles[MAXPILES];        // array of piles
  int totalGold = 250;  
  int currPile = 0;     // value (gold) of current pile
  int currIndex = 0;    // index into array
  int tmp = 0;          // temporary variable used to hold random number

  // initialize piles to -1
  memset(piles, -1, sizeof(piles));
  
  while( totalGold > 0 ){

    if( currIndex == (MAXPILES - 1) ){ // prevents the unlikely case in which we reach 30 piles
      currPile = totalGold;
      totalGold = 0;
    } else {
      tmp = rand();
      currPile = (tmp % 25);
      // to avoid a pile having zero gold
      if(currPile == 0){
        currPile = 25;
      }
      totalGold -= currPile;
    }
    
    piles[currIndex] = currPile;
    currIndex++;
  }
  
  // inserting gold piles into map

  int gridSize = strlen(grid->reference); // length of map string
  int pilesInserted = 0;
  int slot  0;
  tmp = 0;
  

  while( pilesInserted < currIndex){ // we don't want to insert more piles than we have
    
    tmp = rand();
    slot = (tmp % gridSize);

    if( grid->active[slot] == "." ){ // we only insert into valid spaces in the map
      if(grid_replace(grid, slot, "*")){
        pilesInsert++;
      } else {
        fprintf(stderr, "initializeGame: err inserting pile in map\n");
      }

    } 
  }

  return 0;
}
