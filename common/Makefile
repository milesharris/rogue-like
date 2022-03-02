# Makefile for nuggets common library
# 
# Builds and compiles files common to our nuggets game
#
# Winter 2022, CS50 team 1

# object files, library dependency, and the target library
OBJS = grid.o player.o
LIB = common.a
L = ../libcs50


CFLAGS = -Wall -pedantic -std=c11 -ggdb $(FLAGS) -I$L
CC = gcc
MAKE = make
VALGRIND= valgrind --leak-check=full --show-leak-kinds=all

# Build $(LIB) by archiving object files
$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS) 

gridtest: grid.c 
	$(CC) $(CFLAGS) -DGRIDTEST grid.c $L/libcs50.a -o $@
	$(VALGRIND) ./gridtest ../maps/edges.txt &> gridtest.out

playertest: player.c
	$(CC) $(CFLAGS) -DPLAYERTEST player.c -o $@
	$(VALGRIND) ./playertest testname &> playertest.out

# Dependencies: object files depend on header files
grid.o: grid.h
player.o: player.h

.PHONY: clean

# clean up after our compilation
clean:
	rm -f core
	rm -f $(LIB) *~ *.o
	rm -f gridtest
	rm -f playertest
