#Inspiration from suckless https://git.suckless.org/dwm/files.html


CC = gcc
COMMON_EXERCISE_FLAGS =  -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
COMMON_FLAGS = -std=c99 -pedantic -Wall -Werror  -O2 -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L -g -c
COMMON_HARD_FLAGS = ${COMMON_FLAGS} -fstack-protector-strong -fstack-clash-protection -fcf-protection
CFLAGS =  ${COMMON_FLAGS}