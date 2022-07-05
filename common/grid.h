/*
 * This file defines the "grid" module for my rogue-like game
 * The grid module provides functionality for all actions involving the map
 * It does not handle any sort of player behavior
 *
 * A "grid" is the map for my rogue-like
 * It is generated from a text file that we assume produces a valid map
 * The "grid" structure stores two copies of that map as strings
 * One of the strings is the constant "reference map"
 * We refer back to it when making changes in the other, "active map"
 * The "active map" is the one which is modified by the server and rendered
 * When, for example, a player moves on the "active map"
 * We use the "reference map" to replace the character previously occupied by the player
 * It also contains the number of columns and rows for the given grid
 *
 * July 2022, Miles Harris, Rogue-Like
 * Winter 2022, Miles Harris and Karsten Kleyensteuber, CS50 final project
 */
* /

#ifndef __GRID_H
#define __GRID_H

#include <stdbool.h>

    /**************** global types ****************/
    typedef struct grid grid_t; // opaque to users of the module

/**************** functions **************/

/**************** getters **************/
char *grid_getReference(grid_t *grid);
char *grid_getActive(grid_t *grid);
int grid_getNumRows(grid_t *grid);
int grid_getNumColumns(grid_t *grid);
size_t grid_getMapLen(grid_t *grid);
char *grid_getMapfile(grid_t *grid);

/**************** grid_new ***************/
/* initialize a new "grid"
 * takes a string as a parameter where the string is the path to the map file
 * allocates memory for the map string and struct itself
 * that must then be free'd in grid_delete
 * also stores the number of rows and columns in the grid within the struct
 * returns the grid if process successful
 * returns NULL if error at any point in the process (including allocating memory)
 */
grid_t *grid_new(char *mapFile);

/*************** grid_replace *************/
/* replace the given character at the given index position in the map string
 * modifies the "active map" of the given grid structure
 * at the given index position, replacing it with the given character
 * returns true if success, false if error
 */
bool grid_replace(grid_t *grid, int pos, char newChar);

/**************** grid_revertTile **************/
/* Replaces the character at the given position of the given grid's active map
 * with the character at the same position in the given grid's reference map
 * most often used when players move or when gold is picked up
 * returns true if success, false if the strings in the given grid don't exist
 */
bool grid_revertTile(grid_t *grid, int pos);

/************ grid_containsEmptyTile *********/
/* allows a user to determine whether or not a given grid's active map
 * contains an empty room tile. Most useful when adding a player
 * because it determines whether a player can be added or not
 * returns true if there is at least one ROOMTILE ('.')
 * false if there is not
 */
bool grid_containsEmptyTile(grid_t *grid);

/*************** grid_delete **************/
/* free's all memory in use by the given grid
 * checks for existence of strings before deleting them
 * allows function to be called in cases when failure to allocate memory
 */
void grid_delete(grid_t *grid);

/********** grid_calculateVision ***********/
/* Calculates a player's current vision, in the form of an integer array the same size as our map
 * indicating which points are visible with a 1 indicating visibility or -1 indicating non-visibility
 * modifies the given array
 * Parameters:  grid - the grid of the map we are playing the game on
 *              pos - a players position within the map (int)
 *              vision - the int array which stores corresponding visibility information
 * Returns:     void
 */
void grid_calculateVision(grid_t *grid, int pos, int *vision);

#endif
