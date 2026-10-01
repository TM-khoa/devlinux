CC = gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude -g -pthread

all: print_queue

run:
	./print_queue

main:
	$(CC) $(CFLAGS) print_queue.c -o print_queue


.PHONY: clean run all
clean:
	rm print_queue
