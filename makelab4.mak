CC = gcc
CFLAGS=-lpthread -Wall

test: thread.c 
	$(CC) -o test thread.c $(CFLAGS)

.PHONY: clean
clean:
	rm -f test