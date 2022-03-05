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
#include <stdbool.h>
#include "grid.h"
#include "mem.h"
#include "file.h"

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

/* ************************ VISION ************************** */

/***** local vision functions *********************************/

/***** posToCoordinates ***************************************/
/* converts the position integer into cartesian coordinates to make mathematic manipulation easier */

static void
posToCoordinates(grid_t* grid, int pos, int* tuple)
{
  // check arguments
  if( grid == NULL || pos < 0 ){
    return;
  } 
  
  // calculate coordinates, assumes pos is zero indexed
  int x = (pos % (grid->numColumns + 1)); // add one for new line character not counter in numColumns
  int y = (pos / (grid->numColumns + 1));
  tuple[0] = x;
  tuple[1] = y;
  
  // test print statement
  //fprintf(stdout, "pos %d -> coord: (%d,%d)\n", pos, x, y);

  return;
}

/***** coordinatesToPos ***************************************/
/* converts cartesian coordinates back into an integer representation */

static int
coordinatesToPos(grid_t* grid, int x, int y)
{
  if( grid == NULL || x < 0 || y < 0 ){
    return -1;
  }
  
  // where y in the row and x is the column
  int pos = (y * (grid->numColumns + 1)) + x; // add one to numColumns to account for the new line character
  
  // test print statement
  //fprintf(stdout, "coord (%d,%d) -> pos: %d\n", x, y, pos);

  return pos;
}

/***** calculateVision ****************************************/
/* Calculates a player's current vision, returning a boolean array, the same size as our map, indicating which points are visible
 * Parameters:  pos - a player's current position
 *              grid - the grid struct for the map
 *              vision - a boolean array with values -1 (not visible), 0 (unvisited) or 1 (visible)
 *
 * Returns:     void
 */

void
calculateVision(grid_t* grid, int pos, int* vision)
{ 
  // check parameters
  if( grid == NULL || vision == NULL || pos < 0 ){
    return;
  }

  fprintf(stdout, "initial pos is %d\n\n", pos);

  // testing print statement
  fprintf(stdout, "Grid NC: %d, NR: %d, mapLen: %ld\n\n", grid->numColumns, grid->numRows, grid->mapLen);


  // set player position to visible
  vision[pos] = 1; 
  // map we check values with, used reference because we don't care about players or gold at this point, but active would also work
  char* reference = grid_getReference(grid);
  if( reference == NULL ){
    fprintf(stderr, "calculateVision: error grabbing reference map\n");
    return;
  }

  // boolean describing whether we've hit a wall/corner
  bool wallFound = false;
  
  // checking cardinal directions manually, because using our algortithm to check horizontal of vertival 'slopes' won't work

  int up = pos - (grid->numColumns + 1);

  while( up > 0 ){
    if(reference[up] == '.' && !wallFound){ // marks room squares
      vision[up] = 1;
    } 
    else if(reference[up] != '.' && !wallFound){ // marks first obstruction in this direction
      vision[up] = 1;
      wallFound = true;
    } else {    // marks non-visible 
      vision[up] = -1;
    }

    up -= (grid->numColumns + 1); // parentheses for readability
  }

  
 
  // down
  wallFound = false;
  int down = pos + (grid->numColumns + 1);
  while( down < grid->mapLen ){
    if(reference[down] == '.' && !wallFound){
      vision[down] = 1;
    }
    else if(reference[down] != '.' && !wallFound){
      vision[up] = 1;
      wallFound = true;
    } else {
      vision[down] = -1;
    }

    down += (grid->numColumns + 1);
  }
  
  // right
  wallFound = false;
  int right = pos;
  // fixes as bug where reference is unexpectedly not referenced
  reference = grid_getReference(grid);

  while( reference[right] != '\n' ){ // within the current line
    if(reference[right] == '.' && !wallFound){
      vision[right] = 1;
    }
    else if(reference[right] != '.' && !wallFound){
      vision[right] = 1;
      wallFound = true;
    } else {
      vision[right] = -1;
    }

    right++;
    fprintf(stdout, "right = %d\n", right);
  }

  // left
  wallFound = false;
  int left = pos - 1;
  while( reference[left] != '\n' && left >= 0 ){
    if(reference[left] == '.' && !wallFound){
      vision[left] = 1;
    }
    else if(reference[left] != '.' && !wallFound){
      vision[left] = 1;
      wallFound = true;
    } else {
      vision[left] = -1;
    }

    left--;
  }  
 
  // convert from integer to cartesian coordinates
  int posCoor[2];
  posToCoordinates(grid, pos, posCoor);
  int pointCoor[2];
  // slope of line
  double m = 0.0;
  // iterate through points in the grid
  for(int i = 0; i < grid->mapLen; i++){
    wallFound = false;
    if( vision[i] == 0 ){ // point hasn't been visited
      posToCoordinates(grid, i, pointCoor);

      // slope formula
      m = (double)(pointCoor[1] - posCoor[1]) / (double)(pointCoor[0] - posCoor[0]);

      fprintf(stdout, "pointCoor: (%d,%d) \n", pointCoor[0],pointCoor[1]);
      
      // deciding whether the point is to the right or left of pos
      if( posCoor[0] < pointCoor[0] ){
        int diff = pointCoor[0] - posCoor[0]; // difference between the x coords
        int currX = 1;
        double currY = 0.0;

        fprintf(stdout, "diff %d\n\n",diff);

        // walk from pos to point, checking values along the way
        while( currX <= diff ){
          currY = (m * currX) + posCoor[1]; // y = mx + b
          int rounded = (int) currY;
          double roundedD = (double) rounded;
          
          if( currY == roundedD ){ // the case where currY lies exactly on a point
            int currPos = coordinatesToPos(grid, posCoor[0] + currX, (int) currY); // grab int representation

            fprintf(stdout, "currPos: %d  coord: (%d,%d)\n",currPos,posCoor[0] +currX, (int) currY);

            if( reference[currPos] == '.' && !wallFound ){      // check if room tile
              vision[currPos] = 1;
            }
            else if( reference[currPos] != '.' && !wallFound ){ // check if this is the first wall we've seen
              vision[currPos] = 1;
              wallFound = true;
            } else { // otherwise we've already seen a wall, so this point is not visible
              if(vision[currPos] == 0){
                vision[currPos] = -1;
              }
            }
          } else { // the more likely case currY falls between two points and we need to check both
            int pos1 = coordinatesToPos(grid, posCoor[0] + currX, rounded);    // the two positions in reference we need to check
            int pos2 = coordinatesToPos(grid, posCoor[0] + currX, rounded + 1);

            fprintf(stdout, "pos1: %d pos2: %d, coord: (%d,[%d,%d])\n",pos1,pos2,posCoor[0]+currX,rounded,rounded+1);
            
            if(reference[pos1] == '.' && reference[pos2] == '.' && !wallFound){ // haven't hit a wall yet, and current position is between room tiles
              vision[pos1] = 1;
              vision[pos2] = 1;
            }
            else if(!wallFound){ // haven't found a wall yet, but the current position hits a wall
              vision[pos1] = 1;
              vision[pos2] = 1;
              wallFound = true;
            } else { // we've already see a wall, current position is not visible
              vision[pos1] = -1;
              vision[pos2] = -1;
            }
          }
          currX++; // increment current x
        }
      } else { // it must be the case that posCoor[0] > pointCoor[0]
        int diff = posCoor[0] - pointCoor[0]; // difference in x values
        
        fprintf(stdout, "diff: %d\n", diff);

        int currX = 1;
        double currY = 0.0; 

        while( currX <= diff ){
          currY = (m * (posCoor[0]-currX)) + posCoor[1]; // y = mx + b 
          int rounded = (int) currY;
          double roundedD = (double) rounded;

          if( currY == roundedD ){ // we have the case where the point falls exactly on a point in a map (not between two points)
            int currPos = coordinatesToPos(grid, posCoor[0] - currX, (int)currY);
            
            fprintf(stdout, "currPos: %d x: %d y: %f m: %f\n", currPos, posCoor[0] - currX, currY, m);

            if( reference[currPos] == '.' && !wallFound ){
              
              //test printf
              //fprintf(stdout, "currX:%d currY:%f currPos: %d\n", currX, currY, currPos);

              vision[currPos] = 1;
            }
            else if( reference[currPos] != '.' && !wallFound ){
              vision[currPos] = 1;
              wallFound = true;
            } else {
              vision[currPos] = -1;
            }
          } else { // otherwise the point falls between two points in the map and we must check both
            int pos1 = coordinatesToPos(grid, posCoor[0] - currX, rounded);
            int pos2 = coordinatesToPos(grid, posCoor[0] - currX, rounded + 1);
            
            fprintf(stdout, "pos1: %d, pos2: %d\n", pos1, pos2);

            if(reference[pos1] == '.' && reference[pos2] == '.' && !wallFound){
              vision[pos1] = 1;
              vision[pos2] = 1;
            }
            else if(!wallFound){
              vision[pos1] = 1;
              vision[pos2] = 1;
              wallFound = true;
            } else {
              vision[pos1] = -1;
              vision[pos2] = -1;
            }
          } 
          currX++;
        }
      }
    }
  }

  return;
}

/***** updateVision *******************************************/
/*   */



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

#ifdef VISIONTEST
int 
main(int argc, char* argv[])
{
 // check args
 if( argc != 2 ){
   fprintf(stderr, "Invalid num args\n");
   exit(1);
 }
 
 FILE* fp = fopen(argv[1], "r");
 if( fp == NULL ){
   fprintf(stderr, "Invalid map pathname\n");
   exit(2);
 }
 fclose(fp);

 grid_t* grid = grid_new(argv[1]);
 if( grid == NULL ){
   fprintf(stderr, "Grid creation failure\n");
   exit(3);
 }

 int vision[grid->mapLen];
 int pos = 36;
 //int pos = ((grid->numColumns + 1) * 2) + 6;


 for(int i = 0; i < grid->mapLen; i++){
  vision[i] = 0;
 }

 calculateVision(grid, pos, vision); 
 fprintf(stdout, "\n");
 for(int i = 0; i < grid->mapLen; i++){
   if( i % 15 == 0 ){
     fprintf(stdout, "\n");
   } else {
     fprintf(stdout, "%d", vision[i]);
   }
 }

 grid_delete(grid);
 
 exit(0); 
}
#endif
