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
static const char PASSAGETILE = '#';   // char representation of passage tile
static const char GOLDTILE = '*';      // char representation of gold
static const char PLAYERCHAR = '@';    // player's view of themself
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 27;      // maximum number of players (and spectator)
static const int GoldTotal = 250;      // amount of gold in the game

// global game state
static game_t* game;

//TODO: CHECK ALL ITERATORS, MAKE SURE THEY SKIP SPECTATORS IF NEEDED

// function prototypes
// initialization functions and utilities
static void parseArgs(const int argc, char* argv, char** filepathname, int* seed);
static bool initializeGame(char* filepathname, int seed);
static int* generateGold(grid_t* grid, int seed);
static bool strToInt(const char string[], int* number);
// game state changes
static bool handlePlayerConnect(char* playerName, const addr_t from);
static bool pickupGold(player_t* player, int piles[]);
static void pickupGoldHelper(void* arg, const char* key, void* item);
static void movePlayer(grid_t* grid, player_t* player, game_t* game, char directionChar);
static bool movePlayerHelper(player_t* player, char directionChar, int directionValue);
static void updateClientState(char* map);
static bool handleSpectator(addr_t from);
static void handlePlayerQuit(player_t* player);
static void gameOver(bool normalExit);
static void gameOverHelper(void* arg, const char* key, void* item);
// messaging functions
static void sendGrid(addr_t to);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static void sendGold(player_t* player, int goldCollected);
static bool handleKey(char key, addr_t from);
static void sendOK(player_t* player);


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
  if (! initializeGame(filepathname, seed)) { 
    log_v("failed to initialize game, exiting non-zero");
    log_done();
    exit(3);
  } log_v("game initialized\n"); 

  // start networking and announce port number
  ourPort = message_init(stderr);
  // test port
  if (ourPort == 0) {
    fprintf(stderr, "err initializing message module");
    // clean up and exit
    game_delete(game);
    log_done();
    exit(1);
  }
  // log and send to terminal for clients 
  log_d("server listening on port %d", ourPort);
  printf("Server listening for messages on port: %d", ourPort);

  // handles inbound messages until gameOver or fatal error
  if (message_loop(NULL, 0, NULL, NULL, handleMessage)) {
    // if loop completed successfully, send quit info and close down module
    // clean up and exit
    gameOver(true);
    message_done();
    log_v("quitting game normally");
    log_done();
    exit(0);
  } else {
    // if loop quits unexpectedly
    // send quit message with error explanation
    log_v("unexpected error in message_loop, quitting game");
    // clean up and exit 
    gameOver(false);
    message_done();
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
    // convert seed string into an integer
    if ( ! strToInt(argv[2], seed)) {
      log_s("Seed: %s not a valid integer", argv[2]);
      exit(2);
    }
  }
  
  // check filepathname is not NULL
  if ((*filepathname = argv[1]) == NULL) {
    fprintf(stderr, "parseArgs: NULL arg given");
    exit(1);
  }
  
  // create a filepointer and check it 
  if ((fp = fopen(*filepathname, "r")) == NULL ) {
    fprintf(stderr, "parseArgs: err creating filepointer");
    exit(1);
  }

  fclose(fp);
}

/************* strToInt ******************/
/* convert a given string of all numbers to an integer
 * taken from knowledge units
 * returns true if successful
 * false if failure
 */
static bool strToInt(const char string[], int* number)
{
  char nextChar;
  return (sscanf(string, "%d%c", number, &nextChar) == 1);
}

/******************* initializeGame *************/
/* set up data structures for game */
static bool
initializeGame(char* filepathname, int seed)
{
  grid_t* serverGrid = NULL;           // master grid held by server
  int* goldPiles = NULL;               // array of gold piles

  // create the grid
  if ((serverGrid = grid_new(filepathname)) == NULL) {
    log_v("err loading grid from file");
    return false;
  }

  // randomly distribute gold
  goldPiles = generateGold(serverGrid, seed); log_v("generated gold");

  // create global game state
  game = game_new(goldPiles, serverGrid); log_v("created game");

  return true;
}

/************* generateGold **************/
/* randomly generates piles of gold and adds them to the map 
 * helper for initializeGame
 */
static int* generateGold(grid_t* grid, int seed)
{
  int piles[goldMaxNumPiles] = {-1};         // array of piles
  int totalGold = GoldTotal;                 // max gold
  int currPile = 0;                          // value (gold) of current pile
  int currIndex = 0;                         // index into array
  int tmp = 0;                               // temp int
  char* active = grid_getActive(grid);       // server active map
  char* reference = grid_getReference(grid); // server reference map
  int gridLen = grid_getMapLen(grid);        // length of map string
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
    // add gold pile to array of piles
    piles[currIndex] = currPile; log_d("adding pile of %d gold to array", currPile);
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
        fprintf(stderr, "initializeGame: err inserting pile in map");
      }
    } 
  }
  return piles;
}

/************* GAME FUNCTIONS ****************/
/* the functions below modify the game state
 * many of the functions are "message handlers"
 * and do things like move players, adding players to the game, etc.
 */

/************ handlePlayerConnect ************/
/* takes a given playername, which is received from a message in handleMessage
 * and create a new player struct with the given playerName
 * randomizes player's initial position and assigns a letter representation
 * then generates vision based on that position
 * sends approriate messages to all clients
 * returns true on success or non-critical error
 * false if critical error at any point in the function
 */
static bool handlePlayerConnect(char* playerName, addr_t from)
{
  player_t* player;                    // stores information for given player
  int nameLen;                         // length of playerName
  int randPos;                         // random position to drop player
  int mapLen;                          // length of in game map
  grid_t* grid;                        // game grid
  char* activeMap;                     // active map of current game
  int lastCharID;                      // most recently assigned player 'character'
  bool emptySpace = false;             // true iff there is > 1 ROOMTILE in map

  // check params (non-critical)
  if (playerName == NULL) {
    log_v("NULL playername in handleConnect");
    return true;
  }
  if ( ! message_isAddr(from)) {
    log_v("invalid address in handleConnect");
    return true;
  }
  
  // check for maxPlayers (recoverable)
  if (game_getNumPlayers(game) == MaxPlayers - 1) { // leave room for spectator
    log_v("ignoring player connect, MAXPLAYERS already reached");
    message_send(from, "QUIT Game is full: no more players can join.");
    // returns true because error is recoverable
    return true;
  }

  // modify name as applicable
  // get length of name
  nameLen = strlen(playerName);
  // truncate name if too long
  if (nameLen > MaxNameLength) {
    playerName[MaxNameLength] = '\0';
  }
  // replace non-graphics and spaces in name with underscores
  for (int i = 0; i < nameLen; i++) {
    if (isgraph(playerName[i]) == 0 || isblank(playerName[i]) != 0) {
      playerName[i] = '_';
    }
  }

  // check if existing player with same name
  // this is a recoverable error but not covered by later check
  if (game_getPlayer(game, playerName) != NULL) {
    log_s("player with name: %s already in game", playerName);
    message_send(from, "QUIT player with your name already in game");
    // recoverable error
    return true;
  }

  // initialize player and add to hashtable
  // create and check player
  if ((player = player_new(playerName)) == NULL) {
    log_s("could not create player named: %s", playerName);
    // critical malloc error
    return false;
  }
  
  // add to game and check
  if ( ! game_addPlayer(game, player)) {
    log_s("failed to add player named: %s to game", playerName);
    player_delete(player);
    // critical error
    return false;
  }

  // set attributes
  player_setAddr(player, from);
  // game holds charID as int so must be cast to char
  lastCharID = game_getLastCharID(game);
  player_setCharID(player, (char)(lastCharID));
  
  // randomize initial position
  // get length of map and map itself
  grid = game_getGrid(game);
  mapLen = grid_getMapLen(grid);
  activeMap = grid_getActive(grid);

  // check for existence of empty spaces, true if there is at least 1
  emptySpace = grid_containsEmptyTile(grid);

  // clean up and return if no space to add player
  if (! emptySpace) {
    log_s("no room in map to add player: %s", playerName);
    player_delete(player);
    message_send(from, "QUIT no room in map to add you");
    // non-critical error
    return true;
  }
  // loop until valid pos found
  while (true) {
    // constrain rand to the length of the map string
    randPos = (rand() % mapLen);
    // if empty room tile
    if (activeMap[randPos] == ROOMTILE) {
      // set player pos and update server active map
      player_setPos(player, randPos);
      grid_replace(grid, randPos, player_getCharID(player));
      break;
    }
  }
  
  // TODO: calculate their vision and send it to player with DISPLAY message
  //player_setVision(player, );
  //TODO: send new game map (including new player char) to other players
  
  sendGold(player, 0);                 // a player has no gold on entry
  sendGrid(from);
  sendOK(player);

  // return after successfully initializing all player values
  return true;
}

/**************** handleSpectator **************/
/* handles case where spectator asks to connect
 * takes an address as a parameter
 * creates a spectator player and adds them to the player list
 * with some special behavior
 * returns true if successful or non-critical error
 * false if otherwise
 * NOTE: since spectator is in hashtable
 * if looping over all players be sure to ignore those named "spectator" when appropriate
 */
static bool handleSpectator(addr_t from)
{ 
  player_t* spectator;                 // struct to hold the spectator

  // check params
  if ( ! message_isAddr(from)) {
    log_v("invalid address in handleSpectator");
    // non-critical error
    return true;
  }

  // disconnect current spectator if they exist
  if ((spectator = game_getPlayer(game, "spectator")) != NULL) {
    // send quit message to current spectator
    message_send(player_getAddr(spectator), 
                 "QUIT you have been replaced by a new spectator");
    // set spectator's address to new spectator
    player_setAddr(spectator, from);
    sendDisplay(spectator, grid_getActive(game_getGrid(game)));
    sendGold(spectator, 0);
    sendGrid(from);
    return true;
  }

  // create special spectator player if one not present
  if ((spectator = player_new("spectator")) == NULL) {
    log_v("could not allocate player struct for spectator");
    // critical error
    return false;
  }
  // add to game and check
  if (game_addPlayer(game, spectator) == false){
    player_delete(spectator);
    log_v("could not add spectator to game");
    // critical error
    return false;
  }
  
  // set relevant attributes if added to game
  player_setAddr(spectator, from);
  player_setVision(spectator, grid_getActive(game_getGrid(game)));
  
  // update spectator client
  sendGrid(from);
  sendDisplay(spectator, grid_getActive(game_getGrid(game)));
  // spectator collects no gold so send 0
  sendGold(spectator, 0);
  return true;

}

/************* handlePlayerQuit ************/
/* handles the entire process of "removing" a player from the game 
 * the function removes the players character from the in-game map
 * and sends them an appropriate quit message
 * does nothing if the player does not exist
 */
static void handlePlayerQuit(player_t* player) 
{
  grid_t* gameGrid = game_getGrid(game);  // global game's grid

  // check params
  if (player == NULL) {
    return;
  }

  // remove player from the game map and send message
  grid_revertTile(gameGrid, player_getPos);
  message_send(player_getAddr(player), "QUIT Thanks for playing!\n");
}

/*************** gameOver ******************/
/* the gameOver function encapsulates the process of ending the game
 * it takes a boolean parameter that indicates the circumstances the game ending
 * if its true, the game is exiting because the all the gold was collected
 * if false, the game is exiting due to a critical error
 */ 
static void gameOver(bool normalExit)
{
  hashtable_t* playerTable;            // table of players in game
  playerTable = game_getPlayers(game);
  char* gameSummary;                   // game over summary table
  void* container[2];                  // to pass into hashtable_iterate

  // exit procedure if error
  if ( ! normalExit) {
    // log, send message, and clean up memory then return to main
    log_v("calling gameOver(error)");
    hashtable_iterate(playerTable, &normalExit, gameOverHelper);
    game_delete(game);
    return;
  }

  // procedure if game successfully completed
  log_v("calling gameOver(success)");
  // build summary table
  gameSummary = game_buildSummary(game);

  // fill container
  container[0] = &normalExit;
  container[1] = gameSummary;

  // send table to all clients
  hashtable_iterate(playerTable, container, gameOverHelper);
  // clean up
  game_delete(game);
  free(gameSummary);
}

/************** gameOverHelper *************/
/* sends appropriate messages to all players
 * for use in gameOver, passed to hashtable_iterate
 */
static void gameOverHelper(void* arg, const char* key, void* item)
{
  void** container = arg;
  // extract player and exit status from arg
  player_t* player = item;
  bool* normalExit = container[0];
  char* gameSummary = container[1];    // summary of game for normal exit
  // set the 
  addr_t to = player_getAddr(player);

  // send current player a quit message
  if (! *normalExit) {
    message_send(to, "QUIT server encountered a critical error\n");
  } else {
    log_s("sending summary: %s", gameSummary);
    message_send(to, gameSummary);
  }
}

/***************** pickupGold *************/
/* handles case where client picks up gold
 * returns true if last pile picked up (no more gold left after player gets it)
 * so that it can be returned up the chain all the way to handleMessage
 * so that handleMessage can exit properly
 * more to come later i guess
 */
static bool
pickupGold(player_t* player, int piles[])
{
  size_t arrayLen;                     // length of game.piles

  // check params
  if (player == NULL || piles == NULL) {
    log_v("bad params in pickupGold");
    return false;
  }

  // update player gold total
  arrayLen = (sizeof(game_getPiles(game)) / sizeof(int));
  for (int i = 0; i < arrayLen; i++) {
    // skip empty piles
    if (piles[i] == -1) {
      continue;
    }
    // modify player and message
    player_addGold(player, piles[i]);
    sendGold(player, piles[i]);
    // modify game state
    game_subtractGold(game, piles[i]);
    piles[i] = -1;

    // then send GOLD and DISPLAY to all players, this time w/ 0 picked up
    // so that all clients get updated   
    // TODO: more here, read in morning and check 
    hashtable_iterate(game_getPlayers(game), NULL, pickupGoldHelper);

  }

  // return up the chain to trigger gameOver
  if (game_getRemainingGold(game) == 0) {
    return true;
  }

  // don't trigger gameOver if there is still gold
  return false;
}

/************* pickupGoldHelper *************/
/* sends the GOLD and DISPLAY messages to a client
 * for use in pickupGold, passed to hashtable_iterate
 * with the appropriate information after its called in pickupGold
 */
static void pickupGoldHelper(void* arg, const char* key, void* item) 
{
  // extract from params
  player_t* player = item;

  // update each player regarding gold remaining
  sendGold(player, 0);

  // handle the spectator seperately as they see all
  if (strcmp(player_getName(player), "spectator") == 0) {
    player_setVision(player, grid_getActive(game_getGrid(game)));
    // TODO: getVision returns a grid, need a string
    sendDisplay(player, player_getVision(player));
    return;
  }
  // update each player's display

  // calc vision here as well
  // TODO: look at how vision works and determine what to send
  player_updateVision(player, game_getGrid(game), player_getPos);

}
/************** moveIterateHelper*********/
/* helper to pass into hashtable_iterate in moveHelper
 * container passed into helper that holds the following items
 * void* container[2] = {bumpedPlayer, &next}; 
 * stores the matching player back in the container
 */
static void
moveIterateHelper(void* arg, const char* key, void* item)
{
  void** container = arg;
  char* targetCharID = container[1];
  player_t* currPlayer = item;

  // matches player with target ID
  if (player_getChar(currPlayer) == *targetCharID) {
    container[0] = currPlayer;
  }
}

/************* repeatMovePlayerHelper **********/
static void
repeatMovePlayerHelper(player_t* player, char directionChar, int directionValue)
{
  grid_t* grid = game_getGrid(game);   // in-game grid
  // character player is trying to move to
  char next = grid_getActive(grid)[player_getPos(player) + directionValue];
  
  // as long as we encounter a roomtile/passagetile/goldtile/player, move
  while (next == ROOMTILE || next == PASSAGETILE || next == GOLDTILE 
         || isupper(next) != 0) {
    // move player and update next char
    movePlayerHelper(player, directionChar, directionValue);
    next = grid_getActive(grid)[player_getPos(player) + directionValue];
  }
}

/************** movePlayerHelper ********/
//TODO: modify so that it can return a boolean when gold picked up
// then propogate up the chain
// too drunk to figure out ATM but its gotta happen
// also this doesn't handle cases where players move in passages
static bool
movePlayerHelper(player_t* player, char directionChar, int directionValue)
{
  player_t* bumpedPlayer = NULL; // player that current "mover" "collides" with
  char bumpedPlayerCharID;       // that player's char representation on the map
  hashtable_t* playerTable = game_getPlayers(game); // table of players
  grid_t* grid = game_getGrid(game); // in-game grid      
  int playerPos;                 // in game position of current player
  int bumpedPos;                 // position of bumped player, if they exist

  // grid tile that client is trying to move to 
  const char next = grid_getActive(grid)[player_getPos(player) + directionValue];
  // char representation of moving player on map
  const char playerCharID = player_getCharID(player); 
  
  playerPos = player_getPos(player);

  // if the move is valid (does not hit a wall or similar)
  if (next == ROOMTILE || next == PASSAGETILE || next == GOLDTILE 
     || isupper(next) != 0) {

    // if we land on a pile of gold
    if (next == GOLDTILE) {

    // update player gold and the game's piles
    pickupGold(player, game_getPiles(game));

    // update map with removed gold pile and new player position
    grid_revertTile(grid, player_getPos(player));
    grid_replace(grid, player_getPos(player) + directionValue, 
                 playerCharID);

    // update player's position
    player_setPos(player, player_getPos(player) + directionValue);
    
    // if we hit another player, handle collision
    } else if (isupper(next) != 0) {

      // holds two items to pass into iterator
      void* container[2] = {bumpedPlayer, &next}; 
      // iterate over the hashtable to find the player bumped into
      // assigns bumpedPlayer
      hashtable_iterate(playerTable, container, moveIterateHelper);

      // switch the positions of the colliding players
      bumpedPos = player_getPos(bumpedPlayer);
      player_setPos(player, bumpedPos);
      player_setPos(bumpedPlayer, playerPos);
      
      // update map with the new positions of both players
      bumpedPlayerCharID = player_getCharID(bumpedPlayer);
      grid_replace(grid, player_getPos(bumpedPlayer), bumpedPlayerCharID);
      grid_replace(grid, player_getPos(player), playerCharID);

    // if normal move, no gold or collision
    } else {
      // revert player's old position to reference
      grid_revertTile(grid, player_getPos(player));
      
      // then set their new position and update map accordingly
      player_setPos(player, player_getPos(player) + directionValue);
      grid_replace(grid, player_getPos(player) + directionValue, playerCharID);
    }

    // if move is invalid log and do nothing
    } else {
      log_s("invalid move request from %s", player_getName(player));
    }
}

/**************** movePlayer *************/
//TODO : ACCOUNT FOR /N AT END OF LINE? 
//TODO : UPDATE ALL PLAYER VISION?
//TODO: booleans bro
static void 
movePlayer(grid_t* grid, player_t* player, game_t* game, char directionChar)
{
  // switch statement for single space movement
  switch(directionChar) {
    // single move right case
    case 'l' :
      movePlayerHelper(player, directionChar, 1);
      break;
    // single move left case
    case 'h' :
      movePlayerHelper(player, directionChar, -1);
      break;
    // single move up case 
    case 'k' :
      movePlayerHelper(player, directionChar, -grid_getNumColumns(grid));
      break;
    // single move down case
    case 'j' :
      movePlayerHelper(player, directionChar, +grid_getNumColumns(grid));
      break;
    // single move down left case
    case 'b' :
      movePlayerHelper(player, directionChar, -grid_getNumColumns(grid)-1);
      break;
    // single move down right case
    case 'n' :
      movePlayerHelper(player, directionChar, -grid_getNumColumns(grid)+1);
      break;
    // single move up left case
    case 'y' :
      movePlayerHelper(player, directionChar, +grid_getNumColumns(grid)-1);
      break;
    // single move up right case
    case 'u' :
      movePlayerHelper(player, directionChar, +grid_getNumColumns(grid)+1);
      break;
    // repeat move right case
    case 'L' :
      repeatMovePlayerHelper(player, directionChar, 1);
      break;
    // repeat move left case
    case 'H' :
      repeatMovePlayerHelper(player, directionChar, -1);
      break;
    // repeat move up case 
    case 'K' :
      repeatMovePlayerHelper(player, directionChar, -grid_getNumColumns(grid));
      break;
    // repeat move down case
    case 'J' :
      repeatMovePlayerHelper(player, directionChar, +grid_getNumColumns(grid));
      break;
    // repeat move down left case
    case 'B' :
      repeatMovePlayerHelper(player, directionChar, -grid_getNumColumns(grid)-1);
      break;
    // repeat move down right case
    case 'N' :
      repeatMovePlayerHelper(player, directionChar, -grid_getNumColumns(grid)+1);
      break;
    // repeat move up left case
    case 'Y' :
      repeatMovePlayerHelper(player, directionChar, +grid_getNumColumns(grid)-1);
      break;
    // repeat move up right case
    case 'U' :
      repeatMovePlayerHelper(player, directionChar, +grid_getNumColumns(grid)+1);
      break;
    // default to log and ignore
    default:
      log_c("invalid char: %c in movePlayer", directionChar);
  }
}

/******************* updateClientState *************/
static void updateClientState(char* map)
{
  // CREATE AN ITERATOR AND ITERATE THROUGH HASHTABLE OF PLAYERS
  //iterate through players
  //  call updateVision on a player
  //  send the player the updated map
}

/************** MESSAGING FUNCTIONS ***************/
/* These functions handle the process of sending messages to clients
 * or handling messages received from clients
 * They are called throughout the "game functions"
 * The only message not handled here is "QUIT"
 * as the quit message varies every time 
 * and there is no real advantage to making it a function
 */

/**************** handleMessage ***************/
/* helper for message_loop, handles when server recieves a message
 * and then calls appropriate functions
 * returns false when loop should continue
 * returns true when loop should end 
 */
// TODO: This must return true at some point for message_loop to end
// should happen on pickupGold, make it work somehow (booleans my dudes)
static bool handleMessage(void* arg, const addr_t from, const char* message)
{
  char key;                            // key input from key message

  // if invalid message (bad address or null string) log and continue looping
  if ( ! message_isAddr(from) || message == NULL) {
    log_v("bad message received (bad addr or null string)");
    return false;
  }

  log_s("received message: %s", message);

  if (strncmp("PLAY ", message, 5) == 0) { 
    // send just name to helper func
    const char* content = message + strlen("PLAY ");
    // returns false on failure to create player
    if ( ! handlePlayerConnect(content, from)) {
      message_send(from, "ERROR failed to add you to game\n");
      // stop looping as critical error has occurred
      return true;
    }
  } 
  else if (strncmp("SPECTATE", message, 8) == 0) {
    if ( ! handleSpectator(from)) { 
      message_send(from, "ERROR could not add you to game\n");
    }  
  }
  else if (strncmp("KEY ", message, 4) == 0) {
    // send just key to helper func
    const char* content = message + strlen("KEY ");
    sscanf(content, "%c", &key);
    handleKey(key, from);
  } else {
    message_send(from, "ERROR message not PLAY SPECTATE or KEY\n");
    log_s("invalid message received: %s", message);
  }
  // return false to continue receiving messages
  return false;
}

/************* handleKey *******************/
/* handles key input from the client
 * and calls the appropriate function according to their input
 */
static bool handleKey(char key, addr_t from)
{
  player_t* player;                    // player that input is coming from
  bool validKey = false;               // flags if given key is valid input

  // array of all valid inputs from players
  const char playerKeys[] = {'Q', 'h', 'H', 'l', 'L', 'j', 'J', 'k', 'K', 'y',
                             'Y', 'u', 'U', 'b', 'B', 'n', 'N'};    
  const char quitKey = 'Q';            // only valid input from spectators
  
  // assign player to corresponding address
  player = game_getPlayerAtAddr(game, from);

  // check params
  if ( ! message_isAddr(from)) {
    log_V("invalid address in handleKey");
    return false;
  }

  // validate key from spectator and handle accordingly
  if (strcmp(player_getName(player), "spectator") == 0) {
    if (key == quitKey) {
      message_send(from, "QUIT Thanks for watching!\n");
      return true;
    } else {
      log_v("invalid key received");
      message_send(from, "ERROR invalid key\n");
      return false;
    }
  }

  // validate key from player
  size_t arrayLen = sizeof(playerKeys); // sizeof(char) is 1 so len = size
  for (int i = 0; i < arrayLen; i++) {
    // continue function execution if key valid
    if (key == playerKeys[i]) {
      validKey = true;
      break;
    }
  }
  // handle valid key input
  if (validKey) {
    log_v("valid key received");
    // quit if appropriate
    if (key == quitKey) {
      message_send(from, "QUIT Thanks for playing!");
      handlePlayerQuit(player);
      return true;
    } else {
      // all keys except 'Q' are movement keys
      movePlayer(game_getGrid(game), player, game, key);
      return true;
    }
  } else {
    // send error message if key is invalid
    log_v("invalid key received");
    message_send(from, "ERROR invalid key\n");
    return false;
  }
}

/************* sendGrid ****************/
/* this function sends the GRID message to a given address
 * it is abstracted here to prevent having to reference "game" each time
 * format: GRID nrows ncolumns
 */
static void sendGrid(addr_t to)
{
  char* message;                       // message to send to clients
  grid_t* grid;                        // game grid
  
  // do nothing if invalid param
  if ( ! message_isAddr(to)) {
    return;
  }
  grid = game_getGrid(game);
  // build message. allocs 2 ints, plus space for "GRID  \0"
  message = malloc((2 * sizeof(int)) + 7);
  sprintf(message, "GRID %d %d", grid_getNumRows(grid), grid_getNumColumns(grid));
  // send message
  message_send(to, message);
  free(message);
}

/************** sendGold **************/
/* this functions abstracts the process of sending the GOLD message to a client
 * it takes a player as a parameter
 * then creates the message and sends it
 * format: GOLD n p r
 * where n is number of nuggets just collected
 * p is number of nuggets in a player's purse
 * r is the number of nuggets remaining in game
 * returns nothing on success or failure, but it exits early on failure
 */
static void sendGold(player_t* player, int goldCollected)
{
  char* message;                       // message to send to client
  int playerPurse;                     // amount of gold held by player
  int remainingGold;                   // amount of gold left "on the floor"

  // check param
  if (player == NULL) {
    return;
  }
  // extract variables into more readable form
  playerPurse = player_getGold(player);
  remainingGold = game_getRemainingGold(game);

  // allocate space for 3 ints and "GOLD   \0"
  message = malloc((sizeof(int) * 3) + 8);
  // build message and send, then clean up
  sprintf(message, "GOLD %d %d %d", goldCollected, playerPurse, remainingGold);
  message_send(player_getAddr(player), message);
  free(message);
}

/************* sendOk *************/
/* this functions abstracts the process of sending the OK message to a client
 * it takes a player as a parameter, then creates the message and sends
 * format: OK char where char is the player's assigned charID
 */
static void sendOK(player_t* player)
{
  char* message;                       // message to send to client
  // do nothing if invalid param
  if (player == NULL) {
    return;
  }

  // build message. Large enough for a character and "OK \0"
  message = malloc(sizeof(char) * 5);
  sprintf(message, "OK %c", player_getCharID(player));
  message_send(player_getAddr(player), message);
  free(message);
}

/************* sendDisplay ****************/
/* this function sends the client the string it is supposed to render
 * it takes a player and a string as parameters
 * returns early on error
 */
static void sendDisplay(player_t* player, char* displayString) {
  
  addr_t to;                           // address to send message to
  char* output = "DISPLAY\n";          // beginning of display messages
  char* temp;                          // temp to check realloc results
  
  // check params
  if (player == NULL || displayString == NULL) {
    return NULL;
  }
  // get and check address
  to = player_getAddr(player);
  if ( ! message_isAddr(to)) {
    return;
  }

  // build string
  temp = realloc(output, strlen(output) + strlen(displayString) + 1);
  mem_assert(temp, "could not realloc in sendDisplay\n");

  output = temp;
  strcat(output, displayString);
  // send message and clean up
  message_send(to, output);
  free(output);
}
