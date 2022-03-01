/* 
 * client.c - implements the client for Nuggets game
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "file.h"
// #include "player.h"

// functions
static int parseArgs(const int argc, char* argv[]);

static bool handleMessage(void* arg, const addr_t from, const char* message);
static void initialGrid(const char* gridInfo);
static void renderScreen(const char* mapString, player_t* player);
static void joinGame(void* arg, player_t* player);
static void leaveGame(void* arg, player_t* player);
static void updatePlayer(const char* message, const char* first, player_t* player);

static bool handleInput(void* arg);
static bool checkInput(void* arg);

// variables
static bool spectator; // i guess this also doesn't have to be global. But if it is global and static vs just declared static in the body, is that not the same thing? TODO
static player_t* player; // can also just initialize this in join game (when you call player_new) or with handleMessage



/********************* main ********************/
int
main(const int argc, char* argv[])
{


}

/******************* parseArgs *****************/
static int 
parseArgs(const int argc, char* argv[])
{

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
static void renderScreen(const char* mapString, player_t* player)
{


}
/******************** joinGame **********************/
static void joinGame(void* arg, player_t* player)
{


}

/******************* leaveGame *******************/
static void leaveGame(void* arg, player_t* player)
{



}
/******************** updatePlayer *****************/
static void updatePlayer(const char* message, const char* first, player_t* player)
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

