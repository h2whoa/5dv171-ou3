CC=gcc
CFLAGS=-std=c99 -lpthread

iotest:
	$(CC) src/main.c -o iotest $(CFLAGS)

clean:
	rm iotest
