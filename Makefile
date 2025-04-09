CFLAGS = -Wall -Wextra -ggdb

main: main.c ali2.h
	$(CC) $(CFLAGS) -o $@ $<
