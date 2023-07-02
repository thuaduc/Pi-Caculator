CFLAGS= -O3 -g -Wall -Wextra -Wpedantic -std=gnu11
LDFLAGS=-lm

.PHONY: all clean

all: main
main: main.c operations.S multiplication.c pi.c operations.c test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
clean:
	rm -f main
	