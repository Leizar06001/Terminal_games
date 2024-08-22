
CC = gcc
CFLAGS = -Wall -Wextra

all: morp tc

re: clean all

morp: morpion.c
	$(CC) $(CFLAGS) -o morpion morpion.c

tc: toucher_couler.c
	$(CC) $(CFLAGS) -o toucher_couler toucher_couler.c

clean:
	rm -f morpion toucher_couler

.PHONY: all clean
