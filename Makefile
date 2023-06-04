CC = gcc
CFLAGS = -Wall -Wextra -g

stree: stree.c
	$(CC) $(CFLAGS) -o stree stree.c

clean:
	rm -f stree