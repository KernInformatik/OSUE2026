# Src: https://stackoverflow.com/questions/30573481/how-to-write-a-makefile-with-separate-source-and-header-directories
include config.mk

.PHONY: options all clean doc

SRC_DIR = ../src
BIN_DIR = ../bin
LIB_DIR = ../include

# Directly implemented from DWM (suckless)
all: options server generator
options:
	@echo build options:
	@echo "CFLAGS			= ${CFLAGS}"
	@echo "CC				= ${CC}"

.c.o:
	${CC} -c {CFLAGS} $<

doc:
	doxygen Doxyfile

EXE_SUPERVISOR := $(BIN_DIR)/supervisor
EXE_GENERATOR := $(BIN_DIR)/generator

SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
