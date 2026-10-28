.DEFAULT_GOAL = all

# compiler
CC := gcc

# flags
COMMON_FLAGS := -std=c99 -pedantic -Wall -O2 \
	-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE \
	-D_POSIX_C_SOURCE=200809L -g

ASAN := -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-common
CFLAGS := $(COMMON_FLAGS)

# directories
SRC_DIR := src
BIN_DIR := build
INCLUDE_DIR := include
LIB_DIR := lib

#https://stackoverflow.com/questions/2754966/cflags-vs-cppflags
CPPFLAGS := -I$(INCLUDE_DIR) -I$(SRC_DIR)

# source files
GENERATOR_SRC := $(SRC_DIR)/generator.c
SUPERVISOR_SRC := $(SRC_DIR)/supervisor.c

# object files
GENERATOR_OBJ := $(BIN_DIR)/generator.o
SUPERVISOR_OBJ := $(BIN_DIR)/supervisor.o

# executables
GENERATOR := $(BIN_DIR)/generator
SUPERVISOR := $(BIN_DIR)/supervisor

# library
LIB_SEM_INTF_NAME := libsemaphore_interface.so
LIB_SEM_INTF := $(BIN_DIR)/$(LIB_SEM_INTF_NAME)

LIB_SRCS := $(wildcard $(LIB_DIR)/*.c)
LIB_OBJS := $(patsubst $(LIB_DIR)/%.c,$(BIN_DIR)/lib/%.o,$(LIB_SRCS))