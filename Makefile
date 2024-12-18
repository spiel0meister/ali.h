CFLAGS = -Wall -Wextra -ggdb

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $<
