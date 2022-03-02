/*
 * Defines the player module for out group's nuggets game
 * The player module stores necessary information relating to a player 
 * including current gold, position, name, and current vision.
 * It also provides various functions for retrieving and altering this information 
 * as well as for creating and removing player's
 *
 * Winter 2022, CS50 Team 1
 */

#ifndef __PLAYER_H
#define __PLAYER_H

#include "grid.h"

/***** global types ******************************************/

typedef struct player player_t; // opaque to users of the module

/***** functions *********************************************/

/***** getters ***********************************************/
/* returns the value of various player attributes, or NULL/0 where applicable */

grid_t* player_getVision(player_t* player);
char* player_getName(player_t* player);

/* player_getPos will return -1 upon receiving a NULL argument, this is the default pos value */
int player_getPos(player_t* player);

/* player_getGold will return a 0 upon receiving a NULL argument, this is the default gold value */
int player_getGold(player_t* player);

/***** setters ***********************************************/
/* set the value of various attributes of a player struct and return their value */

grid_t* player_setVision(player_t* player, grid_t* vision);
int player_setPos(player_t* player, int pos);
int player_setGold(player_t* player, int gold);

/***** player_new ********************************************/
/* Initalized a new 'player' struct
 * takes a string as parameter, wherein the string refers to a player name
 * allocates memory for the player struct which must be free'd by calling player_delete
 * Stores a copy of the name string, allowing the original name to be free'd
 * initializes other attributes of the player to NULL where applicable, 
 * 0 for gold, and -1 for position
 *
 * returns player_t* if successful, otherwise NULL
 */

player_t* player_new(char* name);

/***** player_addGold ****************************************/
/* Add the provided number of gold to a player's inventory
 * takes an integer value of gold to add as a parameter
 * returns the player's new gold total if successful
 * returns -1 if the given player is null or given newGold < 0
 */
int player_addGold(player_t* player, int newGold);

/***** player_delete *****************************************/
/* Deletes a player struct allocated memory. 
 * Takes a pointer to a player struct, to be deleted, as parameter
 * Returns void
 */

void player_delete(player_t* player);

#endif
