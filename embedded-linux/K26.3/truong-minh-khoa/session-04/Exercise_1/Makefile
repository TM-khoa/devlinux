CC = gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude -g

all: manager

run:
	./manager

main:
	$(CC) $(CFLAGS) manager.c -o manager


.PHONY: clean run all
clean:
	rm manager
