CC=gcc
CFLAGS=-g -lm -Wall

example:
	$(CC) -o $@ example.c base64.h $(CFLAGS)

clean:
	rm example
