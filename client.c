/* 
 * client.c - implements the client for Nuggets game, windows_us team
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ncurses.h>
#include <ctype.h>
#include "file.h"
#include "message.h"
#include "player.h"


// functions
static int parseArgs(const int argc, char* argv[]);
static void initCurses();

static bool handleMessage(void* arg, const addr_t from, const char* message);
static bool initialGrid(const char* gridInfo);
static bool renderMap(const char* mapString);
static void joinGame();
static bool leaveGame(const char* message);
static bool handleError(const char* message);
static bool updatePlayer(const char* message, const char* first);

static bool handleInput(void* arg);

// static global variable, player
static player_t* player; 

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
  const char* serverPort = argv[2];
  fprintf(stderr, "Port %s\n", serverPort); // log port number to stderr
  addr_t server; 
  if (!message_setAddr(serverHost, serverPort, &server)) {
    fprintf(stderr, "Failed to form address from %s %s\n", serverHost, serverPort);
    exit(4);
  }

  player_setAddr(player, server); // player->address is address of SERVER

  // send either SPECTATE or PLAYER [playername] message to join game
  joinGame(); 
  
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
    char* playername = argv[3];
    // playername cannot be "spectator"
    if ((strcmp(playername, "spectator")) == 0) {
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
static void joinGame()
{
  const char* name = player_getName(player);

  // if spectator
  if ((strcmp("spectator", name)) == 0) {
    message_send(player_getAddr(player), "SPECTATE");
    fprintf(stderr, "SPECTATE message sent to server\n"); // log
  }

  // if player
  else {
    // construct string to send
    char playMsg[strlen("PLAY ") + strlen(name) + 1];
    snprintf(playMsg, sizeof(playMsg), "PLAY %s", name);
    
    message_send(player_getAddr(player), playMsg);

    fprintf(stderr, "PLAYER message sent to server\n"); // log
  
  }

}

/********************** initCurses ***************/
/* initializes curses */
static void initCurses()
{

  initscr();
  cbreak();
  noecho();
  start_color();
  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  attron(COLOR_PAIR(1));

} 
  
/******************** handleMessage *****************/
/* Skeleton for distributing messages depending on message type
 * Note: messages are parsed differently than requirements say.  */
static bool handleMessage(void* arg, const addr_t from, const char* message)
{
  // read first word and rest of message into separate strings
  char* first;
  char* remainder;
  char* messageCpy = malloc(strlen(message) + 1);
  strcpy(messageCpy, message);
  first = __strtok_r(messageCpy, " ", &remainder);
  free(messageCpy);

  // put cursor at 0,0 
  move(0,0);

  if ((strcmp(first, "GRID")) == 0) {
    return initialGrid(remainder);
  }

  if ((strcmp(first, "QUIT")) == 0) {
    return leaveGame(remainder);
  }

  if ((strcmp(first, "DISPLAY\n")) == 0) {
    mvprintw(1, 0, "%s", remainder);
    return renderMap(remainder);
  }

  if ((strcmp(first, "ERROR")) == 0) {
    return handleError(remainder);
  }

  // if GOLD or OK or other unknown message
  else {
    return updatePlayer(remainder, first);
  }  

}


/****************** initialGrid ******************/
/* On reception of GRID message, start ncurses and check that display will fit grid. */
static bool initialGrid(const char* gridInfo)
{
  // store nrows and ncols
  int nrows, ncols;
  sscanf(gridInfo, "%d %d", &nrows, &ncols);

  // start ncurses
  initCurses();

  // check that display fits grid; return true if it does not, otherwise return false
  int uy, ux; 
  getmaxyx(stdscr, uy, ux);
  if ((ux < ncols) || (uy < nrows)) {
    fprintf(stderr, "Window must be expanded to play Nuggets. Window is %d by %d and must be %d by %d.", uy, ux, nrows, ncols);
    return true;
  }
 
  fprintf(stderr, "Game initialized successfully.\n"); // log successful boot up
  return false;

}

/********************** renderMap ****************/
/* updates map */
static bool renderMap(const char* mapString)
{
  // print map starting at 1, 0 (header starts at 0, 0)
  mvprintw(1, 0, mapString); 
  refresh();

  return false;

}

/******************* leaveGame *******************/
/* Close ncurses
 * Print QUIT message from server
 * Delete player struct
 * Free anything else that needs it
 * Return true (to close message loop). 
 */
static bool leaveGame(const char* message)
{

  endwin(); // close ncurses
  printf("%s", message);
  player_delete(player);

  fprintf(stderr, "Game ended without fatal error."); // log successful shutdown
  return true; // ends message loop

}


/************************* handleError *****************/
/* handles ERROR message 
 */
static bool handleError(const char* message) 
{
  // prints at x = 70 to make sure it is far over
  mvprintw(0, 70, "%s                     ", message); 
  refresh();
  return false;

}


/******************** updatePlayer *****************/
/* Updates player info depending on what kind of info passed.
 */
static bool updatePlayer(const char* message, const char* first)
{

  if (strcmp(first, "OK") == 0) {
    // get first char of message
    char letter = message[0];
    player_setCharID(player, letter);
    return false;
  }

  if (strcmp(first, "GOLD") == 0) {
    // store gold info
    int n, p, r;
    sscanf(message, "%d %d %d", &n, &p, &r);
    
    const char* name = player_getName(player);

    // if spectator
    if ((strcmp("spectator", name)) == 0) {
      mvprintw(0,0, "Spectator: %d nuggets unclaimed.", r);
    }

    // if player
    else {

      // update player gold
      player_setGold(player, p);

      char letter = player_getCharID(player); 
      // if player collected gold
      if (n != 0) {
        mvprintw(0,0, "Player %c has %d nuggets (%d nuggets unclaimed). GOLD received: %d                                  ", letter, p, r, n);      
        clrtoeol();
      }
      // if player did not collect any gold
      else {
        mvprintw(0,0, "Player %c has %d nuggets (%d nuggets unclaimed).                        ", letter, p, r);
        clrtoeol();
      }
    }
    refresh();
    return false;
  }

  // if unidentifiable message type received, log error and move on
  else {
    fprintf(stderr, "Unknown message type received: %s", first);
    return false;
  }
}


/********************* handleInput ******************/
/* sends all valid input to server */
static bool handleInput(void* arg)
{

  int c = getch();
  addr_t to = player_getAddr(player);

  // if spectator
  if ((strcmp("spectator", player_getName(player))) == 0) {
    switch(c) {
    case 'Q':  message_send(to, "KEY Q"); break; 
    default: mvprintw(0, 70, "unknown keystroke               ");
    }
  }
  
  // if player
  else {
    // send char if valid keystroke
    switch(c) {
    case 'Q':   message_send(to, "KEY Q"); break;
    case 'h':   message_send(to, "KEY h"); break;
    case 'H':   message_send(to, "KEY H"); break;
    case 'l':   message_send(to, "KEY l"); break;
    case 'L':   message_send(to, "KEY L"); break;
    case 'j':   message_send(to, "KEY j"); break;
    case 'J':   message_send(to, "KEY J"); break;
    case 'k':   message_send(to, "KEY k"); break;
    case 'K':   message_send(to, "KEY K"); break;
    case 'y':   message_send(to, "KEY y"); break;
    case 'Y':   message_send(to, "KEY Y"); break;
    case 'u':   message_send(to, "KEY u"); break;
    case 'U':   message_send(to, "KEY U"); break;
    case 'b':   message_send(to, "KEY b"); break;
    case 'B':   message_send(to, "KEY B"); break;
    case 'n':   message_send(to, "KEY n"); break;
    case 'N':   message_send(to, "KEY N"); break;
    // if not valid, print error 
    default: mvprintw(0, 70, "unknown keystroke               ");
    }
  }
  
  return false;

}
  


