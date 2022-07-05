/*
 * game module implementation for my rogue-like
 * stores all information about the current state of a game
 * used as a global variable in both server and client
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
#include "floor.h"

// file-local constants (consistent with those in server)
static const int MAXPLAYERS = 26; // max # players in game
static const int MAXGOLD = 250;   // max # gold in game
// TODO: will be changed, constant for initial test of floor system
static const int NUMFLOORS = 5; // number of floors in game

/**************** file-local functions ****************/
static void game_getAtAddrHelper(void *arg, const char *key, void *item);
static void game_summaryHelper(void *arg, const char *key, void *item);

/*************** global types and functions ***************/
/* that is, visible outside of this file */

/******************** game struct ******************/
/* see game.h for details */

typedef struct game
{
  hashtable_t *players; // hashtable of player IDs
  floor_t **floors;     // array of floors
  int numFloors;        // number of floors in game
  int lastCharID;       // most recent 'player.charID'
  int numPlayers;       // number of players in a game
  int remainingGold;    // gold left in the game
  int numPiles;         // number of gold piles in game
} game_t;

/**************** getters ****************/
hashtable_t *game_getPlayers(game_t *game)
{
  return game ? game->players : NULL;
}

int game_getNumPlayers(game_t *game)
{
  return game ? game->numPlayers : -1;
}

int game_getNumPiles(game_t *game)
{
  return game ? game->numPiles : -1;
}

int game_getNumFloors(game_t *game)
{
  return game ? game->numFloors : -1;
}

int game_getRemainingGold(game_t *game)
{
  return game ? game->remainingGold : -1;
}

int game_getLastCharID(game_t *game)
{
  return game ? game->lastCharID : -1;
}

player_t *game_getPlayer(game_t *game, char *playerName)
{
  // check params
  if (game == NULL || playerName == NULL)
  {
    return NULL;
  }

  return hashtable_find(game->players, playerName);
}

floor_t **game_getFloors(game_t *game)
{
  return game ? game->floors : NULL;
}

/******************* setters *******************/

int game_setLastCharID(game_t *game, int charID)
{
  // check params, constrains input to capital letter ASCII codes
  if (game == NULL || charID < 65 || charID > 90)
  {
    return -1;
  }

  // set and return
  game->lastCharID = charID;
  return game->lastCharID;
}

int game_setNumPlayers(game_t *game, int numPlayers)
{
  if (game == NULL || numPlayers > MAXPLAYERS)
  {
    log_d("failure to set numPlayers to %d", numPlayers);
    return -1;
  }

  game->numPlayers = numPlayers;
  return game->numPlayers;
}

/*************** functions *****************/

/**************** game_new ***************/
/* see game.h or details */
game_t *
game_new(int *piles, floor_t **floors)
{
  hashtable_t *players;         // stores players
  const int defaultCharID = 64; // ASCII for '@', 1st player gets default + 1

  // allocate game struct and check
  game_t *game = malloc(sizeof(game_t));
  if (game == NULL)
  { // malloc issue
    return NULL;
  }

  // make hashtable and handle malloc fail
  if ((players = hashtable_new(MAXPLAYERS)) == NULL)
  {
    // free game as, currently, it's only a pointer to a struct
    free(game);
    return NULL;
  }

  // initialize attributes to default values or parameters
  game->players = players;
  game->floors = floors;
  game->numFloors = NUMFLOORS;
  game->numPlayers = 0;
  game->lastCharID = defaultCharID;
  game->numPiles = -1;
  game->remainingGold = MAXGOLD;

  return game;
}

/**************** game_buildSummary ***************/
/* see header file for details */
char *game_buildSummary(game_t *game)
{
  char *gameSummary; // summary string to return

  // check param
  if (game == NULL)
  {
    return NULL;
  }

  // build start of summary
  char *firstLine = "QUIT GAME OVER:\n";
  if ((gameSummary = malloc(strlen(firstLine) + 1)) == NULL)
  {
    return NULL;
  }
  strcpy(gameSummary, firstLine);

  // pass to iterator to avoid segfault on exit
  char **summaryPtr = &gameSummary;
  // fill in rest of table and return
  hashtable_iterate(game->players, summaryPtr, game_summaryHelper);
  gameSummary = summaryPtr[0];
  return gameSummary;
}

/*************** game_summaryHelper ***************/
/* helper to build the summary table
 * calls player_summarize on each player
 * concatenates it to the string passed in through arg
 * also reallocates string passed through arg to store
 * then free's the string
 */
static void game_summaryHelper(void *arg, const char *key, void *item)
{
  // extract from params
  char **gameSummary = arg;
  char *summary = *gameSummary;
  player_t *player = item;
  char *temp; // checks realloc success

  // ignore spectator in table
  if (strcmp("spectator", player_getName(player)) == 0)
  {
    return;
  }

  char *toAdd = player_summarize(player); // string to concat to summary

  // allocate enough memory to concat
  temp = realloc(summary, (strlen(summary) + strlen(toAdd) + 1));
  // exit on malloc failure.
  if (temp == NULL)
  {
    return;
  }
  summary = temp;
  // add the player's summary to the game string, then clean up memory
  strcat(summary, toAdd);
  gameSummary[0] = summary;
  free(toAdd);
}

/************** game_addPlayer **************/
/* see header file for details */
bool game_addPlayer(game_t *game, player_t *player)
{
  char *playerName; // name of player being added (keys HT)

  // check params
  if (game == NULL || player == NULL)
  {
    return false;
  }

  // get name for key and add to hashtable
  playerName = player_getName(player);
  if (hashtable_insert(game->players, playerName, player))
  {
    // increment values for next add unless handling spectator
    if (strcmp(player_getName(player), "spectator") != 0)
    {
      game->numPlayers++;
      game->lastCharID++;
    }
    return true;
  }
  else
  {
    return false;
  }
}

/************** game_getPlayerAtAddr ***************/
/* see header file for details */
player_t *game_getPlayerAtAddr(game_t *game, addr_t address)
{
  hashtable_t *players;                              // table of players in game
  player_t *player = NULL;                           // player with given address
  const char *addrStr = message_stringAddr(address); // string for comparision
  void *voidplayer = NULL;                           // allows pulling player from container

  // copy string to avoid compiler warnings
  char *addrStrCpy = mem_malloc_assert(strlen(addrStr) + 1,
                                       "failed to alloc addrstrcpy");
  strcpy(addrStrCpy, addrStr);

  // takes a name string and message string to give to hashtable_iterate
  void *container[2] = {addrStrCpy, voidplayer};
  // check params
  if (game == NULL || !message_isAddr(address))
  {
    return NULL;
  }

  // check existence of player table and assign
  if ((players = game_getPlayers(game)) == NULL)
  {
    return NULL;
  }

  // iterate over hashtable, returning the player with given address if exist
  hashtable_iterate(players, container, game_getAtAddrHelper);
  // clean up and return
  player = container[1];
  free(addrStrCpy);
  return player;
}

/***************** game_getAtAddrHelper ************/
/* helper function for game_getPlayerAtAddr
 * passed into hashtable_iterate
 * takes the target address (in string form) and a void pointer
 * that represents the player with the given address
 * inside an array of void pointers
 * updates the player pointer if there is a player matching the address
 */
static void game_getAtAddrHelper(void *arg, const char *key, void *item)
{
  // extract params
  void **container = arg;
  char *targetAddrStr = container[0];
  player_t *currPlayer = item;
  const char *currAddrStr = message_stringAddr(player_getAddr(currPlayer));

  // compares addresses and sends matching player back to parent functions
  if (strcmp(targetAddrStr, currAddrStr) == 0)
  {
    container[1] = currPlayer;
    return;
  }
}

/******************** game_delete ******************/
/* see game.h for full details */
void game_delete(game_t *game)
{
  if (game != NULL)
  {
    // delete all players in game
    if (game->players != NULL)
    {
      // casts player_delete to satisfy hashtable_delete
      hashtable_delete(game->players, (void (*)(void *))player_delete);
    }
    // delete all floors in the game
    for (int i = 0; i < game->numFloors; i++)
    {
      floor_delete(game->floors[i]);
    }
    free(game);
  }
}
