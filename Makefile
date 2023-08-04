CC=gcc
CFLAGS=-g -lm

example:
	$(CC) -o $@ example.c base64.h $(CFLAGS)

clean:
	rm example
