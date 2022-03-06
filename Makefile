# Makefile for our Nuggets game
# Adapted from top-level TSE Makefile
# CS50, Winter 2022, team 1 
C = ./common
L = ./libcs50
S = ./support
LLIBS = $C/common.a $L/libcs50.a $S/support.a

CFLAGS = -Wall -pedantic -std=c11 -ggdb $(FLAGS) -I$C -I$L -I$S
CC = gcc
MAKE = make
VALGRIND= valgrind --leak-check=full --show-leak-kinds=all
L = libcs50

.PHONY: all clean

############## default: make all libs and programs ##########
# If libcs50 contains set.c, we build a fresh libcs50.a;
# otherwise we use the pre-built library provided by instructor.
all: 
	(cd $L && if [ -r set.c ]; then make $L.a; else cp $L-given.a $L.a; fi)
	make -C support
	make -C common
#	server
#	client

# exectuables
server: server.o $(LLIBS)
	$(CC) $(CFLAGS) $^ -o $@

client: client.o $(LLIBS)
		$(CC) $(CFLAGS) $^ -o $@

# Dependencies
server.o: server.c
client.o: client.c

############## clean  ##########
clean:
	rm -f *~
	make -C libcs50 clean
	make -C common clean
	make -C support clean
