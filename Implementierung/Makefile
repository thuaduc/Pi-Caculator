CFLAGS= -O3 -g -Wall -Wextra -Wpedantic -std=gnu11
LDFLAGS=-lm

.PHONY: all clean

all: test
test: test.c operations.S 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
clean:
	rm -f test

