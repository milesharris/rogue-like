/*
 * Defines the player module for my Rogue-like game
 * The player module stores necessary information relating to a player 
 * including current gold, position, name, and current vision.
 * And perhaps eventually inventory
 * It also provides various functions for retrieving and altering this information 
 * as well as for creating and removing players
 *
 * Miles Harris, Summer 2022
 * Miles Harris and Karsten Kleyensteuber, CS50, Winter 2022 - original Nuggets
 */

#ifndef __PLAYER_H
#define __PLAYER_H

#include "grid.h"
#include "message.h"

/***** global types ******************************************/

typedef struct player player_t; // opaque to users of the module

/***** functions *********************************************/

/***** getters ***********************************************/
/* returns the value of various player attributes, or NULL/0 where applicable */

grid_t* player_getVision(player_t* player);
char* player_getName(player_t* player);
char player_getCharID(player_t* player);

/* player_getPos will return -1 upon receiving a NULL argument, this is the default pos value */
int player_getPos(player_t* player);

/* player_getGold will return a 0 upon receiving a NULL argument, this is the default gold value */
int player_getGold(player_t* player);

/* NOTE: This DOES NOT check for NULL within func. Only use on non-null players */
addr_t player_getAddr(player_t* player);

/***** setters ***********************************************/
/* set the value of various attributes of a player struct and return their value */

grid_t* player_setVision(player_t* player, grid_t* vision);
char player_setCharID(player_t* player, char newChar);
int player_setPos(player_t* player, int pos);
int player_setGold(player_t* player, int gold);
addr_t player_setAddr(player_t* player, addr_t address);

/***** player_new ********************************************/
/* Initalized a new 'player' struct
 * takes a string as parameter, wherein the string refers to a player name
 * also takes a mapfile string as a parameter
 * which is then used to generate the initial vision grid
 * allocates memory for the player struct which must be free'd by calling player_delete
 * Stores a copy of the name string, allowing the original name to be free'd
 * initializes other attributes of the player to NULL where applicable, 
 * 0 for gold, and -1 for position
 *
 * returns player_t* if successful, otherwise NULL
 */
player_t* player_new(char* name, char* mapfile);

/***** player_addGold ****************************************/
/* Add the provided number of gold to a player's inventory
 * takes an integer value of gold to add as a parameter
 * returns the player's new gold total if successful
 * returns -1 if the given player is null or given newGold < 0
 */
int player_addGold(player_t* player, int newGold);

/***** player_updateVision ***********************************/
/* Updates a player's vision to that of a given position
 * Takes a point to a player struct, a pointer to a grid struct, and a position integer
 * where, in game, the grid is the server's grid
 * Returns void
 */
void player_updateVision(player_t* player, grid_t* grid);

/***** player_summarize **************************************/
/* creates a summary of the player for printing when the game ends
 * returns the properly formatted summary string on success
 * note that the returned string is malloc'd, caller responsible for free'ing
 * returns NULL if failure to allocate memory or if player is NULL
 */
char* player_summarize(player_t* player);

/***** player_delete *****************************************/
/* Deletes a player struct allocated memory. 
 * Takes a pointer to a player struct, to be deleted, as parameter
 * Returns void
 */
void player_delete(player_t* player);

#endif
