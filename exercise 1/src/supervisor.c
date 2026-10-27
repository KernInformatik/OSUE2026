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

extern volatile sig_atomic_t quit;
static inline void usage(void)
{
    printf("Usage: ./supervisor [-h] [-o outputfile] [-n limit] [-w delay]");
    return;
}

static void delay(uint seconds)
{
    clock_t start_time = clock();
    while (clock() - start_time < seconds * CLOCKS_PER_SEC)
        ;
}

static int readSolution(struct shm *buffer, FILE *Output, bool delay, bool verbose)
{
    struct EdgeList solution = buffer->data[buffer->readhead];
    buffer->readhead = (buffer->readhead + 1) % MAX_BUFF_SIZE;

    if (solution.length == 0)
    {
        printf("The graph is acyclic!\n");
        return 1;
    }
    static long currenBest = SSIZE_MAX;

    if (solution.length < currenBest)
    {
        currenBest = solution.length;
        printf("The graph might not be acyclic, best solution removes %zd edges.\n", currenBest);

        if (verbose)
        {
            for (size_t i = 0; i < solution.length; ++i)
            {
                printf("%d-%d ", solution.NumberOfEdges[i].u.node_number, solution.NumberOfEdges[i].v.node_number);
            }
        }
        fprintf(Output, "\n");
        fflush(Output);
    }
    return 0;
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
    sharedMemory->status = 1;

    /* TODO COMMAND LINE PARSING + Prettier error messages*/
    // supervisor [-h] [-o outfile] [-n limit] [-w delay]
    int opt, seconds;
    bool verboseOutput = false, definesOutput = false, setLimit = false, setDelay = false;

    FILE *out = stdout;

    while ((opt = getopt(argc, argv, "ho:n:w:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            verboseOutput = true;
            break;
        case 'o':
            definesOutput = true;
            out = fopen(optarg, "w+");
            assert(out != NULL);
            break;
        case 'n':
            setLimit = true;
            break;
        case 'w':
            setDelay = true;
            seconds = strtof(optarg, NULL);
            break;
        case '?':
            usage();
            exit(EXIT_SUCCESS);
            error_exit("Unknown argument, see usages");
            break;
        case ':':
            usage();
            printBuddha();
            error_exit("Error no Arguments given");
            break;
        }
    }

    if (optind > argc)
    {
        printBuddha();
        error_exit("Expected argument ... exiting");
    }

    do
    {
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
            fprintf(stderr, "supervisor: woke up\n");
            if (quit)
            {
                sharedMemory->status = 0;
                break;
            }
            int stop = 0;

            if (setDelay)
            {
                delay(seconds);
                stop = readSolution(sharedMemory, stdout, setDelay, verboseOutput);
            }
            else
            {
                stop = readSolution(sharedMemory, stdout, setDelay, verboseOutput);
            }

            if (sem_post(free) == -1)
            {
                printBuddha();
                error_exit("sem_post (free)");
            }
            if (stop)
            {
                sharedMemory->status = 0;
                break;
            }
        }

    } while (++optind < argc);

    /*End of the CLI */

    // while (!quit)
    // {
    //     if (sem_wait(used) == -1)
    //     {
    //         if (errno == EINTR)
    //         {
    //             continue;
    //         }
    //         error_exit("Couldn't initialize sempahore");
    //     }
    //     fprintf(stderr, "supervisor: woke up\n");
    //     if (quit)
    //     {
    //         sharedMemory->status = 0;
    //         break;
    //     }
    //     int stop = 0;

    //     if (setDelay)
    //     {
    //         delay(seconds);
    //         stop = readSolution(sharedMemory, stdout, setDelay, verboseOutput);
    //     }
    //     else
    //     {
    //         stop = readSolution(sharedMemory, stdout, setDelay, verboseOutput);
    //     }

    //     if (sem_post(free) == -1)
    //     {
    //         printBuddha();
    //         error_exit("sem_post (free)");
    //     }
    //     if (stop)
    //     {
    //         sharedMemory->status = 0;
    //         break;
    //     }
    // }

    cleanSemaphore_Server(free, FREE_SPACE_SEMAPHORE);
    cleanSemaphore_Server(used, USED_SPACE_SEMAPHORE);
    cleanSemaphore_Server(write, WRITE_SPACE_SEMAPHORE);

    cleanSharedMemory_Server(sharedMemory, shmfd);

    return EXIT_SUCCESS;
}