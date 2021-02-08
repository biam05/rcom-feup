CC=gcc

# Compiler flags
CFLAGS=-Wall -Wextra -Wno-unused-parameter -pedantic

# Source files
SRCSEMITTER=write.c
SRCSRECEIVER=read.c

# Targets
PROGEMITTER=write
PROGRECEIVER=read

all: emitter receiver

emitter:
	$(CC) $(CFLAGS) -o $(PROGEMITTER) $(SRCSEMITTER)

receiver:
	$(CC) $(CFLAGS) -o $(PROGRECEIVER) $(SRCSRECEIVER)


clean:
	rm $(PROGEMITTER) $(PROGRECEIVER)