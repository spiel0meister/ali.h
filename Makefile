CFLAGS = -Wall -Wextra -Werror -ggdb

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf main
