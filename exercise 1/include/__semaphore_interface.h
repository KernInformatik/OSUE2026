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

#define MAX_NUM_OF_DATA 24

struct shm
{
    struct EdgeList data[MAX_NUM_OF_DATA];
    size_t pos_writehead;
    size_t pos_readhead;
};

/*Static Definitions for SHM and Semaphores BEGIN*/
#define SHM_NAME "/e12519647_shm"

/*https://man7.org/linux/man-pages/man3/shm_open.3.html*/
#define SHM_OFLAG_SERVER (O_RDWR | O_CREAT | O_EXCL)
#define SHM_OFLAG_CLIENT O_RDWR

/* For Setting the  rwx permissions https://ftp.gnu.org/old-gnu/Manuals/glibc-2.2.3/html_node/libc_278.html*/
#define SHM_MODE S_IRWXU

/*https://man7.org/linux/man-pages/man2/mmap.2.html*/
#define MMP_PROT_FLAGS (PROT_READ | PROT_WRITE)
#define MMP_FLAGS MAP_SHARED
#define MMP_OFFSET 0
/*Static Definitions for SHM and Semaphores END*/

void onSignal(int sig, siginfo_t *info, void *unused);
void initSignalHandler(void);

sem_t *initializeSemaphore_Server();
sem_t *initializeSemaphore_Client();

struct shm *initializeSharedMemory_Server();
struct shm *initializeSharedMemory_Client();

void clearSharedMemory_Server();
void clearSharedMemory_Client();

void clearSemaphore_Server();
void clearSemaphore_Client();

#endif
