
CC=$(SAIL_GCC)

CFLAGS=-c -Wall $(SAIL_CFLAGS)

all: hello

hello: test.o
	$(CC) test.o -o hello

test.o: test.c
	$(CC) $(CFLAGS) test.c



clean:
	rm -rf *o hello