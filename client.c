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

static bool handleMessage(void* arg, const addr_t from, const char* message);
static void initialGrid(const char* gridInfo);
static void renderScreen(const char* mapString);
static void joinGame(const addr_t to);
static void leaveGame(const char* message);
static void updatePlayer(const char* message, const char* first);

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

/******************** handleMessage *****************/
static bool handleMessage(void* arg, const addr_t from, const char* message)
{
  

}

/****************** initialGrid ******************/
static void initialGrid(const char* gridInfo)
{


}

/********************** renderScreen ****************/
static void renderScreen(const char* mapString)
{


}

/******************* leaveGame *******************/
static void leaveGame(const char* message)
{



}
/******************** updatePlayer *****************/
static void updatePlayer(const char* message, const char* first)
{



}

/********************* handleInput ******************/
static bool handleInput(void* arg)
{



}

/*********************** checkInput *****************/
static bool checkInput(void* arg)
{



}

