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
player_getCharID(player_t* player)
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
player_setCharID(player_t* player, char newChar)
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
player_new(char* name, char* mapfile)
{
  // check params
  if (name == NULL || mapfile == NULL) {
    return NULL;
  }
  
  player_t* player = malloc(sizeof(player_t));

  // handle malloc error, return NULL if failure to allocate
  if ( player == NULL ) { 
    return NULL;
  } 

  // save a copy of the name string in memory and handle malloc failure
  if ((player->name = malloc(strlen(name) + 1)) == NULL) {
    return NULL;
  }
  // copy param string into player struct
  strcpy(player->name, name);

  // the mapfile is verified by the server before ever being passed here
  grid_t* vision = grid_new(mapfile);

  // initialize values of active vision to be white space
  char* active = grid_getActive(vision);
  int mapLen = grid_getMapLen(vision);

  for(int i = 0; i < mapLen; i++){
    if(active[i] != '\n'){
      active[i] = ' ';
    }
  }

  // initialize all other values address to defaults and return
  player->vision = vision;
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

/***** player_summarize **************************************/
/* see header file for details */
char* player_summarize(player_t* player)
{
  char* summary;                       // summary string to return
  char charID;                         // player character ID
  int purse;                           // player's total gold
  char* name;                          // player's name
  size_t toAlloc;                      // memory to allocate to summary
  
  // amount to add to length of name in malloc
  // holds room for a 10-character number, a space, 2 characters, and '\0'
  const int MEMADD = 14;
  // check param
  if (player == NULL) {
    return NULL;
  }
  // values for string
  charID = player_getCharID(player);
  purse = player_getGold(player);
  name = player_getName(player);
  
  // determine amount of memory to malloc
  toAlloc = strlen(name) + MEMADD;
  // allocate and check success
  if ((summary = malloc(toAlloc)) == NULL) {
    return NULL;
  }
  sprintf(summary, "%c%10d %s\n", charID, purse, name);
  return summary;
}

/***** player_updateVision ***********************************/
/* see player.h for full details */
void
player_updateVision(player_t* player, grid_t* grid)
{
  // check parameters
  if( player == NULL || grid == NULL ){
    return;
  }
  
  int pos = player->pos;
  if( pos < 0 ){
    return;
  }

  // initialize the vision array
  size_t mapLen = grid_getMapLen(grid);
  int vision[mapLen + 1];
  //int rowLen = grid_getNumColumns(grid);

  for(int i = 0; i < mapLen + 1; i++){
    vision[i] = 0;
  }

  // populate vision array
  grid_calculateVision(grid, pos, vision);
  
  // grabbing necessary map copies
  grid_t* currPlayerVision = player_getVision(player);

  char* globalActive = grid_getActive(grid);
  char* playerActive = grid_getActive(currPlayerVision);

  if ( globalActive == NULL ) {
    return;
  }

  // updating PAST player vision to reference map values
  for(int i = 0; i < mapLen; i++){
    // inserting new line characters into vision map
    //if( i != 0 && ( i % (rowLen + 1) == 0 ) ){
    //  grid_replace(currPlayerVision, i, '\n');
    //}
    if( isblank(playerActive[i]) == 0 ){ // is slot is not whitespace, revert it to its reference map tile
      grid_revertTile(currPlayerVision, i);
    } 
    // check if the corresponding value in vision has a value of 1, in which case we use the active map value for this position
    if( vision[i] == 1 ){
      char newChar =  globalActive[i];
      grid_replace(currPlayerVision, i, newChar);
    }
  }

  return;
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
  char* mapFile = NULL;
  
  // check command line args
  if( argc != 3 ){
    fprintf(stderr, "Player test: invalid num args\n");
    exit(1);
  }
  
  // assign name
  name = argv[1];
  mapFile = argv[2];

  // make grid
  fprintf(stdout, "Creating grid... ");
  grid_t* grid = grid_new(mapFile);
  
  if( grid != NULL ){
    fprintf(stdout, "success\n");
  } else {
    fprintf(stdout, "failed\n");
    exit(2);
  }
  
  // creating a new player
  fprintf(stdout, "Creating new player... ");
  player_t* player = player_new(name);

  if( player != NULL ){
    fprintf(stdout, "success!\n");
  } else {
    fprintf(stdout, "failed to create new player\n");
    exit(3);
  }

  fprintf(stdout, "name given: %s\n", name);
  fprintf(stdout, "name set: %s\n", player_getName(player));
  
  // testing gold
  fprintf(stdout, "Setting player gold to 50... ");
  int amt = player_setGold(player, 50);
  fprintf(stdout," set to %d\n", amt);
  fprintf(stdout, "Getting player gold... got %d\n", player_getGold(player));

  // tesing pos
  fprintf(stdout, "\nSetting player pos to 1394... ");
  int position = player_setPos(player, 1394);
  fprintf(stdout, " set to %d\n", position);
  fprintf(stdout, "Getting player pos... got %d\n", player_getPos(player));

  // testing vision 
  player_updateVision(player, grid);
  grid_t* vision = player_getVision(player);

  if( vision == NULL ){
    fprintf(stdout, "failed to get vision\n");
  }
  
  char* visionActive = grid_getActive(vision);
  int mapLen = grid_getMapLen(vision);

  fprintf(stdout, "----- vision map ---------------------------------------------------------------");
  for(int i = 0; i < mapLen; i++){
    fprintf(stdout, "%c", visionActive[i]);
  }

  // valgrind will show if there is mem issue
  player_delete(player);
}

#endif
