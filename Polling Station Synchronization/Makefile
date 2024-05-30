#
# Samantha Foley
# CS 441/541: Synchronization Project
#
#####################################################################
#
# Type "make" to compile your code
# 
# Type "make clean" to remove the executable (and any object files)
#
#####################################################################

CC=gcc
CFLAGS=-Wall -g -O0 -Ilib
LDFLAGS=-pthread

PROGS=finicky-voter
#
# List all of the binary programs you want to build here
# Separate each program with a single space
#
all: $(PROGS)

#
# Finicky Voter
#
finicky-voter: finicky-voter.c finicky-voter.h lib/semaphore_support.o
	$(CC) -o finicky-voter finicky-voter.c lib/semaphore_support.o $(CFLAGS) $(LDFLAGS)

#
# Cleanup the files that we have created
#
clean:
	$(RM) $(PROGS) *.o
	$(RM) -rf *.dSYM
	cd lib && make clean
