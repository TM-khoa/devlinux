CC = gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude -g -pthread

all: main

run:
	./booking

main:
	$(CC) $(CFLAGS) booking.c -o booking


.PHONY: clean run all
clean:
	rm booking
