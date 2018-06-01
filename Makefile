CC=gcc

CFLAGS=-c -std=gnu99 -Wall
LDFLAGS=-lpthread

all: iotest

iotest: main.o tests.o
	$(CC) $(LDFLAGS) main.o tests.o -o $@

main.o: src/main.c
	$(CC) $(CFLAGS) $<

tests.o: src/tests.c
	$(CC) $(CFLAGS) $<

clean:
	rm -fv iotest *.o
