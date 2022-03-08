/*
 * game module definition for CS50 nuggets game
 * stores all information about the current state of a game
 * used as a global variable in both server and client
 */

#ifndef __GAME_H
#define __GAME_H

#include <stdbool.h>
#include "grid.h"
#include "hashtable.h"
#include "player.h"

/**************** global types ****************/
typedef struct game game_t;  // opaque to users of the module

/**************** functions **************/

/**************** getters **************/
grid_t* game_getGrid(game_t* game);
int* game_getPiles(game_t* game);
hashtable_t* game_getPlayers(game_t* game);
int game_getRemainingGold(game_t* game);
int game_getLastCharID(game_t* game);
int game_getNumPlayers(game_t* game);
char* game_getMapfile(game_t* game);
int game_getNumPiles(game_t* game);

/* finds the player in the game with the given address
 * returns NULL if player not found or bad parameters
 * returns player with given address if they exist
 */ 
player_t* game_getPlayerAtAddr(game_t* game, addr_t address);

/**************** setters ***************/
/* return false on failure, true on success */
bool game_setRemainingGold(game_t* game, int gold);

/* Note: the setGrid function calls grid_delete on the previous game->grid
 * to avoid memory leaks
 */
bool game_setGrid(game_t* game, grid_t* grid);

/* returns new value, or -1 if failure.
 * integer input constrained to range of capital letter ASCII codes, [65-90]
 */
int game_setLastCharID(game_t* game, int charID);

/* sets the number of players to the given value
 * returns the new number on success
 * returns -1 when game NULL or numPlayers exceeds MAXPLAYERS
 * */
int game_setNumPlayers(game_t* game, int numPlayers);

/* sets the number of gold piles in a game
 * returns the new number
 */
int game_setNumPiles(game_t* game, int numPiles);

/**************** game_new *****************/
/* The game_new function allocates space for a new 'struct game' 
 * it only malloc's space for itself. All other memory must be allocated before
 * for example, a `game` takes non-null `grids` as parameters
 * so grid_new must be called on a grid before passing it to `game`
 * All memory allocated by the game, its grid, and its int array 
 * are freed in game_delete 
 */
game_t* game_new(int* piles, grid_t* grid);

/*************** game_addPlayer **************/
/* adds a struct player to the hashtable of players within a given game struct
 * the player is keyed by their name, which is copied into the hashtable's memory
 * thus, in the game module's memory. All "players" are free'd with game_delete 
 * the function returns false if invalid params or if failure to add player
 * true on success
 */
bool game_addPlayer(game_t* game, player_t* player);

/***************** game_buildSummary ***************/
/* builds the summary table displayed to players when the game ends nomrmally
 * also includes the corresponding QUIT message
 * returns a malloc'd string, caller is responsible for free'ing it
 * returns NULL if malloc failure or game does not exist
 */
char* game_buildSummary(game_t* game); 

/************** game_getPlayer ***************/
/* returns a pointer to the player struct corresponding to the given name
 * returns NULL if given string or game invalid, or if player not in hashtable
 */
player_t* game_getPlayer(game_t* game, char* playerName);

/*************** game_subtractGold ***********/
/* Simple function to reduce a game's remaining gold by the given amount
 * returns -1 if game does not exist
 * returns the new value of game->remainingGold on success
 */
int game_subtractGold(game_t* game, int gold);

/************** game_delete ****************/
/* free's all memory assosciated with a `game` 
 * sets the int array of gold piles to NULL
 * calls hashtable_delete on the table of players
 * calls grid_delete on the grid
 * then free's the game itself
 */
void game_delete(game_t* game);

#endif
