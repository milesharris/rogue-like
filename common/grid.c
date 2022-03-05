/*
 * This file implements the "grid" module for our nuggets game
 * The "grid" module is defined in grid.h
 * The grid module provides functionality for all actions involving the map
 * It does not handle any sort of player behavior
 *
 * Winter 2022, CS50 team 1
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "grid.h"
#include "mem.h"
#include "file.h"

/**************** file-local constants *******************/
const char ROOMTILE = '.';
/**************** file-local global variables ****************/
/* none */

/**************** local types ****************/
/* none */

/**************** global types ****************/
typedef struct grid {
  char* reference;                     // original map file read into a string
  char* active;                        // map string that changes during game
  size_t mapLen;                       // length of map string
  int numColumns;                      // number of rows in the map
  int numRows;                         // number of columns in the map
} grid_t;

/**************** global functions ****************/
/* that is, visible outside this file */
/* see grid.h for comments about exported functions */

/**************** local functions ****************/
/* not visible outside this file */
static int longestRowLength(char* map);

/**************** getters *****************/
/* returns NULL or 0 if values don't exist as appropriate */
char* grid_getReference(grid_t* grid)
{
  return grid ? grid->reference : NULL;
}

char* grid_getActive(grid_t* grid)
{
  return grid ? grid->active : NULL;
}

int grid_getNumRows(grid_t* grid)
{
  return grid ? grid->numRows : 0;
}

int grid_getNumColumns(grid_t* grid)
{ 
  return grid ? grid->numColumns : 0; 
}

size_t grid_getMapLen(grid_t* grid)
{
  return grid ? grid->mapLen : 0;
}

/**************** grid_new *****************/
/* see header file for details */
grid_t* grid_new(char* mapFile)
{
  FILE* fp = NULL;                     // file to read from
  grid_t* grid = NULL;                 // grid struct to create
  
  // allocate space for grid, return NULL if failure
  if ((grid = mem_malloc(sizeof(grid_t))) == NULL) {
    return NULL;
  }

  // open file and read into struct
  if ((fp = fopen(mapFile, "r")) != NULL) {
    // number of rows in the grid == number of lines in source file
    grid->numRows = file_numLines(fp);
    // allocate reference by reading from file
    grid->reference = file_readFile(fp);
    fclose(fp);
    // clean up and return NULL if failure to allocate reference map
    if (grid->reference == NULL) {
      grid_delete(grid);
      return NULL;
    }
    // store length of map string
    grid->mapLen = strlen(grid->reference);

    // create a copy of the reference map to use as active map
    grid->active = mem_malloc(strlen(grid->reference) + 1);
    // clean up and return NULL if failure to allocate active map
    if (grid->active == NULL) {
      grid_delete(grid);
      return NULL;
    }

    // copy map into new memory
    strcpy(grid->active, grid->reference);

    // number of colums == length of longest row
    grid->numColumns = longestRowLength(grid->reference);
    // return the "complete" grid only if all operations successful
    return grid;

  } else {
    // clean up and return NULL if file unreadable
    mem_free(grid);
    return NULL;
  }
}

/*********** grid_containsEmptyTile **********/
/* see header file for details */
bool grid_containsEmptyTile(grid_t* grid)
{
  // check params and active map
  if (grid == NULL || grid->active == NULL) {
    return false;
  }

  // loop over all tiles
  for (int i = 0; i < grid->mapLen; i++) {
    if (grid->active[i] == ROOMTILE) {
      return true;
    }
  }

  // false if no empty tile
  return false;
}

/**************** grid_replace ***************/
/* see header file for details */
bool grid_replace(grid_t* grid, int pos, char newChar)
{
  // check param existence
  if (grid->active == NULL || grid->reference == NULL) {
    return false;
  }
  // check if pos is out of bounds
  if (pos < 0 || pos > grid->mapLen - 1) {
    return false;
  }

  // set character at given pos to given character and return success
  grid->active[pos] = newChar;
  return true;
}

/**************** grid_revertTile **************/
/* see header file for details */
bool grid_revertTile(grid_t* grid, int pos)
{
  // check param existence
  if (grid->active == NULL || grid->reference == NULL) {
    return false;
  }
  // check if pos is out of bounds
  if (pos < 0 || pos > grid->mapLen - 1) {
    return false;
  }

  // set 'active' character at given pos to reference value and return
  grid->active[pos] = grid->reference[pos];
  return true;
}

/**************** grid_delete ***************/
/* see header file for details */
void grid_delete(grid_t* grid)
{
  // free strings if they exist
  if (grid->active != NULL) {
    mem_free(grid->active);
  }

  if (grid->reference != NULL) {
    mem_free(grid->reference);
  }

  // then free the struct itself
  mem_free(grid);
}

/************* findLongestRow **************/
/* Takes a given string
 * which should be the in-memory representation of the in-game map
 * and returns the length of the longest "row" (without the new line character)
 */
static int longestRowLength(char* map)
{
  size_t mapLen = 0;                   // length of the map string
  int rowLen = 0;                      // length of current "row" in map
  int rowMax = 0;                      // length of longest "row" in map

  // store len for loop
  mapLen = strlen(map);

  // read the entire map
  for (int i = 0; i < mapLen; i++) {
    // when reaching the end of a "row"
    if (map[i] == '\n') {
      // update max row length if needed
      if (rowLen > rowMax) {
        rowMax = rowLen;
      }
      // reset length of current row for next iteration
      rowLen = 0;
    } else {
      // increment length of row if not at end
      rowLen++;
    }
  }

  return rowMax;
}

/* ********************************************************** */
/* a simple unit test of the code above */
#ifdef GRIDTEST
int main(const int argc, char* argv[])
{
  FILE* fp = NULL;                     // map file to read from
  grid_t* grid = NULL;                 // testing grid
  char* active = NULL;                 // active map
  char* reference = NULL;              // reference map

  // test command line args
  if (argc != 2) {
    fprintf(stderr, "invalid num arguments\n");
    exit(1);
  }

  fp = fopen(argv[1], "r");
  if (fp == NULL) {
    fprintf(stderr, "can't open %s\n", argv[1]);
    exit(2);
  }
  fclose(fp);

  // create grid from file
  grid = grid_new(argv[1]);
  if (grid == NULL) {
    fprintf(stderr, "grid creation failure\n");
  }

  // extract maps
  reference = grid_getReference(grid);
  active = grid_getActive(grid);

  // print both maps and some info
  printf("Reference map: \n%s\n", reference);
  printf("Active map: \n%s\n", active);

  printf("numRows: %d\n", grid_getNumRows(grid));
  printf("numColumns: %d\n", grid_getNumColumns(grid));

  // make a few changes to the active map
  grid_replace(grid, 5, '!');
  grid_replace(grid, 2, '3'); 
  
  // reprint active
  printf("Active map after replacement: \n%s\n", active);

  // revert active
  grid_revertTile(grid, 5);
  grid_revertTile(grid, 2);

  printf("Active map after reversion: \n%s\n", active);

  grid_delete(grid);
  // exit successfully after test completion
  exit(0);
}
#endif