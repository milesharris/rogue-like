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
static const char PLAYERCHAR = '@';    // player's view of themself
static const int MaxNameLength = 50;   // max number of chars in playerName
static const int MaxPlayers = 26;      // maximum number of players
static const int GoldTotal = 250;      // amount of gold in the game

// global game state
static game_t* game;

// function prototypes
// initialization functions
static void parseArgs(const int argc, char* argv, char** filepathname, int* seed);
static bool initializeGame(char* filepathname, int seed);
static int* generateGold(grid_t* grid, int seed);
// game state changes
static bool handlePlayerConnect(char* playerName, const addr_t from);
static void pickupGold(int playerID, int piles[]);
static void movePlayer(int playerID, char directionChar);
static void updateClientState(char* map);
static bool handleSpectator(addr_t from);

// messaging functions
static void sendGrid(addr_t to);
static bool handleMessage(void* arg, const addr_t from, const char* message);
static void sendGold(player_t* player, int goldCollected);


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
    log_v("unexpected error in message_loop, quitting game");
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

/**************** handleMessage ***************/
/* helper for message_loop, handles when server recieves a message
 * and then calls appropriate functions
 * returns false when loop should continue
 * returns true when loop should end 
 */
// TODO: This must return true at some point for message_loop to end
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
    handleKey(key);
  } else {
    message_send(from, "ERROR message not PLAY SPECTATE or KEY\n");
    log_s("invalid message received: %s", message);
  }
  // return false to continue receiving messages
  return false;
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
    message_send(player_getAddr(spectator), "QUIT another spectator connected");
    // set spectator's address to new spectator
    player_setAddr(spectator, from);
    // TODO: send DISPLAY message to new spectator
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

  // TODO: send DISPLAY message to spectator
  sendGrid(from);
  // spectator collects no gold so send 0
  sendGold(spectator, 0);
  return true;

}
/***************** pickupGold *************/
/* handles case where client picks up gold
 * more to come later
 */
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

/************** MESSAGING FUNCTIONS ***************/
/* These functions handle the process of sending messages to clients
 * or handling messages received from clients
 * They are called throughout the "game functions"
 * The only message not handled here is "QUIT"
 * as the quit message varies every time 
 * and there is no real advantage to making it a function
 */

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
 * it takes a player and a string as a parameter (probably)
 * returns early on error
 */
static void sendDisplay(player_t* player, char* displayString) {
  //TODO: code here, to finish after I know what vision looks like
}
