# CS50 Nuggets
## Design Spec
### Team 1: windows_us, Winter 2022

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes the message module (in support file) and the grid module.
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

Inputs: The user interacts with the game by entering different keystrokes. Client receives messages from the server, such as 'GRID', 'PLAY', etc. as described in requirements. 

Output: The client logs useful information (errors) to stderr. Usage would thus be through the command line, as described in the requirements spec. The client will also output different QUIT messages, as described in requirements. Finally, the client will output to ncurses the display, with visibility calculated server-side.  

### Functional decomposition into modules

> render screen
> join game
> leave game 
> send keystrokes (movement data and other info) to server
> update game state info from server info

### Pseudo code for logic/algorithmic flow


renderScreen:

    print "header string" with information described in requirements spec
    print local map string to console
    *note: display size is checked once, on initial render*


joinGame: 
```
    connects to server port using hostname and port number
    if player name provided 
        join as player
    otherwise
        join as spectator

```
leaveGame:
```
    if QUIT message received
        print the message provided with QUIT message, as in requirements

```

sendKeys:
```
    if key is pressed 
        send pressed key to server

```
update:
```
    if new info provided by server
        redraw map with new info (call renderScreen)
    
```

### Major data structures


We plan to use the hashtable from libcs50.

---

## Server
### User interface

See the requirements spec for the command-line interface.
There is no interaction with the user.

### Inputs and outputs

Inputs: Reads a map file from the map pathname parameter. 

Outputs: The server logs useful information (errors) to stderr. Usage would thus be through the command line, as described in the requirements spec. 


### Functional decomposition into modules

We expect our server to have the following functions:

start server
initialize game
    generate maps from file
    generate gold, with or without random number seed
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


#### initializeGame
    creates map string from map.txt
    generate a random number of piles: within defined range
    assign each pile a random # of nuggets s.t. total number of nuggets =250
    assigns piles to random valid locations in the map: ie not in walls/corners etc.

#### handle player connect
    some sort of server-client interaction
    initialize new player, adding them to player storage structure
    player starts with 0 gold
    add player to randomly selected and valid room spot
    update map to reflect new player
    
#### handle player disconnect
    if player has disconnected early: ie, game not over
        send "QUIT Thanks for playing!"
        remove player from hashtable which stores players
        update the map to reflect the absence of this player
        * note the gold from this player is non-recoverable *
    otherwise the game must be over so send the end of game message
    close server interaction with this client
    * we do not need to send this player additional messages *
    
#### update clients to state change
    iterate through clients *players*
        call calculate vision on player move
        sends updated map

#### handle spectator 
    server-client interaction
    if spectator doesn't exist:
        create a new spectator *essentially a player with some spectator flag switched on*
        note that a max spectators now exist
    else if spectator does exist:
        send quit message to current spectator
        
    send GRID, GOLD, and DISPLAY messages

#### game over
    sends each client a game over message, displaying a textual table of the results
    (disconnects each client from server)

#### move player
    check move is valid
    check whether player picks ups gold
    if they do:
        call player picks up gold
    update map with new player location
    call update clients to state change
    
#### player picks up gold
    update player gold tally
    update total 'nuggets unclaimed'
    update map - replacing pile char (*) for empty char (.)
    send GOLD message to all clients
    call (update clients to state change)
    
#### calculate vision on move
    for a client:
    calculate their range of vision
    add previously unseen areas to their map
    change previously seen, but now unseen, areas back to their 'base map' values
    return updated map


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

readMap
findNumRows
findNumColumns
handle changes to grid (player move, gold add, gold remove, etc)

### Pseudo code for logic/algorithmic flow

To find numRows, use the file_numLines function in cs50lib

readMap:

    open provided filepath for reading
    read entire file into a string (using file module)
    return the map string
    
findNumColumns:

    loop through all characters in the map string
        increment a counter until newline reached
        if the counter is greater than the previous max length of a line
            remember new maxLength
        return maxLength

modifyMap
    
    change character at given index to given character
    if a player moved off a space
        change the character where they just were to the character in the reference map at that position
        

### Major data structures

A structure to keep track of a grid. The grid itself is stored as a string, and referencing positions is done using string parsing. The struct contains two versions of the map string. The original, "unaltered" map for reference and an "active" map that is modified with gold etc.
