# Makefile for our Nuggets game
# Adapted from top-level TSE Makefile
# CS50, Winter 2022, team 1 

L = libcs50

.PHONY: all clean

############## default: make all libs and programs ##########
# If libcs50 contains set.c, we build a fresh libcs50.a;
# otherwise we use the pre-built library provided by instructor.
all: 
	(cd $L && if [ -r set.c ]; then make $L.a; else cp $L-given.a $L.a; fi)
#	make -C support
	make -C common

############## clean  ##########
clean:
	rm -f *~
	make -C libcs50 clean
	make -C common clean
#	make -C support clean
