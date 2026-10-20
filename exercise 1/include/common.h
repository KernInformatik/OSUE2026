/**
 * @file common.h
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief A common header, to cover the majority of the libraries. Includes also macros
 * @version 0.1
 * @date 2026-10-20
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef COMMON_H
#define COMMON_H

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <semaphore.h>
#include <semaphore.h> // for sem_t
#include <signal.h>
#include <signal.h> // for sig_atomic_t, siginfo_t
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>

static void inline error(const char *msg)
{
    perror(msg);
}

static void inline error_exit(const char *msg, int value)
{
    error(msg);
    if (value == -1)
    {
        _exit(EXIT_FAILURE);
    }
}

#endif
