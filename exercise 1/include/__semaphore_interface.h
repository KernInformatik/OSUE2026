/**
 * @file semaphore.h
 * @author Cihat Simsek (e12519447@student.tuwien.ac.at)
 * @brief The semaphore library, an API for semaphores
 * @version 0.1
 * @date 2026-10-20
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "common.h"
#include "graph.h"
#ifndef __SEMAPHORE_INTERFACE_H
#define __SEMAPHORE_INTERFACE_H

#define SHM_NAME "e125196_SHM"
#define MAX_NUM_OF_DATA 24

struct shm
{
    struct EdgeList shm_object[MAX_NUM_OF_DATA];
};

void onSignal(int sig, siginfo_t *info, void *unused);
void initSignalHandler(void);

#endif
