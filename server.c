/* 
 * server.c - defines implements the server for rogue-like game
 * see design and implementation specs for more details
 * Miles Harris, Summer 2022
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

/****************** TO-DO LIST *********************/
/* 0. Debug original nuggets client, learn ncurses, etc. 
 * 1. Make a multi-leveled dungeon, but keep everything else the same so just multi level nuggets
 * 2. Implement the amulet and Rogue victory conditions, maybe basic inventory
 * 3. Make it all turn-based (yikes)
 * 4. Add monsters into the game without combat, make them random walkers
 * 5. Make monsters hunt players in sight, but keep nuggets "bumping" rules
 * 6. Combat, player stats, death
 * 7. Full inventory w/ item pickups and potions and shit I guess
 * 8. God even knows at this point I might be insane
 */


// global constants
static const int goldMaxNumPiles = 30; // maximum number of gold piles
static const int goldMinNumPiles = 10; // minimum number of gold piles
static const char ROOMTILE = '.';      // char representation of room floor
static const char PASSAGETILE = '#';   // char representation of passage tile
static const char GOLDTILE = '*';      // char representation of gold
static const char PLAYERCHAR = '@';    // player's view of themself
static const char ZOMBIECHAR = 'Z';    // representation of zombie on map
static const char SKELETONCHAR = 'S';  // representation of skeleton on map
static const char GHOULCHAR = 'G';     // representation of ghoul on map
static const char AMULETCHAR = '$';    // representation of amulet on map
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 5;       // maximum number of players (and spectator)
static const int GoldTotal = 250;      // amount of gold per floor

// global game state
static game_t* game;

// function prototypes
// initialization functions and utilities
static void parseArgs(const int argc, char* argv[], char** filepathname, int* seed);
static bool initializeGame(char* filepathname, int seed);
static int generateGold(grid_t* grid, int* piles, int seed);
static bool strToInt(const char string[], int* number);
// game state changes
static bool handlePlayerConnect(char* playerName, const addr_t from);
static bool pickupGold(player_t* player);
static void pickupGoldHelper(void* arg, const char* key, void* item);
static bool movePlayer(player_t* player, char directionChar);
static bool movePlayerHelper(player_t* player, int directionValue);
static void updatePlayersVision();
static void updateHelper(void* arg, const char* key, void* item);
static bool handleSpectator(addr_t from);
static void handlePlayerQuit(player_t* player);
static void gameOver(bool normalExit);
static void gameOverHelper(void* arg, const char* key, void* item);
// messaging functions
static void sendGrid(addr_t to);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static void sendGold(player_t* player, int goldCollected);
static bool handleKey(const char key, addr_t from);
static void sendOK(player_t* player);
static void sendDisplay(player_t* player, char* displayString);

/******************** main *******************/
/* master function for the server
 * initializes all modules
 * loops to receive messages until fatal error or game ends
 * exits 0 if game ends normally, non-zero if error
 */
int main(const int argc, char* argv[])
{
  char* filepathname = NULL;           // filepath of the map file
  // seed for random number gen, changed in parseArgs if applicable
  int seed = getpid();               
  int ourPort;                         // port that server runs on
  
  // initialize logging
  log_init(stderr);

  // validate arguments
  parseArgs(argc, argv, &filepathname, &seed); 
  log_v("Commandline arguments valid");

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
    log_v("err initializing message module");
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
    log_v("quitting game normally");
    // clean up and exit
    gameOver(true);
    message_done();
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
parseArgs(const int argc, char* argv[], char** filepathname, int* seed)
{
  FILE* fp;                            // file pointer to map file for testing

  // make sure arg count is 2 or 3 (depending on if seed is passed)
  if (argc != 2 && argc != 3) {
    log_v("parseArgs: need either 1 arg (map file) or 2 args (map and seed)");
    log_done();
    exit(1);
  }

  // set seed if given
  if (argc == 3) {
    // convert seed string into an integer
    if ( ! strToInt(argv[2], seed) || *seed < 0) {
      log_s("Seed: %s not a valid integer", argv[2]);
      log_done();
      exit(2);
    }
  }
  
  // check filepathname is not NULL
  if ((*filepathname = argv[1]) == NULL) {
    log_v("parseArgs: NULL arg given");
    log_done();
    exit(1);
  }
  
  // create a filepointer and check it 
  if ((fp = fopen(*filepathname, "r")) == NULL ) {
    log_v("parseArgs: err creating filepointer");
    log_done();
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
/* set up data structures for game 
 * allocates memory for the global game struct using game_new
 * which must later be free'd using game_delete
 */
static bool initializeGame(char* filepathname, int seed)
{
  grid_t* serverGrid = NULL;           // master grid held by server
  int numPiles;                        // number of gold piles generated
  
  // create the grid
  if ((serverGrid = grid_new(filepathname)) == NULL) {
    log_v("err loading grid from file");
    return false;
  }
  
  // create and check piles array
  size_t toAlloc = (goldMaxNumPiles * sizeof(int));
  int* goldPiles = mem_malloc_assert(toAlloc, "failed to alloc piles");
  // set all values in the array to -1
  for(int i = 0; i < goldMaxNumPiles; i++){
    goldPiles[i] = -1;
  }

  // randomly distribute gold
  numPiles = generateGold(serverGrid, goldPiles, seed); 
  log_v("generated gold");
  log_v("piles array initially:");
  for (int i = 0; i < goldMaxNumPiles; i++) {
    log_d("%d", goldPiles[i]);
  }
  
  // create global game state
  game = game_new(goldPiles, serverGrid);
  game_setNumPiles(game, numPiles);
  log_v("created game");

  return true;
}

/************* generateGold **************/
/* randomly generates piles of gold and adds them to the map
 * returns the number of piles generated 
 * helper for initializeGame
 */
static int generateGold(grid_t* grid, int* piles, int seed)
{
  int totalGold = GoldTotal;                 // max gold
  int currPile = 0;                          // value (gold) of current pile
  int currIndex = 0;                         // index into array
  int tmp = 0;                               // temp int
  char* active = grid_getActive(grid);       // server active map
  int gridLen = grid_getMapLen(grid);        // length of map string
  int pilesInserted = 0;
  int slot = 0;

  // seed random gen
  srand(seed);

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
      // divides to get a more balanced distribution
      currPile = (tmp % (totalGold/goldMinNumPiles));
      // if random number is greater than gold left to distribute
      if (currPile > totalGold) {
        currPile = totalGold;
      }
      // to avoid a pile having zero gold
      currPile += 1;
      totalGold -= currPile;
    }
    // add gold pile to array of piles
    piles[currIndex] = currPile; 
    log_d("adding pile of %d gold to array", currPile);
    // update index and game state
    currIndex++;
  }

  // insert piles into map
  // loop over all piles of gold
  while ( pilesInserted < currIndex ) {   // we don't want to insert more piles than we have
    
    tmp = rand();
    slot = (tmp % gridLen);

    if ( active[slot] == ROOMTILE ) { // we only insert into valid spaces in the map
      if (grid_replace(grid, slot, GOLDTILE)) {  
        log_d("added gold at index %d", slot);
        pilesInserted++;
      } else {
        log_v("initializeGame: err inserting pile in map");
      }
    } 
  }
  return currIndex;
}

/************* GAME FUNCTIONS ****************/
/* the functions below modify the game state
 * many of the functions are "message handlers"
 * and do things like move players, adding players to the game, etc.
 */

/************ handlePlayerConnect ************/
/* takes a given playername, which is received from a message in handleMessage
 * allocates a new player struct with the given playerName
 * that must later be free'd using player_delete
 * within the server, this is done using the game_delete function
 * 
 * randomizes player's initial position and assigns a letter representation
 * then generates vision based on that position
 * sends approriate messages to all clients
 * returns true on success or non-critical error
 * false if critical error at any point in the function
 */
static bool handlePlayerConnect(char* playerName, addr_t from)
{
  player_t* player;                      // stores information for given player
  int nameLen;                           // length of playerName
  int randPos;                           // random position to drop player
  int mapLen;                            // length of in game map
  grid_t* grid;                          // game grid
  char* activeMap;                       // active map of current game
  int lastCharID;                        // most recently assigned player 'character'
  bool emptySpace = false;               // true iff there is > 1 ROOMTILE in map
  char* mapfile = game_getMapfile(game); // game map used to initialize player vision

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
  if (game_getNumPlayers(game) == MaxPlayers) { 
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
  if ((player = player_new(playerName, mapfile)) == NULL) {
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
  
  // update client with their ID and the state of the game
  sendOK(player);
  sendGrid(from);
  sendGold(player, 0);                 // a player has no gold on entry

  // update all player's vision with new information
  updatePlayersVision();
  // return after successfully initializing all player values
  return true;
}

/**************** handleSpectator **************/
/* handles case where spectator asks to connect
 * takes an address as a parameter
 * creates a spectator player (mallocs memory) and adds them to the player list
 * with some special behavior
 * that must be free'd later using player_delete, called in game_delete
 * 
 * returns true if successful or non-critical error
 * false if otherwise
 * NOTE: since spectator is in hashtable
 * if looping over all players be sure to ignore those named "spectator" when appropriate
 */
static bool handleSpectator(addr_t from)
{ 
  player_t* spectator;                   // struct to hold the spectator
  char* mapfile = game_getMapfile(game); // mapfile used by the server
  
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
  if ((spectator = player_new("spectator", mapfile)) == NULL) {
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
  // note that vision does not need to be send
  // spectator's display is always server's active map
  player_setAddr(spectator, from);
  
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
  grid_revertTile(gameGrid, player_getPos(player));
  message_send(player_getAddr(player), "QUIT Thanks for playing!\n");
  // remove player from all other's screens
  updatePlayersVision();
}

/*************** gameOver ******************/
/* the gameOver function encapsulates the process of ending the game
 * it takes a boolean parameter 
 * that indicates the circumstances of the game ending
 * calls game_delete, free'ing all memory held within the game struct
 * if its true, the game is exiting because the all the gold was collected
 * if false, the game is exiting due to a critical error
 */ 
static void gameOver(bool normalExit)
{
  hashtable_t* playerTable;            // table of players in game
  playerTable = game_getPlayers(game);
  char* gameSummary;                   // game over summary table

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
  log_s("gameSummary: is %s", gameSummary);
  // to pass into hashtable_iterate
  void* container[2] = {&normalExit, gameSummary};

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
  // log status of bool
  if (*normalExit) {
    log_v("normalExit true");
  } else {
    log_v("normalExit false");
  }

  char* gameSummary = container[1];    // summary of game for normal exit
  log_s("gameSummary initial: %s", gameSummary);

  // set the address to send messages to
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
 * passed a player, who is the one picking up the gold
 * returns true if last pile picked up (no more gold left after player gets it)
 * so that it can be returned up the chain all the way to handleMessage
 * so that handleMessage can exit properly and the game can end
 * returns false if otherwise
 */
static bool
pickupGold(player_t* player)
{
  int* piles = game_getPiles(game);   // array of gold piles in game

  // check params and values
  if (player == NULL || piles == NULL) {
    log_v("bad params in pickupGold");
    return false;
  }

  // update player gold total
  for (int i = 0; i < game_getNumPiles(game); i++) {
    const int currPile = piles[i];
    // skip empty piles
    if (currPile == -1) {
      log_d("skipping empty pile of gold, value is: %d", piles[i]);
      continue;
    }
    // modify player and game state
    player_addGold(player, currPile);
    game_subtractGold(game, currPile);
    piles[i] = -1;
    
    // notify player
    sendGold(player, currPile);

    // notify all players of new gold state using GOLD message w/ 0 picked up
    hashtable_iterate(game_getPlayers(game), player, pickupGoldHelper);
    // exit loop once gold picked up
    break;
  }

  // return up the chain to trigger gameOver if all gold collected
  if (game_getRemainingGold(game) == 0) {
    return true;
  }

  // don't trigger gameOver if there is still gold
  return false;
}

/************* pickupGoldHelper *************/
/* sends the GOLD message to a client saying they picked up 0 gold
 * for use in pickupGold, passed to hashtable_iterate
 * with the appropriate information after its called in pickupGold
 * serves only to update all players about the game state
 * when a player picks up gold
 */
static void pickupGoldHelper(void* arg, const char* key, void* item) 
{
  // extract from params
  player_t* triggerPlayer = arg;       // player who picked up gold
  player_t* currPlayer = item;         // current player in iteration

  // update each player regarding gold remaining
  if (strcmp(player_getName(triggerPlayer), player_getName(currPlayer)) != 0) {
    // dont double-update player who picked up gold
    sendGold(currPlayer, 0);
  }
  
}
/************** moveIterateHelper*********/
/* helper to pass into hashtable_iterate in moveHelper
 * finds a player with a provided charID
 * stores the matching player back in the container of void pointers
 */
static void
moveIterateHelper(void* arg, const char* key, void* item)
{
  void** container = arg;
  char* targetCharID = container[1];
  player_t* currPlayer = item;

  // matches player with target ID
  char targetID = *targetCharID;
  char currID = player_getCharID(currPlayer);
  if (targetID == currID) {
    container[0] = item;
  }
}

/************* repeatMovePlayerHelper **********/
/* repeatedly moves a player by a given integer value
 * where the integer represents the distance moved in the in-game map
 * returns true if, at any point in the "big move", the last gold is collected
 * false if otherwise
 */
static bool
repeatMovePlayerHelper(player_t* player, int directionValue)
{
  bool gameOverFlag = false;           // set to true if last gold picked up
  grid_t* grid = game_getGrid(game);   // in-game grid
  // character player is trying to move to
  char next = grid_getActive(grid)[player_getPos(player) + directionValue];
  
  // as long as we encounter a roomtile/passagetile/goldtile/player, move
  while (next == ROOMTILE || next == PASSAGETILE || next == GOLDTILE 
         || isupper(next) != 0) {
    // move player and update next char
    gameOverFlag = movePlayerHelper(player, directionValue);
    // return early if game ends before move ends
    if (gameOverFlag) {
      return gameOverFlag;
    }
    next = grid_getActive(grid)[player_getPos(player) + directionValue];
  }
  // returns false if game continues, true if it ends
  return gameOverFlag;
}

/************** movePlayerHelper ********/
/* handles the actual in-game process of moving players
 * takes the player to move, and an integer representing the distance
 * to shift the player's position in the in-game map
 * moves the player, picks up gold if necessary, and updates all vision
 * returns true if player picks up gold and there is no gold remaining
 * false if otherwise
 */
static bool
movePlayerHelper(player_t* player, int directionValue)
{
  player_t* bumpedPlayer = NULL; // player that current "mover" "collides" with
  char bumpedPlayerCharID;       // that player's char representation on the map
  hashtable_t* playerTable = game_getPlayers(game); // table of players
  grid_t* grid = game_getGrid(game); // in-game grid      
  int playerPos;                 // in game position of current player
  int bumpedPos;                 // position of bumped player, if they exist
  bool gameOverFlag = false;     // becomes true if pickupGold returns true

  // grid tile that client is trying to move to 
  char next = grid_getActive(grid)[player_getPos(player) + directionValue];
  log_c("in move, nextChar = %c", next);
  // char representation of moving player on map
  const char playerCharID = player_getCharID(player); 
  playerPos = player_getPos(player);

  // if the move is valid (does not hit a wall or similar)
  if (next == ROOMTILE || next == PASSAGETILE || next == GOLDTILE 
      || isupper(next) != 0) {

    // if we land on a pile of gold
    if (next == GOLDTILE) {
      log_v("nextchar is a goldtile");
      // update map with removed gold pile and new player position
      grid_revertTile(grid, player_getPos(player));
      player_setPos(player, player_getPos(player) + directionValue);
      grid_replace(grid, player_getPos(player), playerCharID);

      // update player gold and the game's piles
      gameOverFlag = pickupGold(player);

    // if we hit another player, handle collision
    } else if (isupper(next) != 0) {
      log_v("handling a collision");
      // holds two items to pass into iterator
      void* voidPlayer = NULL;
      void* container[2] = {voidPlayer, &next}; 
      
      // iterate over the hashtable to find the player bumped into
      // assigns bumpedPlayer
      hashtable_iterate(playerTable, container, moveIterateHelper);
      bumpedPlayer = container[0];

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
      log_v("making a normal move");
      // revert player's old position to reference
      grid_revertTile(grid, player_getPos(player));
      
      // then set their new position and update map accordingly
      player_setPos(player, player_getPos(player) + directionValue);
      grid_replace(grid, player_getPos(player), playerCharID);
    }
  // if move is invalid log and do nothing
  } else {
      log_s("invalid move request from %s", player_getName(player));
      // no need to update vision if the player never actually moved
      return gameOverFlag;
  }
  // update all client's vision after a move
  updatePlayersVision();
  // true if no more gold in the game, false if otherwise
  return gameOverFlag;
}

/**************** movePlayer *************/
/* Master function to move a given player
 * takes a player and a pre-validated move key as parameters 
 * Mostly a switch statement calling helper function for each case
 * returns false if a move does not collect the last pile of gold
 * so that message_loop will continue looping
 * and true if the last pile is collected or a critical error occurred
 * so that message_loop will stop looping and call gameOver()
 */ 
static bool 
movePlayer(player_t* player, char directionChar)
{
  bool gameOverFlag = false;           // set to true if all gold collected
  grid_t* grid = game_getGrid(game);   // game grid
  // calls appropriate function for given move char
  switch(directionChar) {
    // single move right case
    case 'l' :
      gameOverFlag = movePlayerHelper(player, 1);
      break;
    // single move left case
    case 'h' :
      gameOverFlag = movePlayerHelper(player, -1);
      break;
    // single move up case 
    case 'k' :
      // make move negative, and subtract 1 to account for newline
      gameOverFlag = movePlayerHelper(player, 0 - grid_getNumColumns(grid) - 1);
      break;
    // single move down case
    case 'j' :
      // add 1 to account for newline
      gameOverFlag = movePlayerHelper(player, grid_getNumColumns(grid) + 1);
      break;
    // single move down left case
    case 'b' :
      // ncolumns is 1 less than the move needed for pure down
      gameOverFlag = movePlayerHelper(player, grid_getNumColumns(grid));
      break;
    // single move down right case
    case 'n' :
      // add 2 to ncolumns, 1 to get pure down then 1 to go right
      gameOverFlag = movePlayerHelper(player, grid_getNumColumns(grid) + 2);
      break;
    // single move up left case
    case 'y' :
      // make negative, then subtract 1 for pure up then 1 to go left
      gameOverFlag = movePlayerHelper(player, 0 - grid_getNumColumns(grid) - 2);
      break;
    // single move up right case
    case 'u' :
      // make negative to move up, ncolumns is 1 less than a pure up
      gameOverFlag = movePlayerHelper(player, 0 - grid_getNumColumns(grid));
      break;
    // repeat move right case
    case 'L' :
      gameOverFlag = repeatMovePlayerHelper(player, 1);
      break;
    // repeat move left case
    case 'H' :
      gameOverFlag = repeatMovePlayerHelper(player, -1);
      break;
    // repeat move up case 
    case 'K' :
      gameOverFlag = repeatMovePlayerHelper(player, 0 - grid_getNumColumns(grid) - 1);
      break;
    // repeat move down case
    case 'J' :
      gameOverFlag = repeatMovePlayerHelper(player, grid_getNumColumns(grid) + 1);
      break;
    // repeat move down left case
    case 'B' :
      gameOverFlag = repeatMovePlayerHelper(player, grid_getNumColumns(grid));
      break;
    // repeat move down right case
    case 'N' :
      gameOverFlag = repeatMovePlayerHelper(player, grid_getNumColumns(grid) + 2);
      break;
    // repeat move up left case
    case 'Y' :
      gameOverFlag = repeatMovePlayerHelper(player, 0 - grid_getNumColumns(grid) - 2);
      break;
    // repeat move up right case
    case 'U' :
      gameOverFlag = repeatMovePlayerHelper(player, 0 - grid_getNumColumns(grid));
      break;
    // default to log and ignore
    default:
      log_c("invalid char: %c in movePlayer", directionChar);
      break;
  }
  // returns true if all gold picked up
  return gameOverFlag;
}

/****************** updateHelper ******************/
/* helper function for updatePlayersVision
 * passed into hashtable_iterate
 * does all the work of updating vision and sending display messages
 */
static void updateHelper(void* arg, const char* key, void* item)
{
  player_t* currPlayer = item;         // current player struct in hashtable
  grid_t* playerVisionGrid;            // current player's vision
  int playerPos;                       // current player's position
  
  // handle spectator differently
  if (strcmp(player_getName(currPlayer), "spectator") == 0) {
    log_v("updating spectator vision");
    // send them the active map, don't bother changing their vision
    sendDisplay(currPlayer, grid_getActive(game_getGrid(game)));
    return;
  }

  // handle normal players
  log_s("updating %s's vision", player_getName(currPlayer));
  playerVisionGrid = player_getVision(currPlayer);
  playerPos = player_getPos(currPlayer);

  // calculate and update a player's vision grid
  player_updateVision(currPlayer, game_getGrid(game));
  // replace the character at the player's position with the '@' symbol
  // in the player's local vision string
  grid_replace(playerVisionGrid, playerPos, PLAYERCHAR);

  // message player with updated vision
  sendDisplay(currPlayer, grid_getActive(player_getVision(currPlayer)));
}

/******************* updatePlayersVision *************/
/* updates vision for all players currently in the game
 * handles spectator seperately as vision functions don't work on them
 * then sends the DISPLAY message with appropriate vision string
 * takes no parameters and returns void
 */
static void updatePlayersVision()
{
  hashtable_t* playerTable;            // table of players in game

  // assign and check playerTable
  playerTable = mem_assert(game_getPlayers(game), 
                           "players NULL in updateVision"); 

  // iterate over all players and update their vision
  hashtable_iterate(playerTable, NULL, updateHelper);
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
 * returns false when loop should continue (non-critical errors) or normal behavior
 * returns true when loop should end (critical errors or last gold collected)
 */
static bool handleMessage(void* arg, const addr_t from, const char* message)
{
  //char key;                            // key input from key message
  bool gameOverFlag = false;           // true if all gold collected

  // if invalid message (bad address or null string) log and continue looping
  if ( ! message_isAddr(from) || message == NULL) {
    log_v("bad message received (bad addr or null string)");
    return false;
  }

  log_s("received message: %s", message);

  if (strncmp("PLAY ", message, 5) == 0) { 
    
    // workaround for param being const but needs to be modified
    char* messageCopy = mem_malloc_assert(strlen(message) + 1, 
                                          "failed to alloc message copy\n");
    strcpy(messageCopy, message);
    
    // send just playername to handlePlayerConnect
    char* content = messageCopy + strlen("PLAY ");

    // returns false on failure to create player
    if ( ! handlePlayerConnect(content, from)) {
      message_send(from, "ERROR failed to add you to game\n");
      free(messageCopy);
      // stop looping as critical error has occurred
      return true;
    }
    // clean up memory
    free(messageCopy);
  } 
  else if (strncmp("SPECTATE", message, 8) == 0) {
    if ( ! handleSpectator(from)) { 
      message_send(from, "ERROR could not add you to game\n");
    }  
  }
  else if (strncmp("KEY ", message, 4) == 0) {
    // send just key to helper func
    const char key = message[4];
    // set to true if gold picked up and remaining is 0
    gameOverFlag = handleKey(key, from);
    return gameOverFlag;
  } else {
    message_send(from, "ERROR message not PLAY SPECTATE or KEY\n");
    log_s("invalid message received: %s", message);
  }
  // return true if game over or critical error to end loop
  // false otherwise
  return gameOverFlag;
}

/************* handleKey *******************/
/* handles key input from the client
 * and calls the appropriate function according to their input
 * returns true if the message_loop should stop looping
 * which happens when the game ends or encounters a critical error
 * and false if it should continue
 */
static bool handleKey(const char key, addr_t from)
{
  player_t* player;                    // player that input is coming from
  bool validKey = false;               // flags if given key is valid input
  bool gameOverFlag = false;           // true if all gold picked up
  // array of all valid inputs from players
  const char playerKeys[17] = {'Q', 'h', 'H', 'l', 'L', 'j', 'J', 'k', 'K', 'y',
                               'Y', 'u', 'U', 'b', 'B', 'n', 'N'};    
  const char quitKey = 'Q';            // only valid input from spectators
  
  // assign player to corresponding address
  if ((player = game_getPlayerAtAddr(game, from)) == NULL) {
    log_v("failed to get player from addr passed to handleKey");
    return false;
  }

  // check params
  if ( ! message_isAddr(from)) {
    log_v("invalid address in handleKey");
    // non-critical
    return false;
  }

  // validate key from spectator and handle accordingly
  if (strcmp(player_getName(player), "spectator") == 0) {
    log_v("player is spectator, only allowing 'Q' key");
    if (key == quitKey) {
      message_send(from, "QUIT Thanks for watching!\n");
      return false;
    } else {
      message_send(from, "ERROR invalid key for spectator");
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
      // send message, remove chaar from map, and continue looping
      handlePlayerQuit(player);
      return gameOverFlag;
    } else {
      // all keys except 'Q' are movement keys
      gameOverFlag = movePlayer(player, key);
      // will be true if player moved and collected last pile of gold
      return gameOverFlag;
    }
  } else {
    // send error message if key is invalid
    message_send(from, "ERROR invalid key for player");
    return gameOverFlag;
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
  char* initial = "DISPLAY\n";         // beginning of display messages
  char* message = NULL;                // final message sent to clients
  
  // check params
  if (player == NULL || displayString == NULL) {
    return;
  }
  // get and check address
  to = player_getAddr(player);
  if ( ! message_isAddr(to)) {
    return;
  }

  // build string
  message = mem_malloc_assert(strlen(initial) + strlen(displayString) + 1, 
                              "failed to alloc message in sendDisplay\n");
  strcpy(message, initial);
  strcat(message, displayString);
  // send message and clean up
  message_send(to, message);
  free(message);
}
