/*
 * Team - windows_us
 * A module for the player struct and its functions, used in the Nugget game, 
 */


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "message.h"
#include "grid.h"

const char DEFAULTCHAR = '?';

typedef struct player {
  char* name;           // name provided by client
  grid_t* vision;       // map of user vision
  addr_t address;       // address of player
  char charID;          // character representation in game
  int pos;              // index position in the map string
  int gold;             // amount of gold held by player
} player_t;

//TODO: Document additions to player module in readme and specs
/**** getter functions ***************************************/

grid_t* 
player_getVision(player_t* player)
{
  return player ? player->vision : NULL;
}

char* 
player_getName(player_t* player)
{
  return player ? player->name : NULL;
}

int 
player_getPos(player_t* player)
{
  return player ? player->pos : -1;
}

int 
player_getGold(player_t* player)
{
  return player ? player->gold : 0;
}

char
player_getChar(player_t* player)
{
  return player ? player->charID : DEFAULTCHAR;
}

addr_t
player_getAddr(player_t* player)
{
  return player->address;
}

/***** setter functions **************************************/

grid_t* 
player_setVision(player_t* player, grid_t* vision)
{
  if ( player == NULL || vision == NULL ) {
    return NULL;
  }
  player->vision = vision;
  return player->vision;
}

int 
player_setPos(player_t* player, int pos)
{
  if ( player == NULL || pos < 0 ) {
    return -1;
  }
  player->pos = pos;
  return player->pos;
}

int 
player_setGold(player_t* player, int gold)
{
  if ( player == NULL || gold < 0 ) {
    return 0;
  }
  player->gold = gold;
  return player->gold;
}

addr_t
player_setAddr(player_t* player, addr_t address)
{
  player->address = address;
  return player->address;
}

char
player_setChar(player_t* player, char newChar)
{
  // return '?' if invalid params
  if (player == NULL || isalpha(newChar) == 0) {
    return DEFAULTCHAR;
  }
  // set and return
  player->charID = newChar;
  return player->charID;
}

/***** player_new ********************************************/
/* see player.h for details */ 

player_t* 
player_new(char* name)
{
  player_t* player = malloc(sizeof(player_t));

  // handle malloc error, return NULL if failure to allocate
  if ( player == NULL ) { 
    return NULL;
  } 

  // save a copy of the name string in memory and handle malloc failure
  if ((player->name = malloc(sizeof(name))) == NULL) {
    return NULL;
  }
  // copy param string into player struct
  strcpy(player->name, name);

  // initialize all other values address to defaults and return
  player->vision = NULL;
  player->pos = -1;
  player->gold = 0;
  player->charID = DEFAULTCHAR;
  player->address = message_noAddr();
  return player;
}

/***** player_addGold ****************************************/
/* see player.h for full details */
int
player_addGold(player_t* player, int newGold)
{
  // validate params
  if (player == NULL || newGold < 0) {
    return -1;
  }

  // add given amount of gold to player's total and return new value
  player->gold += newGold;
  return player->gold;
}

/***** player_delete *****************************************/
/* see player.h for full details */

void 
player_delete(player_t* player)
{
  // check param
  if ( player == NULL ) {
    return;
  }
  // free internal grid/string if not null (prevent invalid free)
  if (player->vision != NULL) {
    grid_delete(player->vision);
  }
  if (player->name != NULL) {
    free(player->name);
  }
  // finally free player 
  free(player);
}

/***** unit testing ******************************************/
/* simple unit test for the player module */

#ifdef PLAYERTEST

int
main(const int argc, char* argv[])
{
  char* name = NULL;
  
  // check command line args
  if( argc != 2 ){
    fprintf(stderr, "Player test: invalid num args\n");
    exit(1);
  }
  
  // assign name
  name = argv[1];
  
  // creating a new player
  fprintf(stdout, "Creating new player... ");
  player_t* player = player_new(name);

  if( player != NULL ){
    fprintf(stdout, "success!\n");
  } else {
    fprintf(stdout, "failed to create new player\n");
    exit(2);
  }

  fprintf(stdout, "name given: %s\n", name);
  fprintf(stdout, "name set: %s\n", player_getName(player));
  
  // testing gold
  fprintf(stdout, "Setting player gold to 50... ");
  int amt = player_setGold(player, 50);
  fprintf(stdout," set to %d\n", amt);
  fprintf(stdout, "Getting player gold... got %d\n", player_getGold(player));

  // tesing pos
  fprintf(stdout, "\nSetting player pos to 100... ");
  int position = player_setPos(player, 100);
  fprintf(stdout, " set to %d\n", position);
  fprintf(stdout, "Getting player pos... got %d\n", player_getPos(player));

  // testing vision 
  char example[] = "example str";
  fprintf(stdout, "Setting vision to %s... ", example);
  player_setVision(player, example);
  fprintf(stdout, "vision set to %s\n", player_getVision(player));

  // valgrind will show if there is mem issue
  player_delete(player);
}

#endif
