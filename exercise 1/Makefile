# Src: https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories

.PHONY: all clean

SRC_DIR = ../src
BIN_DIR = ../bin
LIB_DIR = ../include

CC = gcc
COMMON_EXERCISE_FLAGS =  -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c 
COMMON_FLAGS = -std=c99 -pedantic -Wall -Werror  -O2 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
COMMON_HARD_FLAGS = -std=c99 -pedantic -Wall -Werror  -O2 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c D_FORTIFY_SORUCE = 2 -fstack-protector-strong -fstack-clash-protection -fcf-protection
CFLAGS =  ${COMMON_FLAGS}

EXE := $(BIN_DIR)/

