# CS50 Nuggets
## CS50 Winter 2022, Team 1 windows_us

### common

The `common` library contains a set of modules that are used in both the client and server for our nuggets game. 

Currently, it contains the `grid` module, which: 
Implements the map used by the nuggets game and encapsulates all functions that create, delete, or modify the in-game map.

It also contains the `player` module, which:
Implements a suite of functions to handle actions involving player. It defines a player struct, and provides functions to change that player's attributes.

### Usage

To build common.a, run `make`.
To run the grid unit test, run `make gridtest`.
To run the vision unit test, run `make visiontest`.
To run the player unit test, run  make playertest`.
To clean up, run `make clean`.

### grid

The `grid` module, as stated above, handles all creation, modification, and deletion of the in-game map. A "grid" data structure contains two copies of the map (stored as strings), a "reference" map which is read from the given map file on grid creation and remains constant, and an "active" map that is modified by the server as clients take action. The "active" map is the one rendered in-game, while the "reference" map is used to replace tiles after characters move or pick up gold. The `grid` module exports the following functions:

```c
char* grid_getReference(grid_t* grid);
char* grid_getActive(grid_t* grid);
int grid_getNumRows(grid_t* grid);
int grid_getNumColumns(grid_t* grid);
grid_t* grid_new(char* mapFile);
bool grid_replace(grid_t* grid, int pos, char newChar);
bool grid_containsEmptyTile(grid_t* grid);
bool grid_revertTile(grid_t* grid, int pos);
void grid_delete(grid_t* grid);
```

### player

The player module define and implements a structure to contain and manipulate information pertinent to a playe, including name, vision grid, address, char ID, and current gold. Includes the following types and functions:

```c
typedef struct player player_t;
grid_t* player_getVision(player_t* player);
char* player_getName(player_t* player);
int player_getPos(player_t* player);
int player_getGold(player_t* player);
char player_getCharID(player_t* player);
addr_t player_getAddr(player_t* player);
grid_t* player_setVision(player_t* player, grid_t* vision);
int player_setPos(player_t* player, int pos);
int player_setGold(plauer_t* player, in gold);
addr_t player_setAddr(player_t* player, addr_t address);
char player_setCharID(player_t* player, char newChar);
player_t* player_new(char* name, char* mapfile);
int player_addGold(player_t* player, int newGold);
char* player_summarize(player_t* player);
void player_updateVision(player_t* player, grid_t* grid);
void player_delete(player_t* player);
```

### game

The game module defines, and implements a structure to hold the state of the game, allowing the struct to be used as a global variable in `server.c` and `client.c` for readability. It also provides a range of functions to interact with a `struct game`. For more information, see the corresponding `game.h`. The `game` module exports the following functions and types:

```c
typedef struct game game_t; 
grid_t* game_getGrid(game_t* game);
int* game_getPiles(game_t* game);
hashtable_t* game_getPlayers(game_t* game);
int game_getRemainingGold(game_t* game);
int game_getLastCharID(game_t* game);
int game_getNumPlayers(game_t* game);
int game_setNumPlayers(game_t* game, int numPlayers);
bool game_setRemainingGold(game_t* game, int gold);
bool game_setGrid(game_t* game, grid_t* grid);
int game_setLastCharID(game_t* game, int charID);
game_t* game_new(int* piles, grid_t* grid);
bool game_addPlayer(game_t* game, player_t* player);
player_t* game_getPlayer(game_t* game, char* playerName);
int game_subtractGold(game_t* game, int gold);
void game_delete(game_t* game);

```

### Implementation

The common library and all modules within are implemeted according to the DESIGN and IMPLEMENTATION specs in the parent directory. 

### Assumptions

For the "grid" module we assume that the number of rows or columns of an in-game map does not exceed INT_MAX.

### Files

* `Makefile` - compilation procedure
* `grid.h` - defines the grid module
* `grid.c` - implements the grid module

### Compilation

To compile, simply `make`.
To run the `grid` unit test, type `make gridtest` and refer to `gridtest.out` for results.
To run a test of player vision, which is included in the grid module, run `make visiontest` and refer to `visiontest.out` for results.
To run the `player` unit test, type `make playertest` and refer to `playertest.out` for results.
