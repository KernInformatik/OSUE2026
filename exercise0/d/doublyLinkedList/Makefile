.PHONY: all

all: main

main: main.o list.o
	gcc -std=c99 -Wall -pedantic -o $@ $^

list.o: list.c list.h
	gcc -std=c99 -Wall -pedantic -c -o $@ $<

main.o: main.c list.h
	gcc -std=c99 -Wall -pedantic -c -o $@ $<

list.o: list.c list.h
