CC = gcc
CFLAGS=-Wall -Werror -Wextra -std=c99 -Iinclude -g -pthread

all: print_queue

run:
	./print_queue

print_queue:
	$(CC) $(CFLAGS) print_queue.c -o print_queue


.PHONY: clean run all
clean:
	rm print_queue
