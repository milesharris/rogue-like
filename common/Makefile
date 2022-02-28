# Makefile for nuggets common library
# 
# Builds and compiles files common to our nuggets game
#
# Winter 2022, CS50 team 1

# object files, library dependency, and the target library
OBJS = pagedir.o index.o word.o query.o
LIB = common.a
L = ../libcs50


CFLAGS = -Wall -pedantic -std=c11 -ggdb $(FLAGS) -I$L
CC = gcc
MAKE = make

# Build $(LIB) by archiving object files
$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS) 

# Dependencies: object files depend on header files
grid.o: grid.h

.PHONY: clean

# clean up after our compilation
clean:
	rm -f core
	rm -f $(LIB) *~ *.o
