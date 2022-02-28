/*
 * This file defines the "grid" module for our nuggets game
 * The grid module provides functionality for all actions involving the map
 * It does not handle any sort of player behavior
 * 
 * A "grid" is the map for out nuggets game.
 * It is generated from a text file that we assume produces a valid map
 * The "grid" structure stores two copies of that map as strings
 * One of the strings is the constant "reference map"
 * We refer back to it when making changes in the other, "active map"
 * The "active map" is the one which is modified by the server and rendered
 * When, for example, a player moves on the "active map"
 * We use the "reference map" to replace the character previously occupied by the player
 * It also contains the number of columns and rows for the given grid
 *
 * Winter 2022, CS50 team 1
 */


#ifndef __GRID_H
#define __GRID_H

#include <stdbool.h>

/**************** global types ****************/
typedef struct grid grid_t;  // opaque to users of the module

/**************** functions **************/

/**************** getters **************/
char* grid_getReference(grid_t*);
char* grid_getActive(grid_t*);
int grid_getNumRows(grid_t*);
int grid_getNumColumns(grid_t*);

/**************** grid_new ***************/
/* initialize a new "grid"
 * takes a string as a parameter where the string is the path to the map file
 * allocates memory for the map string that must then be free'd in grid_delete 
 * also stores the number of rows and columns in the grid within the struct
 */
grid_t* grid_new(char* mapFile);

/*************** grid_replace *************/
/* replace the given character at the given index position in the map string
 * modifies the "active map" of the given grid structure 
 * at the given index position, replacing it with the given character
 */
bool grid_replace(grid_t* grid, int pos, char newChar);

/*************** grid_delete **************/
/* free's all memory in use by the given grid
 */
void grid_delete(grid_t* grid);

/*************** local functions **************/


#endif