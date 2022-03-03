/* 
 * server.c - implements the server for Nuggets game
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "file.h"
#include "grid.h"
#include "mem.h"
#include "game.h"
#include "player.h"
#include "message.h"
#include "log.h"

// global constants
static const int goldMaxNumPiles = 40; // maximum number of gold piles
static const int goldMinNumPiles = 5;  // minimum number of gold piles
static const char ROOMTILE = '.';      // char representation of room floor
static const char GOLDTILE = '*';      // char representation of gold
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game
static const int GoldMinNumPiles = 10; // minimum number of gold piles
static const int GoldMaxNumPiles = 30; // maximum number of gold piles

// global game state
static game_t* game;

// function prototypes
static void parseArgs(const int argc, char* argv, char** filepathname, int* seed);
static int initializeGame(char* filepathname, int seed);
static int* generateGold(grid_t* grid, int seed);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static void pickupGold(int playerID, int piles[]);
static void movePlayer(int playerID, char directionChar);
static void updateClientState(char* map);

/******************** main *******************/
int
main(const int argc, char* argv[])
{
  char* filepathname = NULL;           // filepath of the map file
  int seed = NULL;                     // seed for random number gen (optional)
  int ourPort;                         // port that server runs on
  
  // initialize logging
  log_init(stderr);

  // validate arguments
  parseArgs(argc, argv, &filepathname, &seed); log_v("parseargs passed\n");
  // generate necessary data structures
  initializeGame(filepathname, seed); log_v("game initialized\n");

  // start networking and announce port number
  ourPort = message_init(stderr);
  // test port
  if (ourPort == 0) {
    fprintf(stderr, "err initializing message module\n");
    // clean up and exit
    game_delete(game);
    log_done();
    exit(1);
  }
  // log and send to terminal for clients 
  log_d("server listening on port %d\n", ourPort);
  printf("Server listening for messages on port: %d", ourPort);

  // handles inbound messages until quit message or fatal error
  if (message_loop(NULL, 0, NULL, NULL, handleMessage)) {
    // if loop completed successfully, send quit info and close down module
    // call quitGame

    // clean up and exit
    message_done();
    game_delete(game);
    log_done();
    exit(0);

  } else {
    // if loop quits unexpectedly
    // send quit message with error explanation
    log_v("unexpected error in message_loop, quitting game\n");
    // clean up and exit 
    message_done();
    game_delete(game);
    log_done();
    exit(2);
  }
}

/****************** parseArgs ******************/
/* Parses arguments for use in server.c */
static void
parseArgs(const int argc, char* argv, char** filepathname, int* seed)
{
  FILE* fp;                            // file pointer to map file for testing

  // make sure arg count is 2 or 3 (depending on if seed is passed)
  if (argc != 2 && argc != 3) {
    fprintf(stderr, "parseArgs: need either 1 arg (map file) or 2 args (map and seed)");
    exit(1);
  }

  // set seed if given
  if (argc == 3) {
    *seed = argv[2];  log_d("random seed set to %d\n", *seed);
    // TODO: Does seed have any restrictions? (cant be negative, etc.)
  }
  
  // check filepathname is not NULL
  if ((*filepathname = argv[1]) == NULL) {
    fprintf(stderr, "parseArgs: NULL arg given\n");
    exit(1);
  }
  
  // create a filepointer and check it 
  if ((fp = fopen(*filepathname, "r")) == NULL ) {
    fprintf(stderr, "parseArgs: err creating filepointer\n");
    exit(1);
  }

  fclose(fp);
}

/******************* initializeGame *************/
/* set up data structures for game */
static int
initializeGame(char* filepathname, int seed)
{
  grid_t* serverGrid = NULL;           // master grid held by server
  int* goldPiles = NULL;               // array of gold piles
  int playerList[MaxPlayers] = {0};    // array of player IDs

  // create the grid
  if ((serverGrid = grid_new(filepathname)) == NULL) {
    log_v("err loading grid from file\n");
    return -1;
  }

  // randomly distribute gold
  goldPiles = generateGold(serverGrid, seed); log_v("generated gold\n");

  // create global game state
  game = game_new(goldPiles, playerList, serverGrid); log_v("created game\n");

}

/************* generateGold **************/
/* randomly generates piles of gold and adds them to the map 
 * helper for initializeGame
 */
static int* generateGold(grid_t* grid, int seed)
{
  int piles[goldMaxNumPiles] = {-1}; // array of piles
  int totalGold = GoldTotal;         // max gold
  int currPile = 0;                  // value (gold) of current pile
  int currIndex = 0;                 // index into array
  int tmp = 0;                       // temp int
  char* active = grid_getActive(grid);
  char* reference = grid_getReference(grid);
  int gridLen = grid_getMapLen(grid);    // length of map string
  int pilesInserted = 0;
  int slot = 0;

  // setup pseudo-random number sequence
  if ( seed == NULL ) {
    srand(getpid());
  } else {
    srand(seed);
  }

  // generating random piles
  // loops until no more gold to distribute
  while ( totalGold > 0 ) {
    // prevents the unlikely case in which we reach maxPiles
    if ( currIndex == (goldMaxNumPiles - 1) ) { 
      // put remaining gold into a pile
      currPile = totalGold;
      totalGold = 0;
    } else {
      tmp = rand();
      currPile = (tmp % totalGold);
      // if random number is greater than gold left to distribute
      if (currPile > totalGold) {
        currPile = totalGold;
      }
      // to avoid a pile having zero gold
      currPile += 1;
      totalGold -= currPile;
    }
    
    piles[currIndex] = currPile; log_d("adding pile of %d gold to array\n", currPile);
    currIndex++;
  }
  
  // insert piles into map
  // loop over all piles of gold
  while ( pilesInserted < currIndex ) {   // we don't want to insert more piles than we have
    
    tmp = rand();
    slot = (tmp % gridLen);

    if ( active[slot] == ROOMTILE ) { // we only insert into valid spaces in the map
      if (grid_replace(grid, slot, GOLDTILE)) {  log_d("added gold at index %d", slot);
        pilesInserted++;
      } else {
        fprintf(stderr, "initializeGame: err inserting pile in map\n");
      }
    } 
  }
  return piles;
}

/**************** handleMessage ***************/
static bool handleMessage(void* arg, const addr_t from, const char* message)
{

}
/******************* pickupGold *************/
static void 
pickupGold(int playerID, int piles[])
{
  //update player gold total
  //update total gold remainging
  //update map to reflect absence of pile
  //update piles[] to reflect absence of pile
  //send GOLD message to all clients
  //update clients to state change

}

/**************** movePlayer *************/
static void 
movePlayer(int playerID, char directionChar)
{
  // check if move is valid
  if (directionChar == "h" || directionChar == "l" || directionChar == "j" || directionChar == "k" 
  || directionChar == "y" || directionChar == "u" || directionChar == "b" || directionChar == "n") {

    // if move is capitalized
    if(isupper(directionChar)) {

      // switch statement to handle all cases of repeat movement
      switch(directionChar) {

        // repeat move right case
        case 'L' :

        // CAN TURN THIS INTO A HELPER FUNCTION WHEN DONE

         // while the next space is an empty room spot or empty passage spot

         // while ( grid_getActive(grid)[(players[playerID])->pos)+1] == "." || grid_getActive(grid)[(players[playerID])->pos)+1] == "#" ) {

           // char next = grid_getActive(grid)[(players[playerID])->pos)];

           // if we land on a pile of gold
           // if (next == "*") {
             // pickupGold(playerID, game->piles);

          // if we hit another player, handle collision
           // if (isalpha(isupper(next))) {
                // grab player at that spot
                // switch player positions

           //}
           // players[playerID])->pos += 1;
           // update map
           // update all player vision

           //}

         //}
         // break

        // repeat move left case
        case 'H' :
          // same thing but grid_getActive(grid)[(players[playerID])->pos)-1]
          // break

        // repeat move up case 
        case 'K' :
          // same thing but grid_getActive(grid)[(players[playerID])->pos)-(grid->numColumns)]
          // break

        // repeat move down case
        case 'J' :
          // same thing but grid_getActive(grid)[(players[playerID])->pos)+(grid->numColumns)]
          // break

        // repeat move down left case
        case 'B' :
          // same thing but grid_getActive(grid)[(players[playerID])->pos)-(grid->numColumns)-1]
          // break

        // repeat move down right case
        case 'N' :
          // same thing but grid_getActive(grid)[(players[playerID])->pos)-(grid->numColumns)+1]
          // break

        // repeat move up left case
        case 'Y' :
          // same thing but grid_getActive(grid)[(players[playerID])->pos)+(grid->numColumns)-1]
          // break

        // repeat move up right case
        case 'U' :
          // same thing but grid_getActive(grid)[(players[playerID])->pos)+(grid->numColumns)+1]
          // break

      }

    } else {
      // switch statement for single space movement

      // make sure to check if next space is free


    }


  } else {
    fprintf(stderr, "invalid direction input\n");
  }
  
//if move is capitalised: 
    //while move is valid:
      //  if gold pile is in new position
      //      call goldPickup
      //  update player location
      //  update map
      //  update all player vision
//else:
   // if gold pile is in new position
    //    call goldPickup
   // update player location
   // update map
   // update all player vision

}

/******************* updateClientState *************/
static void updateClientState(char* map)
{
  //iterate through players
  //  call updateVision on a player
  //  send the player the updated map
}
