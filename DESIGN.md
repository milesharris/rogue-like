# CS50 Nuggets
## Design Spec
### Team 1: windows_us, Winter 2022

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes the message module, the grid module, z modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Player

The *client* acts in one of two modes:

 1. *spectator*, the passive spectator mode described in the requirements spec.
 2. *player*, the interactive game-playing mode described in the requirements spec.

### User interface

See the requirements spec for both the command-line and interactive UI.

### Inputs and outputs

Inputs: The user interacts with the game by entering 

We log to log files and/or stderr in cases where we experience errors and want to communicate to the user how the program should be properly used. We also use log files when we want to display text to the user. 

### Functional decomposition into modules

> parseArgs
> initialize grid
> render screen
> join game
> leave game
> update player
> handle messages

### Pseudo code for logic/algorithmic flow

renderScreen:
```
print "header string" with information described in requirements spec
print local map string to console

```

parseArgs:
```
validate commandline
initialize message module
print assigned port number
decide whether spectator or player
initialize player
```

initialGrid
```
read string into two integers, row num and column num
check that display fits grid
```

joinGame
```
initialize player struct
store name from command line in player struct if it is valid (non-space characters)
build string "PLAY [real name]"
send string to server
free string
if there was no playername provided at command line start
send SPECTATE to server
```

leaveGame
```
print quit message from server 
delete player struct
free everything else
```

updatePlayer
```
if GOLD message
    read string into variables
    update player gold
    print all gold info
if DISPLAY message
    update player display
    call renderScreen with map string
if OK message
    update player letter
```

handleMessage
```
read first word into first string
read everything after first word into message string
if first is GRID
    pass message to initialGrid
    call joinGame
if first is GOLD or DISPLAY or OK
    pass message, first, player to updatePlayer
if first is QUIT
    pass message, player to leaveGame
```

### Major data structures

We plan to implement the `hashtable.h` data structure from libcs50.

---

## Server
### User interface

See the requirements spec for the command-line interface.
There is no interaction with the user.

### Inputs and outputs

collision handling: if a player moves into a spot occupied by another player, swap them.

vision: each player keeps track of what it has already seen, keeping a map string that uses filler for non-seen space and copies from the grid for seen space. Note: players cannot "remember" gold location in previous rooms, nor can they see player movement in rooms they have already visited (maybe send "original" map to player).

### Functional decomposition into modules

We expect our server to have the following functions:

start server
initialize game
    generate maps
    generate gold
handle player connect
handle player disconnect
handleMessage
update clients whenever state change
handle spectator
game over
move player (collision handled here)
player picks up gold
calculate vision for each player on move


### Pseudo code for logic/algorithmic flow

The server will run as follows:

	execute from a command line per the requirement spec
	parse the command line, validate parameters
	call initializeGame() to set up data structures
	initialize the 'message' module
	print the port number on which we wait
	call message_loop(), to await clients
	call gameOver() to inform all clients the game has ended
	clean up


### Major data structures

Player data structure: Contains all information for in-game players, such as: current gold, mode, current position, name, and current vision.

A hashtable containing all players, keyed by their character representation in game world (A, B, C...)
	stores the player structs as items
	does not contain the spectator
	

A "game" data structure to hold the state of the current game, including number of players, table of players, remaining gold, spectator, etc.

A grid structure to keep track of the map state (includes both "base" and "active" map)

---

## Grid module

A module to handle all functions related to loading, drawing, modifying, and redrawing the map.

### Functional decomposition

read map file into string
generate numRows and numColumns
handle changes to grid (player move, gold add, gold remove, etc)

### Major data structures

A structure to keep track of a grid. The grid itself is stored as a string, and referencing positions is done using string parsing. The struct contains two versions of the map string. The original, "unaltered" map for reference and an "active" map that is modified with gold etc.

## Game module

A module to handle all functions related to handling game structures.

### Functional decomposition

getters
setters
creating new game structs
deleting game structs
adding to game structs
summarizing game structs

### Major data structures

A "game" data structure to hold the state of the current game, including number of players, table of players, remaining gold, spectator, etc.

## Player module

The player module stores necessary information relating to a player including current gold, position, name, and current vision. It also provides various functions for retrieving and altering this information as well as for creating and removing player's.

### Functional decomposition

getters
setters
creating new player structs
deleting player structs
updating vision of player structs
summarizing player structs

### Major data structures

A structure to keep track of a player. 
