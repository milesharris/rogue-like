/*
 * game module definition for my rogue like
 * stores all information about the current state of a game
 * used as a global variable in both server and client
 */

#ifndef __GAME_H
#define __GAME_H

#include <stdbool.h>
#include "grid.h"
#include "hashtable.h"
#include "player.h"
#include "floor.h"

/**************** global types ****************/
typedef struct game game_t; // opaque to users of the module

/**************** functions **************/

/**************** getters **************/
hashtable_t *game_getPlayers(game_t *game);
floor_t **game_getFloors(game_t *game);
int game_getRemainingGold(game_t *game);
int game_getLastCharID(game_t *game);
int game_getNumPlayers(game_t *game);
int game_getNumPiles(game_t *game);
int game_getNumFloors(game_t *game);
player_t *game_getPlayer(game_t *game, char *playerName);

/* finds the player in the game with the given address
 * returns NULL if player not found or bad parameters
 * returns player with given address if they exist
 */
player_t *game_getPlayerAtAddr(game_t *game, addr_t address);

/**************** setters ***************/

/* returns new value, or -1 if failure.
 * integer input constrained to range of capital letter ASCII codes, [65-90]
 */
int game_setLastCharID(game_t *game, int charID);

/* sets the number of players to the given value
 * returns the new number on success
 * returns -1 when game NULL or numPlayers exceeds MAXPLAYERS
 * */
int game_setNumPlayers(game_t *game, int numPlayers);

/**************** game_new *****************/
/* The game_new function allocates space for a new 'struct game'
 * it only malloc's space for itself. All other memory must be allocated before
 * for example, a `game` takes non-null `floors` as parameters
 * so the floors array must exist before passing it to `game`
 * All memory allocated by the game and its floors
 * are freed in game_delete
 */
game_t *game_new(int *piles, floor_t **floors);

/*************** game_addPlayer **************/
/* adds a struct player to the hashtable of players within a given game struct
 * the player is keyed by their name, which is copied into the hashtable's memory
 * thus, in the game module's memory. All "players" are free'd with game_delete
 * the function returns false if invalid params or if failure to add player
 * true on success
 */
bool game_addPlayer(game_t *game, player_t *player);

/***************** game_buildSummary ***************/
/* builds the summary table displayed to players when the game ends nomrmally
 * also includes the corresponding QUIT message
 * returns a malloc'd string, caller is responsible for free'ing it
 * returns NULL if malloc failure or game does not exist
 */
char *game_buildSummary(game_t *game);

/************** game_getPlayer ***************/
/* returns a pointer to the player struct corresponding to the given name
 * returns NULL if given string or game invalid, or if player not in hashtable
 */
player_t *game_getPlayer(game_t *game, char *playerName);

/************** game_delete ****************/
/* free's all memory assosciated with a `game`
 * sets the int array of gold piles to NULL
 * calls hashtable_delete on the table of players
 * calls grid_delete on the grid
 * then free's the game itself
 */
void game_delete(game_t *game);

#endif
