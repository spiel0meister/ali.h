CFLAGS = -Wall -Wextra -Werror -fsanitize=undefined,address -ggdb

all: main

main: main.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf main
