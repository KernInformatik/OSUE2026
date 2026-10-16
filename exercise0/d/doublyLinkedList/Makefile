.PHONY: all clean
CC = gcc
CFLAGS = -std=c99 -Wall -Werror -pedantic -g
all: main


main: main.o list.o
	$(CC) $(CFLAGS) -o $@ $^

list.o: list.c list.h
	$(CC) $(CFLAGS) -c -o  $@ $<

main.o: main.c list.h
	$(CC) $(CFLAGS) -c -o $@ $<

list.o: list.c list.h

clean: 
	rm -rf *.o main