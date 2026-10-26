/**
 * @file supervisor.c
 * @author Cihat Simsek (e12519647@student.tuwien.ac.at)
 * @brief
 * @version 0.1
 * @date 2026-10-25
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "../include/__semaphore_interface.h"
#include <bits/getopt_core.h>
#include <stdio.h>
#include <time.h>

extern volatile sig_atomic_t quit;

static void delay(uint seconds)
{
    clock_t start_time = clock();
    while (clock() - start_time < seconds * CLOCKS_PER_SEC)
        ;
}

void readSolution(struct shm *buffer)
{
    struct EdgeList solution = buffer->data[buffer->readhead];
    buffer->readhead = (buffer->readhead + 1) % MAX_BUFF_SIZE;

    if (solution.length == 0)
    {
        printf("The graph is acyclic!\n");
    }
    static long currenBest = ULONG_MAX;

    if (solution.length < currenBest)
    {
        currenBest = solution.length;
        printf("found solution with %zu edges\n", solution.length);

        for (ssize_t i = 0; i < solution.length; ++i)
        {
            printf("%d-%d", solution.NumberOfEdges[i].u.node_number, solution.NumberOfEdges[i].v.node_number);
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    int shmfd;
    struct shm *sharedMemory = sharedMemory_Server(&shmfd);
    sem_t *free, *used, *write;
    free = initializeSemaphore_Server(FREE_SPACE_SEMAPHORE, FREE_SPACE_SEMAPHORE_SIZE);
    write = initializeSemaphore_Server(WRITE_SPACE_SEMAPHORE, WRITE_SPACE_SEMAPHORE_SIZE);
    used = initializeSemaphore_Server(USED_SPACE_SEMAPHORE, USED_SPACE_SEMAPHORE_SIZE);

    initSignalHandler();

    /* TODO COMMAND LINE PARSING + Prettier error messages*/
    // supervisor [-h] [-o outfile] [-n limit] [-w delay]
    int opt;

    while ((opt = getopt(argc, argv, "ho:n:w:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            break;
        case 'o':
            break;
        case 'n':
            break;
        case 'w':
            break;
        case '?':
            break;
        case ':':
            break;
        }
    }

    if (optind > argc)
    {
        error_exit("Expected argument ... exiting");
    }

    do
    {

    } while (++optind < argc);

    /*End of the CLI */

    while (!quit)
    {
        if (sem_wait(used) == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            error_exit("Couldn't initialize sempahore");
        }
        if (quit)
        {
            break;
        }
        readSolution(sharedMemory);
        if (sem_post(free) == -1)
        {
            error_exit("sem_post Server side");
        }
    }

    cleanSemaphore_Server(free, FREE_SPACE_SEMAPHORE);
    cleanSemaphore_Server(used, USED_SPACE_SEMAPHORE);
    cleanSemaphore_Server(write, WRITE_SPACE_SEMAPHORE);

    cleanSharedMemory_Server(sharedMemory, shmfd);
}