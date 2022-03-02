# CS50 Nuggets Implementation 
# CS50 Nuggets
## Implementation Spec
### windows_us, Winter 2022

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes the player and grid modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Plan for division of labor

Ethan: Client, Client Testing, nCurses.

Miles: Grid, Grid Testing, Server/Client Communication, overall module integration.

Matthew: Player movement (serverside).

Karsten: Vision and vision testing, Player.

All: Server (split by function), Server Testing, Documentation.

## Client

### Data structures

Client will use the `struct player`, as defined below. 

### Definition of function prototypes

```c
static int parseArgs(const int argc, char* argv[]);
```
Parses arguments to make sure they are valid. Decide if client is player or spectator. 

```c
static bool handleMessage(void* arg, const addr_t from, const char* message);
```
Framework for receiving messages; calls appropriate function based on message received. Returns true on fatal error. 

```c
static bool handleInput(void* arg);
```
Reads lines from stdin and sends them to server if valid. Returns true if message loop should end. 

```c
static bool checkInput(void* arg);
```
Checks input to ensure it is valid. Returns true if yes, false if no. Helper to handleInput. 

```c
static void initialGrid(const char* gridInfo);
```
Deals with initial GRID message (checks size of display). 

```c
static void renderScreen(const char* mapString, player_t* player);
```
Renders the map. 

```c
static void joinGame(void* arg, player_t* player);
```
Sends message containing player's 'real name' (taken from the command line) and initializes player. If there was no player name passed in from the command line (so the client is a spectator) send the `SPECTATOR` message.

```c
static void leaveGame(const char* message, player_t* player);
```
Prints disconnect message from server and frees all data structures. 

```c
static void updatePlayer(const char* message, const char* first, player_t* player);
```
Updates player struct as new information (GOLD, OK, DISPLAY) comes in. 


### Detailed pseudo code


#### `parseArgs`:

	validate commandline
	initialize message module
	print assigned port number
	decide whether spectator or player
    initialize player
    
#### `handleMessage`:
    
    read first word into first string
    read everything after first word into message string
    if first is GRID
        pass message to initialGrid
        call joinGame
    if first is GOLD or DISPLAY or OK
        pass message, first, player to updatePlayer
    if first is QUIT
        pass message, player to leaveGame

#### `handleInput`:

    checkInput
    allocate buffer
    read input into buffer
        strip newline
    send message to server in 'KEY [key]' format as in requirements

#### `checkInput`:

    check that arg is not null
    check that server exists
    read input
    check that input is a valid command (keystroke)
        if spectator, q is only valid keystroke

#### `initialGrid`:

    read string into two integers, row num and column num
    check that display fits grid
    
#### `renderScreen`:

    print "header string" with information described in requirements spec
    print local map string (stored in player) to console 
    
#### `joinGame`:

    initialize player struct
    store name from command line in player struct if it is valid (non-space characters)
      build string "PLAY [real name]"
      send string to server
      free string
    if there was no playername provided at command line start
      send SPECTATE to server
    
#### `updatePlayer`:


    if GOLD message
        read string into variables
        update player gold
        print all gold info
    if DISPLAY message
        update player display
        call renderScreen with map string
    if OK message
        update player letter

#### `leaveGame`:

    print quit message from server 
    delete player struct
    free everything else
    
---

## Server

### Data structures

We use a hashtable to store player structs (defined in the player module) for the current players in the game, the key refers to a player's letter name and the item is a player struct.

We use a grid struct, defined in the grid module, to represent the game map.

We use a player struct to represent and store data pertaining to each player connected to the server.

### Definition of function prototypes

A function to parse the command-line arguments, initialize the game struct, initialize the message module, and (BEYOND SPEC) initialize analytics module.

```c
static int parseArgs(const int argc, char* argv[]);
```

A function which utilizes the grid module to create a map, generates a random number (between 10 and 30) of piles each with a random number of gold (total of 250) populate the map with gold piles. 

```c
static int initializeGame(char* filepathname, int seed);
```

Initializes a new player setting their gold to 0 and placing them randomly on the map, connects player with server

```c
static void handlePlayerConnect(char* name, int playerIndex, player_t** players);
```

Handles player disconnects, if a player disconnects early, print QUIT message and remove player. If game is over, disconnects all players and sends end of game message

```c
static void handlePlayerDisconnect(int playerIndex, player_t** players);
```

Updates all players to a statechange, calculating new visibility and updating their maps

```c
static void updateClientState(char* map);
```

Creates a new spectator if one does not exists, otherwise sends QUIT message to current spectator and creates a new spectator

```c
static void handleSpectator(void);
```

Send each client a GAMEOVER message, including results from the game, disconnect each client from the game.

```c
static void gameOver(void);
```

Handles a player move, validating move and updating map

```c
static void movePlayer(int playerID, char directionChar);
```

Handles when a player picks ups gold, updating their total, updating the map, and sending a GOLD message to all clients

```c
static void goldPickup(int playerID, int[] piles);
```

Calculates a player's current vision based on location, updates their map accordingly

```c
static void updateVision(int playerID);
```

### Detailed pseudo code

#### `parseArgs`:

	validate commandline
	verify map file can be opened for reading
	if seed provided
		verify it is a valid seed number
		seed the random-number generator with that seed
	else
		seed the random-number generator with getpid()
        
#### `initializeGame`:

    validate parameters
    create grid calling grid_new
    generate random gold piles
    place piles randomly in grid, checking that they are placed in valid spots
        
#### `handlePlayerConnect`:

     validate name, making sure that it:
     is not empty
     does not exceed maxNameLength
     if name is valid
         check if hashtable size equals maxPlayers
         if there's room
             assign the player a non-used letter as their key
             add player to hashtable, initializing gold to 0
             save player name for future use, replacing isspace() and isgraph() with underscores
             add player to random valid room spot
             update map to reflect player
             send message to the client "OK L" where L is the player's letter
         else
             error message         
      else
        error message
        
             
#### `handlePlayerDisconnect`:
        
    if early disconnect
        grab player data struct from hashtable
        free data if necessary
        delete player struct from hashtable
        send quit message
        close socket
    else
        loop through hashtable
        grab player data struct from hashtable
        free data if necessary
        delete player struct from hashtable
        close socket
    send game over message
    
    
#### `handleSpectator`:
    if no spectator in game:
         create new player struct with spectator switch turned on 
         note there are now max spectators in the game
    else:
        call handle player disconnect for current spectator
        create new player struct with spectator switch turned on 
        note there are max spectators
        
    send GRID, GOLD, and DISPLAY messages
    
#### `updateClientState`
    iterate through players
        call updateVision on a player
        send the player the updated map

#### `gameOver`
    Iterate through players:
        call handle player disconnect
    
#### `movePlayer`
    check move is valid
    if move is capitalised: 
        while move is valid:
            if gold pile is in new position
                call goldPickup
            update player location
            update map
            update all player vision
    else:
        if gold pile is in new position
            call goldPickup
        update player location
        update map
        update all player vision

#### `pickupGold`
    update player gold total
    update total gold remainging
    update map to reflect absence of pile
    update piles[] to reflect absence of pile
    send GOLD message to all clients
    update clients to state change
    
#### `updateVision`
    set all previously seen points to 'reference' grid values
    calculate range of vision
    set current range of vision to values from "active" grid
    return updated map
    
---

## Grid 

The *grid* module implements the map used by the nuggets game and encapsulates all functions that create, delete, or modify the in-game map.

### Data structures

The primary data structure within the `grid` module is the `struct grid`. It stores both the reference and active mapDeclaration:

```c
typedef struct grid {
  char* reference;
  char* active;
  int numColumns;
  int numRows;
} grid_t;

```

### Definition of function prototypes

#### Getters
Getters are fairly self-explanatory, returning the relevant values or `NULL`/ 0 if they don't exist. 
```c
char* grid_getReference(grid_t* grid);
char* grid_getActive(grid_t* grid);
int grid_getNumRows(grid_t* grid);
int grid_getNumColumns(grid_t* grid);
```

#### `grid_new`
The grid_new function creates a `struct grid` that contains information about the in-game map. It is built by reading the file at the path provided.
```c
grid_t* grid_new(char* mapFile);
```

#### `grid_replace`
The grid_replace function provides the ability to replace the character at the given index position in the given grid's active map with the given character. It is primarily used as a helper for other functions, but is still exported for its general functionality.

```c
bool grid_replace(grid_t* grid, int pos, char newChar);
```

#### `grid_delete`
The grid_delete function free's all memory in use by the provided grid. It can be called on a grid where the inner strings are `NULL`, as it checks before free'ing those strings.

```c
void grid_delete(grid_t* grid);
```

#### `longestRowLength`
The longestRowLength function finds the length of the longest row in a map, which corresponds to the number of columns in the map. It is a helper function for grid_new.

```c
static int longestRowLength(char* map);
```

### Detailed pseudo code

Getters are fairly self-explanatory and will not be detailed here, see the appropriate README for details.

#### `grid_new`:
```
allocate space for the grid struct
open map file
if it opens successfully
  find number of rows using file_numLines
  read reference map into memory using file_readFile
  create active map as a copy of reference
  set number of columns using findLongestRow()
  return the grid
delete grid and return NULL in case of failure to open file or allocate memory 

```

#### `grid_replace`:
```
if either string in parameters are NULL or if given index is less than 0
  return false
set the character at the given position in the given grid's active map to the given character
return true
```

#### `grid_delete`:
```
if the active map is not null
  free it
if the reference map is not null
  free it
free the given grid
```


#### `longestRowLength`:
```
store the length of the given map
loop over all characters in the map
  if the current character is '\n'
    if the length of the current row is greater than previous max
      set max to current length
    reset current length 
  else
    increment current length
return max length
```


---

## Player

The *player* module defines and implements the `struct player` that encapsulates all of the information required for each player, including their name, a `struct grid` containing the player's map, which is the sum of their current and previous vision, current position, and current gold. 

### Data structures
The primary data structure within the *player* module is the `struct player`, which is then used by both the `client` and the `server`. It is defined as follows:
```c
typedef struct player {
  char* name;
  char* letter;
  char* vision;
  int pos;
  int gold;
} player_t
```

### Definition of function prototypes
#### Getters
Getters are fairly self-explanatory, returning the relevant values or `NULL`/ 0 if they don't exist. 
```c
char* player_getVision(player_t* player);
char* player_getName(player_t* player);
char* player_getLetter(player_t* player);
int player_getPos(player_t* player);
int player_getGold(player_t* player);
```
#### Setters
Setters are fairly self-explanatory, providing the ability to set member values without directly referencing them. Stylistic choice to make code more readable.
```c
char* player_setVision(player_t* player, char* vision);
int player_setPos(player_t* player, int pos);
int player_setGold(player_t* player, int gold);
```
#### `player_new`
The *player_new* function creates a new `struct player` with the given name, position, and vision. The name and vision strings are copied by the `player` so the original strings can be free'd by the user after function call. Gold is initialized to 0.
```c
player_t* player_new(char* name);
```

#### `player_delete`
The *player_delete* function frees all the memory in use by a `struct player`. It free's the name and vision strings if they are not `NULL`, and always free's the struct itself.
```c
void player_delete(player_t* player);
```

### Detailed pseudo code

#### `player_new`
```
if name parameter not null
  allocate space for a new player struct
  set its name to the given string
  set gold to 0
  return the struct
else
  return NULL
```

#### `player_delete`
```
if player not null
  if vision not null
    free vision
  if name not null
    free name
free player
else
  do nothing
```

---

## Testing plan

### unit testing

The grid module contains a small unit test that is enabled by compiling it with the GRIDTEST flag. It reads a grid into memory from a given file, prints the intial states of both reference and active maps, then modifies the active map and reprints. The grid module was tested on a variety of map files during development.

The player module is small enough that it does not require a seperate unit test. Its functionality can be determined during our larger integration and systems tests. 

### integration testing

We will test the server using the provided miniclient to test the messages we send to the server, simply echoing them back to the client's console. 

Once we have a server that works we will test our client by connecting to our server and observing its behavior, testing whether it can send and receive messages. 

### system testing

First, we will test our complete system by connecting a spectator and seeing what that client receives from the server. 

Then, we will test the complete product by connecting several player clients to our server and attempting to play the game on a variety of maps. We anticipate testing edge cases such too many players, spectators, and all players disconnecting. 

---

## Limitations

None as of inital specs, will potentially be filled in later.
