BIN = bfi
CC = gcc
CFLAGS = -O3
DEBUG = -g
SRC = src/main.c

default: build
all: debug

clean: 
	rm $(BIN)
build:
	$(CC) $(SRC) $(CFLAGS) -o $(BIN)

debug:
	$(CC) $(SRC) $(DEBUG) -o $(BIN)

install:
	cp bfi /usr/bin/bfi
	chmod +x /usr/bin/bfi
