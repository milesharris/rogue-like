/*
 * game module implementation for CS50 nuggets game
 * stores all information about the current state of a game
 * used as a global variable in both server and client
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "game.h"
#include "grid.h"
#include "hashtable.h"
#include "player.h"
#include "log.h"

//TODO: Document in specs
// file-local constants (consistent with those in server)
static const int MAXPLAYERS = 26;      // max # players in game
static const int MAXGOLD = 250;        // max # gold in game

/**************** file-local functions ****************/
static void game_getAtAddrHelper(void* arg, const char* key, void* item);
static void game_summaryHelper(void* arg, const char* key, void* item);

/*************** global types and functions ***************/
/* that is, visible outside of this file */

/******************** game struct ******************/
/* see game.h for details */

typedef struct game {
    int* piles;           // ptr to array of piles
    hashtable_t* players; // hashtable of player IDs
    int remainingGold;    // gold left in the game
    grid_t* grid;         // current game grid
    int lastCharID;       // most recent 'player.charID'
    int numPlayers;       // number of players in a game
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

int game_getNumPlayers(game_t* game)
{
  return game ? game->numPlayers : -1;
}

int game_getRemainingGold(game_t* game) 
{
  return game ? game->remainingGold : -1;
}

int game_getLastCharID(game_t* game)
{
  return game ? game->lastCharID : -1;
}

player_t* game_getPlayer(game_t* game, char* playerName)
{
  // check params
  if (game == NULL || playerName == NULL) {
    return NULL;
  }

  return hashtable_find(game->players, playerName);
}

/******************* setters *******************/

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

int game_setLastCharID(game_t* game, int charID)
{
  // check params, constrains input to capital letter ASCII codes
  if (game == NULL || charID < 65 || charID > 90) {
    return -1;
  }

  // set and return
  game->lastCharID = charID;
  return game->lastCharID;
}

int game_setNumPlayers(game_t* game, int numPlayers)
{
  if (game == NULL || numPlayers > MAXPLAYERS) {
    log_d("failure to set numPlayers to %d", numPlayers);
    return -1;
  }

  game->numPlayers = numPlayers;
  return game->numPlayers;
}

/*************** functions *****************/

/**************** game_new ***************/
/* see game.h or details */

game_t* 
game_new(int* piles, grid_t* grid)
{
  hashtable_t* players;         // stores players
  const int defaultCharID = 64; // ASCII for '@', 1st player gets default + 1

  // allocate game struct and check
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

  // initialize attributes to default values or parameters
  game->numPlayers = 0;
  game->lastCharID = defaultCharID;
  game->piles = piles;
  game->remainingGold = MAXGOLD;
  game->grid = grid;

  return game;
}

/**************** game_buildSummary ***************/
/* see header file for details */
char* game_buildSummary(game_t* game) 
{
  char* gameSummary;                   // summary string to return
  
  // check param
  if (game == NULL) {
    return NULL;
  }

  // build start of summary
  char* firstLine = "QUIT GAME OVER:\n";
  if ((gameSummary = malloc(strlen(firstLine) + 1)) == NULL) {
    return NULL;
  }
  strcpy(gameSummary, firstLine);

  // fill in rest of table and return
  hashtable_iterate(game->players, gameSummary, game_summaryHelper);
  return gameSummary;
}

/*************** game_summaryHelper ***************/
/* helper to build the summary table
 * calls player_summarize on each player
 * concatenates it to the string passed in through arg
 * also reallocates string passed through arg to store 
 * then free's the string
 */
static void game_summaryHelper(void* arg, const char* key, void* item)
{
  // extract from params
  char* gameSummary = arg;
  player_t* player = item;
  char* toAdd = player_summarize(player);  // string to concat to summary
  char* temp;                              // checks realloc success           
  
  // allocate enough memory to concat
  temp = realloc(gameSummary, strlen(gameSummary) + strlen(toAdd) + 1);
  // exit on malloc failure. 
  if (temp == NULL) {
    return;
  }
  gameSummary = temp;
  // add the player's summary to the game string, then clean up memory
  strcat(gameSummary, toAdd);
  free(toAdd);
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
    // increment numPlayers and lastCharID for next add
    game->numPlayers++;
    game->lastCharID++;
    return true;
  } else {
    return false;
  }
}

/************** game_getPlayerAtAddr ***************/
/* see header file for details */
player_t* game_getPlayerAtAddr(game_t* game, addr_t address)
{
  hashtable_t* players;                        // table of players in game
  player_t* player = NULL;                     // player with given address
  
  // takes a name string and message string to give to hashtable_iterate
  void* container[] = {&address, player};
  // check params
  if (game == NULL || ! message_isAddr(address)) {
    return NULL;
  }

  // check existence of player table and assign
  if ((players = game_getPlayers(game)) == NULL) {
    return NULL;
  }

  // iterate over hashtable, returning the player with given address if exist
  hashtable_iterate(players, container, game_getAtAddrHelper);

  return player;
}

/***************** game_getAtAddrHelper ************/
static void game_getAtAddrHelper(void* arg, const char* key, void* item) {
  // extract params
  void** container = arg;
  addr_t* targetAddr = container[0];
  player_t* currPlayer = item;
  
  // compares addresses and sends matching player back to parent functions
  if (message_eqAddr(*targetAddr, player_getAddr(currPlayer))) {
    container[1] = currPlayer;
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
      // casts player_delete to satisfy hashtable_delete
      hashtable_delete(game->players, (void (*)(void*))player_delete);
    }
    grid_delete(game->grid); // make sure not to free this memory twice
    free(game);
  } 
}
