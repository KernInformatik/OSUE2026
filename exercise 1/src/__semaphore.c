/**
 * @file semaphore.c
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief The semaphore library, uses the library ../include/semaphore.h
 * @version 0.1
 * @date 2026-10-20
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "../include/__semaphore_interface.h"

volatile sig_atomic_t quit = false;

void onSignal(int sig, siginfo_t *info, void *unused)
{
    quit = true;
}

void initSignalHandler()
{
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = onSignal;
}

/*BEGINE HELPER FUNCTIONS*/

/*END HELPER FUNCTIONS*/

/**************************************/

/*BEGIN INTERFACE*/

/*END INTERFACE*/