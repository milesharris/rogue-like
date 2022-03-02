/*
 * Team - windows_us
 * A module for the player struct and its functions, used in the Nugget game, 
 */


#include <stdlib.h>
#include <stdio.h>


typedef struct player {
  char* name;
  char* vision; // map of user vision
  int pos;
  int gold;
} player_t;

/**** getter functions ***************************************/

char* 
player_getVision(player_t* player)
{
  if( player == NULL ){
    return NULL;
  } 
  return player->vision;
}

char* 
player_getName(player_t* player)
{
  if( player == NULL ){
    return NULL;
  }
  return player->name;
}

int 
player_getPos(player_t* player)
{
  if( player == NULL ){
    return -1;
  }
  return player->pos;
}

int 
player_getGold(player_t* player)
{
  if( player == NULL ){
    return 0;
  }
  return player->gold;
}

/***** setter functions **************************************/

char* 
player_setVision(player_t* player, char* vision)
{
  if( player == NULL || vision == NULL ){
    return NULL;
  }
  player->vision = vision;
  return player->vision;
}

int 
player_setPos(player_t* player, int pos)
{
  if( player == NULL || pos < 0 ){
    return -1;
  }
  player->pos = pos;
  return player->pos;
}

int 
player_setGold(player_t* player, int gold)
{
  if( player == NULL || gold < 0 ){
    return 0;
  }
  player->gold = gold;
  return player->gold;
}

/***** player_new ********************************************/
/* see player.h for details */ 

player_t* 
player_new(char* name)
{
  player_t* player = malloc(sizeof(player_t));

  if( player == NULL ){ // malloc error
    return NULL;
  } else {
    player->name = name;
    player->vision = NULL;
    player->pos = -1;
    player->gold = 0;
  }
  
  return player;
}

/***** player_delete *****************************************/
/* see player.h for full details */

void 
player_delete(player_t* player)
{
  if( player == NULL ){
    return;
  }
  
  free(player);
  return;
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
