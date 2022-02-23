# CS50 Nuggets
## Design Spec
### Team name, term, year

> This **template** includes some gray text meant to explain how to use the template; delete all of them in your document!

According to the [Requirements Spec](REQUIREMENTS.md), the Nuggets game requires two standalone programs: a client and a server.
Our design also includes x, y, z modules.
We describe each program and module separately.
We do not describe the `support` library nor the modules that enable features that go beyond the spec.
We avoid repeating information that is provided in the requirements spec.

## Player

The *client* acts in one of two modes:

 1. *spectator*, the passive spectator mode described in the requirements spec.
 2. *player*, the interactive game-playing mode described in the requirements spec.

### User interface

See the requirements spec for both the command-line and interactive UI.

> You may not need much more.

### Inputs and outputs

Inputs: The user interacts with the game by entering 

> If you write to log files, or log to stderr, describe that here.
> Command-line arguments are not 'input'.

### Functional decomposition into modules


> render screen
> join game
> 
> send move info to server
> receive game state info from server




### Pseudo code for logic/algorithmic flow

> For each function write pseudocode indented by a tab, which in Markdown will cause it to be rendered in literal form (like a code block).
> Much easier than writing as a bulleted list!
> See the Server section for an example.

> Then briefly describe each of the major functions, perhaps with level-4 #### headers.

### Major data structures


> Mention, but do not describe, any libcs50 data structures you plan to use.

---

## Server
### User interface

See the requirements spec for the command-line interface.
There is no interaction with the user.

> You may not need much more.

### Inputs and outputs

collision handling: if a player moves into a spot occupied by another player, swap them.

vision: each player keeps track of what it has already seen, keeping a map string that uses filler for non-seen space and copies from the grid for seen space. Note: players cannot "remember" gold location in previous rooms, nor can they see player movement in rooms they have already visited (maybe send "original" map to player).

> Briefly describe the inputs (map file) and outputs (to terminal).
> If you write to log files, or log to stderr, describe that here.
> Command-line arguments are not 'input'.

### Functional decomposition into modules

start server
initialize game
    generate maps
    generate gold
handle player connection
handle player disconnect
update clients whenever state change
handle spectator
game over
move player (collision handled here)
player picks up gold
calculate vision for each player





### Pseudo code for logic/algorithmic flow

> For each function write pseudocode indented by a tab, which in Markdown will cause it to be rendered in literal form (like a code block).
> Much easier than writing as a bulleted list!
> For example:

The server will run as follows:

	execute from a command line per the requirement spec
	parse the command line, validate parameters
	call initializeGame() to set up data structures
	initialize the 'message' module
	print the port number on which we wait
	call message_loop(), to await clients
	call gameOver() to inform all clients the game has ended
	clean up


> Then briefly describe each of the major functions, perhaps with level-4 #### headers.

### Major data structures

Player data structure: Contains all information for in-game players, such as: current gold, mode, current position, name, and current vision.

A list of all players

A "game" data structure to hold the state of teh current game, including number of players, remaining gold, 



> Mention, but do not describe, data structures implemented by other modules (such as the new modules you detail below, or any libcs50 data structures you plan to use).

A structure to keep track of a grid. 

---

## Grid module

A module to handle all functions related to loading, drawing, and redrawing the map.

### Functional decomposition

read map file into string
generate numRows and numColumns
handle changes to grid (player move, gold add, etc)


### Pseudo code for logic/algorithmic flow

> For any non-trivial function, add a level-4 #### header and provide tab-indented pseudocode.
> This pseudocode should be independent of the programming language.

### Major data structures

A structure to keep track of a grid. The grid itself is stored as a string, and referencing positions is done using string parsing

---

## Message module

A module to handle all functions related to loading, drawing, and redrawing the map.

### Functional decomposition

> List each of the main functions implemented by this module, with a phrase or sentence description of each.

### Pseudo code for logic/algorithmic flow

> For any non-trivial function, add a level-4 #### header and provide tab-indented pseudocode.
> This pseudocode should be independent of the programming language.

### Major data structures

A structure to keep track of a grid. The grid itself is stored as a string, and referencing positions is done using string parsing
