/* 
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "file.h"


static int
initializeGame(char* filepathname, int seed)
{
  // check args
  if( filepathname == NULL ){
    fprintf(stderr, "initializeGame: NULL arg given\n");
    return 1;
  }
  
  // create a filepointer and check it 
  FILE* fp = fopen(filepathname, "r");
  if( fp == NULL ){
    fprintf(stderr, "initializeGame: err creating filepointer\n");
    return 1;
  }

  // setup pseudo-random number sequence
  if( seed == NULL ){
    srand(getpid());
  } else {
    srand(seed);
  }

  /***** following section can probably be replaced with functions from the grid module *****/

  // returns the number of lines in a file, hence the number of rows in our map
  int numRows = file_numLines(fp);
  // grabs the file as one long string, will need to be free'd
  char* mapString = file_readFile(fp);
  // number of total columns and the current character from the string we are considering
  int numCols = 0;
  char currChar = "";

  // walks through the string counting the number of characters seen before a '\n' char is found (number of cols)
  while( sscanf(mapString, "%c", currChar) != EOF ){
    if( currChar == "\n" ){
      break;
    } else {
      currChar++;
    }
  }

  /*******************************************************************************************/
  
  // Generating random piles

  int piles[30];        // array of piles
  int totalGold = 250;  
  int currPile = 0;     // value (gold) of current pile
  int currIndex = 0;    // index into array
  int tmp = 0;
  
  while( totalGold > 0 ){

    if( currIndex == 29 ){ // prevents the unlikely case in which we reach 30 piles
      currPile = totalGold;
      totalGold = 0;
    } else {
      tmp = rand();
      currPile = (tmp % 25);
      totalGold -= currPile;
    }
    
    piles[currIndex] = currPile;
    currIndex++;
  }
  
  

  return 0;
}
