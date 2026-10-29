.DEFAULT_GOAL = all

# compiler
CC := gcc

# flags
COMMON_FLAGS := -std=c99 -pedantic -Wall -O2 \
	-D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE \
	-D_POSIX_C_SOURCE=200809L -g

ASAN := -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer -fno-common
CFLAGS := $(COMMON_FLAGS)


#https://stackoverflow.com/questions/2754966/cflags-vs-cppflags
#CPPFLAGS := -I$(INCLUDE_DIR) -I$(SRC_DIR)

# source files
GENERATOR_SRC := generator.c
SUPERVISOR_SRC := supervisor.c

# object files
GENERATOR_OBJ := generator.o
SUPERVISOR_OBJ := supervisor.o

# executables
GENERATOR := generator
SUPERVISOR := supervisor

# library
LIB_SEM_INTF_NAME := libsemaphore_interface.so
LIB_SEM_INTF := $(LIB_SEM_INTF_NAME)

LIB_SRCS := semaphore.c
LIB_OBJS := __lib.so
