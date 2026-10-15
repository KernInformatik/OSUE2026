.PHONY: all clean

all: err1 err2

err%: err%.c
	gcc -std=c99 -Wall -pedantic -o $@ $<

clean:
	rm err[0-9]
