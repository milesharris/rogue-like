/* 
 * client.c - implements the client for Nuggets game
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "file.h"
#include "message.h"
#include "player.h"

// functions
static int parseArgs(const int argc, char* argv[]);
static void initCurses();

static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool initialGrid(const char* gridInfo);
static bool renderScreen(const char* mapString);
static void joinGame(const addr_t to);
static bool leaveGame(const char* message);
static bool updatePlayer(const char* message, const char* first);

static bool handleInput(void* arg);
static bool checkInput(void* arg);

// static global variable, player
static player_t* player; 

/* NOTES:
 *
 * How do those void args work exactly? a little confused
 * How does handleMessage know what address its coming from?
 */





/********************* main ********************/
int
main(const int argc, char* argv[])
{
 
  parseArgs(argc, argv);

  // if message module cannot be initialized
  if (message_init(NULL) == 0) {
    fprintf(stderr, "could not initialize message module");
    return 2; 
  }

  // build server address
  const char* serverHost = argv[1];
  const char* serverPort = argv[2]; // print port number? check specs
  addr_t server; 
  if (!message_setAddr(serverHost, serverPort, &server)) {
    fprintf(stderr, "Failed to form address from %s %s\n", serverHost, serverPort);
    return 4;
  }

  // send either SPECTATE or PLAYER [playername] message to join game
  joinGame(server);

  // start ncurses
  initCurses();

  // loop, waiting for input or messages
  bool ok = message_loop(&server, 0, NULL, handleInput, handleMessage);

  // close message module
  message_done();

  return ok? 0 : 1; // return status depends on result of loop

}

/******************* parseArgs *****************/
/* Parses args for use in handleMessage and handleInput.
 * Decides whether or not it is spectator. 
 * Initializes player module.  
 *
 * Assumes: playername cannot contain spaces
 */
static int 
parseArgs(const int argc, char* argv[])
{

  // check arg count
  // if spectator
  if (argc == 3) {
    player = player_new("spectator"); // spectator's player name is "spectator"
    return 0;
  }

  // if player
  else if (argc == 4) {
    const char* playername = argv[3];
    // playername cannot be "spectator"
    if (strcmp(playername, "spectator")) {
      fprintf(stderr, "usage: Playername cannot be 'spectator'");
      exit(3);
    }
    player = player_new(playername);
    return 0;
  }

  // if neither 2 nor 3 args provided
  else {
    fprintf(stderr, "usage: Client can take either 2 args (spectator) or 3 args (player). Player names cannot include spaces.\n");
    exit(3);
  }

}

/******************** joinGame **********************/
/* joins game by sending either SPECTATE or PLAYER [playername] messages to server */
static void joinGame(const addr_t to)
{
  const char* name = player_getName(player);

  // if spectator
  if (strcmp("spectator", name)) {
    message_send(to, "SPECTATE");
  }

  // if player
  else {
    // construct string to send
    char playMsg[strlen("PLAY ") + strlen(name) + 1];
    snprintf(playMsg, sizeof(playMsg), "PLAY %s", name);
    
    message_send(to, playMsg);
  
    // free playMsg? idt it is necessary, but idk why it isn't
  }
}

/********************** initCurses ***************/
/* initializes ncurses */
static void initCurses()
{
  
  initscr();
  cbreak();
  noecho();
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  attron(COLOR_PAIR(1));


} 

  // AM I DOING NCURSES RIGHT??? get help -> TA 
  // TODO: This is the plan
  //       initialize screen and whatever
  //       display header and map at all times 
  //            header: Player [letter] has [num] nuggets ([num2] nuggets unclaimed).
  //            map: most recent display sent
  //            updated after any GOLD or DISPLAY message respectively
  //       other functions will print messages to header, but only briefly displayed. 
  //       At QUIT, ncurses will exit and a final message will be sent to stdout w newline.  


/******************** handleMessage *****************/
/* Skeleton for distributing messages depending on message type */
static bool handleMessage(void* arg, const addr_t from, const char* message)
{
  // read first word and rest of message into separate strings
  char* first;
  char* remainder;
  first = strtok_r(message, " ", &remainder);
  
  if (strcmp(first, "GRID")) {
    return initialGrid(remainder);
  }

  if (strcmp(first, "QUIT")) {
    return leaveGame(remainder);
  }

  if (strcmp(first, "DISPLAY")) {
    return renderScreen(remainder);
    // TODO figure out render screen with ncurses (in renderScreen)

  // if GOLD or OK
  else {
    return updatePlayer(remainder, first);
  }  

}

/****************** initialGrid ******************/
/* On reception of GRID message, checks that display will fit grid. */
static bool initialGrid(const char* gridInfo)
{
  // store nrows and ncols
  int nrows, ncols;
  sscanf(remainder, "%d %d", &nrows, &ncols);

  // check that display fits grid; return true if it does not, otherwise return false
  int ly, lx, uy, ux; 
  getbegyx(stdscr, ly, lx);
  getmaxyx(stdscr, uy, ux);
  if (((ux - lx) < ncols) || (ly-uy) < nrows) {
    return true;
  }

  return false;


}

/********************** renderScreen ****************/
static bool renderScreen(const char* mapString)
{


}

/******************* leaveGame *******************/
static bool leaveGame(const char* message)
{



}
/******************** updatePlayer *****************/
/* Updates player info depending on what kind of info passed.
 */
static bool updatePlayer(const char* message, const char* first)
{
  if (strcmp(first, "OK")) {
    // player->letter = remainder; // update player letter, however you do it (setter?)
    // used in header display
    return false;
  }

  if (strcmp(first, "GOLD")) {
    // store gold info
    int n, p, r;
    sscanf(remainder, "%d %d %d", &n, &p, &r);
    
    // update player gold
    player_setGold(player, p);

    // print an update about gold collected
    if (n != 0) {
        // print brief message next to normal header
    }

    // update normal header 


    //    TODO: I think you need separate 'updateHeader' and 'updateMap' functions
    //    as well as a way to print to the header briefly (gold collection message, unknown keystroke, etc.)
    //    updateHeader: displays last header until new header received; then, displays new header. 
    //                  (also: modifies header based on whether player or spectator)
    //    updateMap: displays last map until new map recieved; 
    //
    // TODO I think this is an ncurses thing. Figure it out!


    return false;
  }


  }

  // if unidentifiable message type received, don't do anything
  else {
    return false;
}

/********************* handleInput ******************/
static bool handleInput(void* arg)
{



}

/*********************** checkInput *****************/
static bool checkInput(void* arg)
{



}

