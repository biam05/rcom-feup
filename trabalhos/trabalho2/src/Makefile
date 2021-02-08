CC=gcc

# Compiler flags
CFLAGS=-Wall -Wextra -Wno-unused-parameter -pedantic

# Source files
SRCS=download.c

# Targets
PROG=download

all: download

download:
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS)

clean:
	rm $(PROG)