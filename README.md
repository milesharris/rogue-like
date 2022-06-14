# Rogue-like
This repository contains my best attempts at writing *Rogue*. It is based on Dartmouth's CS50 final project "nuggets", which is itself inspired by *Rogue*. For transparencies sake I will keep all documentation related to the original *Nuggets* implementation in relevant documents. Further details can be found in the design and implementation specs. 

## To-do list

 * 0. Debug original nuggets client, learn ncurses, etc. 
 * 1. Make a multi-leveled dungeon, but keep everything else the same so just multi level nuggets
 * 2. Implement the amulet and Rogue victory conditions, maybe basic inventory
 * 3. Make it all turn-based (yikes)
 * 4. Add monsters into the game without combat, make them random walkers
 * 5. Make monsters hunt players in sight, but keep nuggets "bumping" rules
 * 6. Combat, player stats, death
 * 7. Full inventory w/ item pickups and potions and shit I guess
 * 8. God even knows at this point I might be insane


## Nuggets

This repository contains the code for the CS50 "Nuggets" game, in which players explore a set of rooms and passageways in search of gold nuggets.
The rooms and passages are defined by a *map* loaded by the server at the start of the game.
The gold nuggets are randomly distributed in *piles* within the rooms.
Up to 26 players, and one spectator, may play a given game.
Each player is randomly dropped into a room when joining the game.
Players move about, collecting nuggets when they move onto a pile.
When all gold nuggets are collected, the game ends and a summary is printed.

## Materials provided

See the [support library](support/README.md) for some useful modules.
See the [maps](maps/README.md) for some draft maps.
The profclient file in this directory is a copy of the professor's shared client. We copied it here so that we could more easily use it to test and redirect its output. It remains in the repository for transparency reasons, as we extensively made use of it to test. 
