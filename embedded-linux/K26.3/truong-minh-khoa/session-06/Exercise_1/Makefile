CC = gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude -g

all: monitor

run:
	./monitor

monitor:
	$(CC) $(CFLAGS) monitor.c -o monitor


.PHONY: clean run all
clean:
	rm monitor

