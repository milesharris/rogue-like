/*
 * Team - windows_us
 * A module for the player struct and its functions, used in the Nugget game, 
 */


#include <stdlib.h>
#include <stdio.h>
#include "grid.h"
#include <string.h>
#include "message.h"

typedef struct player {
  char* name;           // name provided by client
  char letter;          // letter representation of player
  grid_t* vision;         // map of user vision
  int pos;              // index position in the map string
  int gold;             // amount of gold held by player
  addr_t* server;       // server address
} player_t;

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

char
player_getLetter(player_t* player)
{
  return player ? player->letter : NULL;
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

addr_t*
player_getServer(player_t* player)
{
  return player ? player->server : NULL;
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


char 
player_setLetter(player_t* player, char letter)
{
  if ( player == NULL || letter == NULL ) {
    return NULL;
  }
  player->letter = letter;
  return player->letter;
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

addr_t* 
player_setServer(player_t* player, addr_t* server)
{
  if ( player == NULL || server == NULL ) {
    return NULL;
  }
  player->server = server;
  return player->server;
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

  // initialize all other values to defaults and return
  player->vision = NULL;
  player->pos = -1;
  player->gold = 0;
  
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
