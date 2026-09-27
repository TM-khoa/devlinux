CC = gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude -g

all: manager searcher

run:
	./manager

manager:
	$(CC) $(CFLAGS) manager.c -o manager

searcher:
	$(CC) $(CFLAGS) searcher.c -o searcher


.PHONY: clean run all
clean:
	rm manager searcher
