/* 
 * server.c - implements the server for Nuggets game
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "file.h"
#include "grid.h"
#include "mem.h"

// global constants
const int MAXPILES = 30;
const int MAXPLAYERS = 25;
const char ROOMTILE = '.';
const char GOLDTILE = '*';


// function prototypes
static void parseArgs(const int argc, char* argv, char** filepathname, int* seed);
static int initializeGame(char* filepathname, int seed);


/******************** main *******************/
int
main(const int argc, char* argv[])
{
  char* filepathname = NULL;
  int seed = NULL;
  parseArgs(argc, argv, &filepathname, &seed);

  initializeGame(filepathname, seed);



}

/****************** parseArgs ******************/
/* Parses arguments for use in server.c */
static void
parseArgs(const int argc, char* argv, char** filepathname, int* seed)
{

 // make sure arg count is 2 or 3 (depending on if seed is passed)
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "parseArgs: need either 1 arg (map file) or 2 args (map and seed)");
    exit(1);
  }

  *filepathname = argv[1];
  if (argc == 3) {
    *seed = argv[2];
    // TODO: Does seed have any restrictions? (cant be negative, etc.)
  }

  // check filepathname is not NULL
  if (*filepathname == NULL) {
    fprintf(stderr, "parseArgs: NULL arg given\n");
    exit(1);
  }
  
  // create a filepointer and check it 
  FILE* fp = fopen(*filepathname, "r");
  if ( fp == NULL ) {
    fprintf(stderr, "parseArgs: err creating filepointer\n");
    exit(1);
  }

}

/******************* initializeGame *************/
/* set up data structures for game */
static int
initializeGame(char* filepathname, int seed)
{
  grid_t* serverGrid = NULL;           // master grid held by server

  // setup pseudo-random number sequence
  if ( seed == NULL ) {
    srand(getpid());
  } else {
    srand(seed);
  }


  // create the grid
  grid_t* grid = grid_new(filepathname);
    
  // Generating random piles

  int piles[MAXPILES] = {-1};        // array of piles
  int totalGold = 250;  
  int currPile = 0;                  // value (gold) of current pile
  int currIndex = 0;                 // index into array
  int tmp = 0;                       // temp int to hold random number

  // initialize piles to -1
  //memset(piles, -1, sizeof(piles));
  
  while( totalGold > 0 ) {
    // prevents the unlikely case in which we reach 30 piles
    if( currIndex == (MAXPILES - 1) ) { 
      currPile = totalGold;
      totalGold = 0;
    } else {
      tmp = rand();
      currPile = (tmp % 25);
      // to avoid a pile having zero gold
      if(currPile == 0){
        currPile = 25;
      }
      totalGold -= currPile;
    }
    
    piles[currIndex] = currPile;
    currIndex++;
  }
  
  // inserting gold piles into map
  char* active = grid_getActive(grid);
  char* reference = grid_getReference(grid);
  int gridSize = strlen(reference);    // length of map string
  int pilesInserted = 0;
  int slot = 0;
  tmp = 0;
  

  while ( pilesInserted < currIndex ) {   // we don't want to insert more piles than we have
    
    tmp = rand();
    slot = (tmp % gridSize);

    if ( active[slot] == ROOMTILE ) { // we only insert into valid spaces in the map
      if (grid_replace(grid, slot, GOLDTILE)) {
        pilesInserted++;
      } else {
        fprintf(stderr, "initializeGame: err inserting pile in map\n");
      }

    } 
  }

  return 0;
}
